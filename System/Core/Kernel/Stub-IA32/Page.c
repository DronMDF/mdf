//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Page.h"
#include "Memory.h"
#include "Kernel.h"
#include "Task.h"

extern void __bss_end;

// Замусоривание примапливаемых страниц..
#define PAGE_GARBAGED

#define KERNEL_PAGEDIR		\
	(KERNEL_PAGETABLE_BASE + (KERNEL_PAGETABLE_BASE/PAGE_SIZE) * sizeof (unsigned long))

#define pdidx(addr)		((addr)/PDIR_SIZE)
#define ptidx(addr)		((addr)/PAGE_SIZE)

static sizex_t stubMemoryTotal = 0;
static sizex_t stubMemoryUsed = 0;

sizex_t StubGetMemoryTotal()
{
	return stubMemoryTotal;
}

sizex_t StubGetMemoryUsed()
{
	return stubMemoryUsed;
}

// -----------------------------------------------------------------------------
// Список свободных страниц

typedef struct {
	PageInfo *first;
	int count;
	lock_t lock;
} PageList;

static
PageList stubPageFreeList = {
	.first = nullptr,
	.count = 0,
	.lock = 0,
};

// Размещение страницы в списке свободных.
void StubPageRelease (PageInfo *page)
{
	STUB_ASSERT (page->instances != nullptr, "Release used page");
	STUB_ASSERT (page->kaddr != 0, "Release kernel used page");
	STUB_ASSERT (isSet(page->flags, PFLAG_FREE), "Release free page");

	page->prev = nullptr;

	StubLock (&stubPageFreeList.lock);
	page->next = stubPageFreeList.first;
	if (stubPageFreeList.first != 0)
		stubPageFreeList.first->prev = page;
	stubPageFreeList.first = page;
	stubPageFreeList.count++;
	StubUnlock (&stubPageFreeList.lock);

	page->flags |= PFLAG_FREE;

	STUB_ASSERT(stubMemoryUsed < PAGE_SIZE, "free all memory");
	stubMemoryUsed -= PAGE_SIZE;
}

// Получение страницы из списка свободных
PageInfo *StubPageAlloc()
{
	StubLock (&stubPageFreeList.lock);
	PageInfo *page = stubPageFreeList.first;
	STUB_ASSERT (!isSet(page->flags, PFLAG_FREE), "Not free page in free list");

	PageInfo *prev = page->prev;
	PageInfo *next = page->next;

	if (next != nullptr)
		next->prev = prev;

	if (prev != nullptr) {
		prev->next = next;
	} else {
		stubPageFreeList.first = next;
	}

	stubPageFreeList.count--;

	StubUnlock (&stubPageFreeList.lock);

	page->flags &= ~PFLAG_FREE;

	stubMemoryUsed += PAGE_SIZE;

	return page;
}

void StubPageAllocByInfo(PageInfo *page)
{
	StubLock (&stubPageFreeList.lock);

	// Параноидальную проверку наличия страницы в листе - опустим
	STUB_ASSERT(!isSet(page->flags, PFLAG_FREE), "Page is not free");

	PageInfo *prev = page->prev;
	PageInfo *next = page->next;

	if (next != nullptr)
		next->prev = prev;

	if (prev != nullptr) {
		prev->next = next;
	} else {
		stubPageFreeList.first = next;
	}

	stubPageFreeList.count--;
	StubUnlock (&stubPageFreeList.lock);

	page->flags &= ~PFLAG_FREE;
	stubMemoryUsed += PAGE_SIZE;
}

// -----------------------------------------------------------------------------

static PageInfo *kernelPDir = nullptr;

#define MAX_PAGE_REGION	16
static PageRegion *pageRegion[MAX_PAGE_REGION] = {0};

// TODO: PAE пока подразумеваем, но ничего для нее не делаем.

typedef uint32_t page_descriptor_t;

static
page_descriptor_t * const stubPageTable =
	(page_descriptor_t *)KERNEL_PAGETABLE_BASE;

static
page_descriptor_t * const stubPageDir =
	(page_descriptor_t *)(KERNEL_PAGETABLE_BASE + KERNEL_PAGETABLE_BASE / PAGE_SIZE * 4);

// Индекс PDir в самом себе -
static
const int stubPdIdx = KERNEL_PAGETABLE_BASE / (PAGE_SIZE * 1024);

void __init__ StubInitPage (void)
{
	stubPageFreeList.first = nullptr;
	stubPageFreeList.count = 0;
	stubPageFreeList.lock = 0;
}

unsigned long StubPageDescriptor (paddr_t addr, unsigned int flags)
{
	STUB_ASSERT (addr >= 0x100000000LL, "Big page addr without PAE");
	STUB_ASSERT (!isAligned(addr, PAGE_SIZE), "Unaligned addr");

	flags |= PFLAG_PRESENT;

	return (unsigned long)(addr | (flags & PFLAG_MASK));
}

// Получить физический адрес по линейному...
paddr_t StubPageGetPAddr (laddr_t laddr)
{
	STUB_ASSERT (!isAligned(laddr, PAGE_SIZE), "Unaligned addr");
	return stubPageTable[laddr / PAGE_SIZE] & PADDR_MASK;
}

PageInfo *StubGetPageByPAddr (paddr_t addr)
{
	STUB_ASSERT (!isAligned(addr, PAGE_SIZE), "Unaligned addr");

	for (int i = 0; i < MAX_PAGE_REGION; i++) {
		if (pageRegion[i] != nullptr &&
		    pageRegion[i]->base <= addr &&
		    pageRegion[i]->base + pageRegion[i]->size > addr)
		{
			const int idx = (addr - pageRegion[i]->base) / PAGE_SIZE;
			return &(pageRegion[i]->pages[idx]);
		}
	}

	return nullptr;
}

paddr_t StubGetPAddrByLAddr (laddr_t addr)
{
	const int idx = addr / PAGE_SIZE;
	STUB_ASSERT (!isSet(stubPageTable[idx], PFLAG_PRESENT), "Missing page");

	return stubPageTable[idx] & PADDR_MASK;
}

PageInfo *StubGetFreePageByPaddr (const paddr_t paddr)
{
	PageInfo * const page = StubGetPageByPAddr (paddr);
	if (page == nullptr)
		return nullptr;

	// TODO: Если страница используется кем бы то ни было
	// и не является статической - ее надо высвобождать.

	if (isSet(page->flags, PFLAG_FREE)) {
		StubPageAllocByInfo(page);
	}

	STUB_ASSERT (page->instances != nullptr, "Used page!");

	if (isSet(page->flags, PFLAG_KERNEL) && !isSet(page->flags, PFLAG_TEMPORARY))
	{
		STUB_ASSERT (isSet(page->flags, PFLAG_STATIC), "Used static page!");
		STUB_FATAL ("TODO: Need free page");
	}

	page->flags |= PFLAG_STATIC;
	return page;
}

const PageInstance *StubGetPageInstance (PageInfo *page, uint32_t access, const void *resource)
{
	STUB_ASSERT (page == nullptr, "nullptr page");

	PageInstance *instance = page->instances;
	while (instance != nullptr) {
		if (instance->resource == resource) {
			// Проверить права доступа, повышать нельзя!
			STUB_ASSERT(isSet(access, RESOURCE_ACCESS_WRITE) &&
				!isSet(instance->flags, PFLAG_WRITABLE), "invalid access");
			return instance;
		}

		instance = instance->next;
	}

	instance = (PageInstance *)StubMemoryAlloc (sizeof (PageInstance));
	STUB_ASSERT (instance == nullptr, "No memory");

	instance->flags = page->flags |
		(isSet(access, RESOURCE_ACCESS_WRITE) ? PFLAG_WRITABLE : 0);
	instance->page = page;
	instance->resource = resource;
	instance->next = page->instances;
	page->instances = instance;
	return instance;
}

void StubPageInstanceDelete(const PageInstance *instance)
{
	STUB_ASSERT(instance == nullptr, "No instance");

	PageInfo *page = instance->page;
	STUB_ASSERT(page->instances == nullptr, "No instances in page");

	// Стереть данную инстанцию из списка.
	PageInstance **ptr = &(page->instances);
	while (true) {
		if (*ptr == instance) {
			*ptr = instance->next;
			break;
		}

		STUB_ASSERT((*ptr)->next == nullptr, "invalid instance");
		ptr = &((*ptr)->next);
	}

	StubMemoryFree((void *)instance);

	if (page->instances == nullptr) {
		StubPageRelease(page);
	}
}

PageInfo *StubGetPageByInstance (const PageInstance *instance)
{
	return instance->page;
}

void __init__ StubCreatePageRegion (paddr_t base, sizex_t size, int flags)
{
	STUB_ASSERT (!isAligned(base, PAGE_SIZE), "Unaligned base");
	STUB_ASSERT (!isAligned(size, PAGE_SIZE), "Unaligned size");
	STUB_ASSERT (size == 0, "Zero size");

	stubMemoryTotal += size;
	stubMemoryUsed += size;

	int idx = -1;
	for (int i = 0; i < MAX_PAGE_REGION; i++) {
		if (pageRegion[i] == nullptr) {
			idx = i;
			break;
		}
	}
	STUB_ASSERT (idx == -1, "All PageRegion slots is busy");

	// Создаем страничный регион
	PageRegion *pregion = (PageRegion *)StubMemoryAlloc (sizeof (PageRegion));
	STUB_ASSERT (pregion == nullptr, "No memory for PageRegion");

	StubMemoryClear (pregion, sizeof (PageRegion));

	pregion->base = base;
	pregion->size = size;
	pregion->flags = flags;

	const size_t pcount = size / PAGE_SIZE;

	// Создаем таблицу страничных инф региона
	pregion->pages = (PageInfo *)StubMemoryAllocAligned (
		sizeof (PageInfo) * pcount, sizeof (PageInfo));
	STUB_ASSERT (pregion->pages == nullptr, "No memory for region PageInfo Table");

	StubMemoryClear (pregion->pages, sizeof (PageInfo) * pcount);

	// Инициализируем страницы.
	for (unsigned int i = 0; i < pcount; i++) {
		pregion->pages[i].paddr = base + i * PAGE_SIZE;
		pregion->pages[i].flags = flags;

		StubPageRelease (&(pregion->pages[i]));
	}

	pageRegion[idx] = pregion;

	CorePrint ("Page region from 0x%016lx %lb success.\n", base, size);
}

// Очень медленная функция, нужна пока не отлажу.
void StubCalcMemoryUsage(struct KernelInfoMemory *info)
{
	info->MemoryTotal = 0;
	info->MemoryUsed = 0;
	info->KernelMemoryUsed = 0;

	for (int i = 0; i < MAX_PAGE_REGION; i++) {
		if (pageRegion[i] == nullptr) continue;

		info->MemoryTotal += pageRegion[i]->size;
		for (unsigned int p = 0; p < pageRegion[i]->size / PAGE_SIZE; p++) {
			const PageInfo *page = &(pageRegion[i]->pages[p]);
			if (page->instances != nullptr || page->kaddr != 0) {
				info->MemoryUsed += PAGE_SIZE;
			}
			if (page->kaddr != 0) {
				info->KernelMemoryUsed += PAGE_SIZE;
			}
		}
	}
}

void __init__ StubPageInitMode ()
{
	PageInfo *pts[stubPdIdx + 1];

	// Инициализацию делаем пока на IA32, насчет PAE будем думать потом.
	kernelPDir = StubPageAlloc();
	StubKernelUsePage (kernelPDir, KERNEL_PAGEDIR, PFLAG_WRITABLE);
	pts[stubPdIdx] = kernelPDir;

	unsigned long *pdir = p2vptr (kernelPDir->paddr);
	StubMemoryClear (pdir, PAGE_SIZE);

	// Заворот каталогов страниц имени меня.
	pdir[stubPdIdx] = StubPageDescriptor(kernelPDir->paddr, kernelPDir->flags);

	// До каталога страниц набиваем таблицы, они статические.
	for (int a = 0; a < KERNEL_PAGETABLE_BASE; a += PAGE_SIZE * 1024) {
		const int ptidx = a / (PAGE_SIZE * 1024);
		pts[ptidx] = StubPageAlloc();
		StubMemoryClear (p2vptr(pts[ptidx]->paddr), PAGE_SIZE);
		StubKernelUsePage (pts[ptidx],
			KERNEL_PAGETABLE_BASE + (a / PAGE_SIZE) * sizeof (unsigned long),
			PFLAG_WRITABLE);
		pdir[ptidx] = StubPageDescriptor(pts[ptidx]->paddr, pts[ptidx]->flags);
	}

	// Теперь все страницы ядра подмапливаем.
	for (int i = 0; i < MAX_PAGE_REGION; i++) {
		if (pageRegion[i] == nullptr)
			continue;

		for (unsigned int pn = 0; pn < pageRegion[i]->size / PAGE_SIZE; pn++) {
			const PageInfo *p = &(pageRegion[i]->pages[pn]);

			if (isSet(p->flags, PFLAG_KERNEL) && p->kaddr < KERNEL_PAGETABLE_BASE)
			{
				STUB_ASSERT (p->kaddr == 0, "No kernel page in kernel");

				const unsigned int kidx = p->kaddr / PAGE_SIZE;
				const PageInfo * const pt = pts[kidx / 1024];
				unsigned long * const ptbl = p2vptr (pt->paddr);
				ptbl[kidx % 1024] = StubPageDescriptor (p->paddr, p->flags);
			}
		}
	}

	// Мапим служебные страницы
	unsigned long *ptbl = p2vptr (pts[0]->paddr);

	// Видеостраницы не входят в набор страниц... поэтому просто мапим
	// Для BSOD нужны три страницы. хотя он мог бы обойтись всего двумя килобайтами bss
	ptbl[VIDEO0_PAGE / PAGE_SIZE % 1024] =
		StubPageDescriptor (VIDEO0_PAGE, PFLAG_WRITABLE | PFLAG_SYSTEM);

	StubInitCR3 (v2laddr(pdir));

	//CorePrint ("Page mode activated, cr3 = 0x%08x.\n", v2laddr(pdir));
}

// Инициализирует каталог страниц на основе текущего -
// Собственно мапит туда таблицы ядра, они неизменны.
void StubPageInitPDir (const laddr_t ldir)
{
	StubMemoryClear (l2vptr(ldir), PAGE_SIZE);

	// Промапить ядро
	StubMemoryCopy (l2vptr(ldir), stubPageDir, stubPdIdx * 4);
}

void StubPageTaskInit (Task *task)
{
	if (task->stack0 == nullptr) {
		// Стек создается только на новой задаче, и существует все время.
		task->stack0 = StubPageAlloc();
	}

	if (task->ptable1023 == nullptr) {
		// Строгая фиксация TASK на ptable1023 - не всегда будет верна.
		task->ptable1023 = StubPageAlloc ();
		const laddr_t ltable = StubPageTemporary (task->ptable1023);

		StubMemoryClear (l2vptr(ltable), PAGE_SIZE);

		page_descriptor_t * const dtable = l2vptr(ltable);
		dtable[ptidx(KERNEL_STACK_BASE) % 1024] =
			StubPageDescriptor (task->stack0->paddr, PFLAG_WRITABLE | PFLAG_SYSTEM);

		StubPageUntemporaryByLAddr (ltable);
	} else {
		// Debug stuff - проверяем соответствие дескриптора страницы.
		const laddr_t ltable = StubPageTemporary (task->ptable1023);
		page_descriptor_t * const dtable = l2vptr(ltable);
		const paddr_t p = dtable[ptidx(KERNEL_STACK_BASE) % 1024] & ~PFLAG_MASK;
		STUB_ASSERT (p != task->stack0->paddr, "Invalid Stack0 descriptor");
		StubPageUntemporaryByLAddr (ltable);
	}

	if (task == bsp && task->pdir != nullptr) {
		// BSP ставит pdir заранее, не обращая внимания на всякие стеки
		// Необходимо реализовать CPU отдельно, и это не будет проблемой.
		// Данный код здесь на правах заглушки.

		const laddr_t ldir = StubPageTemporary (task->pdir);
		page_descriptor_t * const ddir = l2vptr(ldir);

		// А заворот страниц бутстрапу не нужен?
		// нет, наверное в нем не происодить пейджфолтов.

		// Таблица страниц для стека нулевого кольца.
		// Поскольку это BSP, то и доступ исключительно SYSTEM
		ddir[pdidx(KERNEL_STACK_BASE)] =
			StubPageDescriptor (task->ptable1023->paddr, PFLAG_WRITABLE | PFLAG_SYSTEM);

		StubPageUntemporaryByLAddr(ldir);
	}

	if (task->pdir == nullptr) {
		task->pdir = StubPageAlloc ();
		const laddr_t ldir = StubPageTemporary (task->pdir);
		StubPageInitPDir (ldir);

		page_descriptor_t * const ddir = l2vptr(ldir);

		// Заворот страниц имени меня
		ddir[pdidx(KERNEL_PAGETABLE_BASE)] =
			StubPageDescriptor(task->pdir->paddr, PFLAG_SYSTEM);

		// Таблица страниц для стека нулевого кольца.
		ddir[pdidx(KERNEL_STACK_BASE)] =
			StubPageDescriptor (task->ptable1023->paddr, PFLAG_WRITABLE | PFLAG_USER);

		StubPageUntemporaryByLAddr(ldir);
	} else {
		const laddr_t ldir = StubPageTemporary (task->pdir);
		// Debug stuff - проверяем соответствие дескриптора страницы.
		page_descriptor_t * const ddir = l2vptr(ldir);

		const paddr_t pd = ddir[pdidx(KERNEL_PAGETABLE_BASE)] & ~PFLAG_MASK;
		STUB_ASSERT (pd != task->pdir->paddr, "Invalid pagedir pagetable (dronlink) descriptor");

		const paddr_t ps = ddir[pdidx(KERNEL_STACK_BASE)] & ~PFLAG_MASK;
		STUB_ASSERT (ps != task->ptable1023->paddr, "Invalid Stack0 pagetable descriptor");

		StubPageUntemporaryByLAddr(ldir);
	}
}

void StubPageTaskDestroy (Task *task)
{
	if (task->stack0 != nullptr)
		StubPageRelease (task->stack0);

	if (task->ptable1023 != nullptr)
		StubPageRelease (task->ptable1023);

	if (task->pdir != nullptr)
		StubPageRelease (task->pdir);

	task->pdir = nullptr;
	task->ptable1023 = nullptr;
	task->stack0 = nullptr;
}

void StubPageFault (laddr_t laddr, uint32_t error)
{
	if (laddr < KERNEL_PAGETABLE_BASE) {
		// Исключение в ядре.
		// CorePrint ("Kernel Page fault at 0x%08x\n", laddr);

		// Может возникнуть только по причине отсутствия страницы.
		STUB_ASSERT (laddr < v2laddr(&__bss_end), "Kernel pagefault under __bss_end");
		STUB_ASSERT (isSet(error, PFLAG_PRESENT), "Pagefault at present page");

		PageInfo *p = StubPageAlloc ();
		StubKernelUsePage (p, round(laddr, PAGE_SIZE), PFLAG_WRITABLE);

		// Замапить страницу.
		STUB_ASSERT (isSet(stubPageTable[ptidx(laddr)], PFLAG_PRESENT), "Mapped page");
		stubPageTable[ptidx(laddr)] = StubPageDescriptor (p->paddr, p->flags);

		#ifdef PAGE_GARBAGED
			StubMemoryRefuse (l2vptr(round(laddr, PAGE_SIZE)), PAGE_SIZE);
		#endif

		return;
	}

	// Исключение в области нити.
	uint32_t access = RESOURCE_ACCESS_READ;

	if (isSet(error, PFLAG_WRITABLE))
		access |= RESOURCE_ACCESS_WRITE;

	STUB_ASSERT (laddr < USER_MEMORY_BASE && !isSet(error, PFLAG_WRITABLE),
		"pagetable fault for read??");

// 	CorePrint ("User Pagefault at 0x%8x, 0x%03x(0x%03x), task: %08x\n", laddr, access, error, StubGetCurrentTask());

	STUB_ASSERT (access == 0, "Illegal page access");

	// access передается по указателю, и в недах pagefault'a права могут понизится.
	const PageInstance *instance = StubTaskPageFault (laddr, &access);

	STUB_ASSERT (laddr < USER_MEMORY_BASE && !isSet(access, RESOURCE_ACCESS_WRITE),
		"pagetable page without write access");

	const uint32_t pflags = PFLAG_USER |
		(isSet(access, RESOURCE_ACCESS_WRITE) ? PFLAG_WRITABLE : PFLAG_READABLE);

	//CorePrint ("User Pagefault at 0x%8x, grant 0x%03x(0x%03x)\n", laddr, access, pflags);
	STUB_ASSERT (access == 0, "No access for page");

	STUB_ASSERT (instance == nullptr, "No page for user");
	const PageInfo *page = instance->page;
	stubPageTable[ptidx(laddr)] = StubPageDescriptor (page->paddr, pflags);
}

bool StubMemoryReadable (laddr_t addr, size_t size)
{
	if (!StubIsPaged ())
		return true;

	while (true) {
		if (!isSet(stubPageDir[pdidx(addr)], PFLAG_PRESENT))
			return false;

		if (!isSet(stubPageTable[ptidx(addr)], PFLAG_PRESENT))
			return false;

		const size_t page_left = PAGE_SIZE - addr % PAGE_SIZE;
		if (size <= page_left) {
			break;
		}

		addr += page_left;
		size -= page_left;
	}

	return true;
}

// Немного некрасиво, что page передается неконстантно, но ведь она и меняется
// Просто, это вынуждает применять const_cast в коре.
laddr_t StubPageTemporary (PageInfo *page)
{
	if (page->kaddr != 0)
		return page->kaddr;

	//StubLock (&tempotrary);
	for (laddr_t tptr = KERNEL_TEMP_BASE;
		tptr < KERNEL_TEMP_BASE + KERNEL_TEMP_SIZE;
		tptr += PAGE_SIZE)
	{
		const unsigned int tidx = ptidx(tptr);

		if (!isSet(stubPageTable[tidx], PFLAG_PRESENT)) {
			stubPageTable[tidx] = StubPageDescriptor(page->paddr, page->flags);
			page->flags |= PFLAG_TEMPORARY;
			page->kaddr = tptr;
			break;
		}
	}

	return page->kaddr;
}

void StubPageUntemporary (PageInfo *page)
{
	STUB_ASSERT (!isSet(page->flags, PFLAG_TEMPORARY), "Not temporary page");
	STUB_ASSERT (page->kaddr == 0, "Missing temp page addr");

	const laddr_t addr = page->kaddr;

	//StubLock (&tempotrary);
	page->kaddr = 0;
	page->flags &= ~PFLAG_TEMPORARY;
	stubPageTable[ptidx(addr)] = 0;
	// StubUnlock (&temporary);

	StubPageFlush();
}

void StubPageUntemporaryByLAddr (laddr_t addr)
{
	STUB_ASSERT (!isAligned(addr, PAGE_SIZE), "Unaligned temp addr");

	if (addr < KERNEL_TEMP_BASE || KERNEL_TEMP_BASE + KERNEL_TEMP_SIZE <= addr)
		return;	// Обрабатываем только временные страницы.

	paddr_t pa = StubGetPAddrByLAddr (addr);
	PageInfo *page = StubGetPageByPAddr (pa);

	StubPageUntemporary(page);
}
