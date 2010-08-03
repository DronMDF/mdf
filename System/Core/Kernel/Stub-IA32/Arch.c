//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"

#include "Arch.h"
#include "Time.h"
#include "Memory.h"
#include "Task.h"
#include "Page.h"
#include "Descriptor.h"

extern void __init_begin;
extern void __init_ro;
extern void __init_end;
extern void __text_begin;
extern void __text_end;
extern void __rodata_begin;
extern void __rodata_end;
extern void __data_begin;
extern void __data_end;
extern void __bss_begin;
extern void __bss_end;

#define offsetof(type, field)  ((unsigned long)(&(((type *)0)->field)))

// -----------------------------------------------------------------------------
// Низкоуровневые функции из Stublo.S

uint32_t StubGetCurrentTaskSelector();
void StubTaskSwitch (uint32_t selector);

void StubSetGDT (volatile void *gdt, size_t gdt_size);

// -----------------------------------------------------------------------------
// Сегментинг (Сегменты, GDT и слоттинг задач, и контексты задач тоже)

#define STUB_MAX_TASK_COUNT	4096
#define STUB_MAX_CPU_COUNT	2048

volatile descriptor_t *GDT = NULL;
static volatile tick_t *task_time = NULL;

enum GDT_IDX {
	GDT_CPU_BASE	= 2048,
	GDT_TASK_BASE	= 4096,
	GDT_SIZE	= 8192,
};

STATIC_ASSERT (GDT_CPU_BASE + STUB_MAX_CPU_COUNT == GDT_TASK_BASE);
STATIC_ASSERT (GDT_TASK_BASE + STUB_MAX_TASK_COUNT == GDT_SIZE);

static
void StubSetSegmentCPU (unsigned int ci, laddr_t base, size_t size)
{
	STUB_ASSERT (ci >= STUB_MAX_CPU_COUNT, "Invalid CPU no");

	const int di = GDT_CPU_BASE + ci;
	STUB_ASSERT (GDT[di].raw != 0, "Busy CPU slot");

	GDT[di] = StubGenerateSegmentDescriptor(base, size, DESCRIPTOR_TASK | DESCRIPTOR_PL0);
}

uint32_t StubGetSelectorCPU (const unsigned int ci)
{
	STUB_ASSERT (ci >= STUB_MAX_CPU_COUNT, "Invalid CPU no");

	const unsigned int selector = (GDT_CPU_BASE + ci) * sizeof(descriptor_t);
	return selector;
}

void __init__ StubInitGDT ()
{
	// Нулевой дескриптор - все понятно...
	// Далее два ядерных дескриптора (код и данные)
	// 4 дескриптора отведем для приложений. (помимо кода и данных там может
	// появиться стек, и еще что нибудь..

	// Итого:
	//  0 - 2047 системные дескрипторы...
	// 2048 - 4095 дескрипторы процессоров (тоже нити только системные)
	// 4096 - 8191 дескрипторы для нитей (слоты)

	const int gdt_size = sizeof (descriptor_t) * GDT_SIZE;

	GDT = StubMemoryAllocAligned (gdt_size, sizeof (descriptor_t));
	// FIXME: RELEASE
	STUB_ASSERT (GDT == nullptr, "Cannot alloc GDT");

	StubMemoryClear ((descriptor_t *)GDT, gdt_size);

	StubSetSegmentDescriptorBySelector (KERNEL_CODE_SELECTOR,
		0, (size_t)&__text_end,
		DESCRIPTOR_CODE | DESCRIPTOR_USE32 | DESCRIPTOR_PL0);
	StubSetSegmentDescriptorBySelector (KERNEL_DATA_SELECTOR, 0, 0,
		DESCRIPTOR_DATA | DESCRIPTOR_USE32 | DESCRIPTOR_PL0);

	StubSetSegmentDescriptorBySelector (USER_CODE_SELECTOR,
		USER_MEMORY_BASE, USER_CODE_SIZE,
		DESCRIPTOR_CODE | DESCRIPTOR_USE32 | DESCRIPTOR_PL3);
	StubSetSegmentDescriptorBySelector (USER_DATA_SELECTOR,
		USER_MEMORY_BASE, USER_MEMORY_SIZE,
		DESCRIPTOR_DATA | DESCRIPTOR_USE32 | DESCRIPTOR_PL3);

	StubSetGDT (GDT, gdt_size);

	// Выделить память для таймеров слотов задач
	const int task_time_size = sizeof(tick_t) * STUB_MAX_TASK_COUNT;
	task_time = StubMemoryAllocAligned(task_time_size, sizeof(tick_t));
	STUB_ASSERT (task_time == nullptr, "No memory for task slot time");

	// Чистить эту память в принципе не обязательно, но для порядку почистим.
	StubMemoryClear((tick_t *)task_time, task_time_size);

	CorePrint ("GDT initialized.\n");
}

// -----------------------------------------------------------------------------
// Таск контекстинг и слоттинг

// -----------------------------------------------------------------------------
// Здесь платформенный контекст задачи.

// Здесь теперь открываются широкие возможности по оптимизации.
// Кому, как не Arch.c лучше всего понятен смысл слова slot.
// Даже более того, нету необходимости держать список задач по слотам.
// Эту информацию легко можно достать из дескриптора.

// Единственное, что не очень то получится выкинуть - это время использования
// слота. Его можно было бы хранить в контекстах задач, но доставатьвсе задачи
// из дескриптора для того чтобы определить какая из них дольше не
// использовалась - будет накладно.

// Вообще в задачах такой счетсик даже есть. :) но доставать его долго.
// Значит таблица с временем использования слотов - нужна.

typedef struct {
	unsigned long link;
	unsigned long esp0, ss0;
	unsigned long esp1, ss1;
	unsigned long esp2, ss2;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned long es;
	unsigned long cs;
	unsigned long ss;
	unsigned long ds;
	unsigned long fs;
	unsigned long gs;
	unsigned long ldt;
	unsigned short trace;
	unsigned short iomap_offset;

	size_t iomap_size;
	Task *task;
	unsigned int slot;

	unsigned char iomap[];
} __attribute__ ((packed)) tss_t;

STATIC_ASSERT (offsetof(tss_t, iomap_size) == 104);

// Биты eflags
enum {
	EFLAGS_INTERRUPT_ENABLE = (1 << 9),
};

static
void StubSetSegmentTask (unsigned int ti, laddr_t base, size_t size)
{
	STUB_ASSERT (ti >= STUB_MAX_TASK_COUNT, "Invalid task no");

	const int di = GDT_TASK_BASE + ti;
	GDT[di] = StubGenerateSegmentDescriptor(base, size, DESCRIPTOR_TASK | DESCRIPTOR_PL0);
}

uint32_t StubGetSelectorTask (const unsigned int ti)
{
	STUB_ASSERT (ti >= STUB_MAX_TASK_COUNT, "Invalid task no");

	const unsigned int selector = (GDT_TASK_BASE + ti) * sizeof(descriptor_t);
	return selector;
}

static
tss_t *StubGetTaskContextBySlot (unsigned int slot)
{
	STUB_ASSERT (slot >= STUB_MAX_TASK_COUNT, "Invalid slot");

	const unsigned int di = GDT_TASK_BASE + slot;

	tss_t *tss = l2vptr(StubGetSegmentBase (di));
	STUB_ASSERT (v2laddr(tss) < v2laddr(&__bss_end) || KERNEL_TEMP_BASE <= v2laddr(tss),
		"Invalid TSS");

	STUB_ASSERT (StubGetSegmentSize (di) != offsetof (tss_t, iomap) + tss->iomap_size,
		"Invalid task selector size");

	unsigned int type = StubGetSegmentFlags(di) & DESCRIPTOR_TYPE;
	STUB_ASSERT (type != DESCRIPTOR_TASK && type != DESCRIPTOR_TASK_BUSY,
		"Invalid task selector type");

	STUB_ASSERT (tss->slot != slot, "Invalid slot in tss");
	return tss;
}

static
Task *StubGetTaskBySlot (unsigned int slot)
{
	const tss_t *tss = StubGetTaskContextBySlot(slot);
	Task *task = tss->task;
	STUB_ASSERT (v2laddr(task) < v2laddr(&__bss_end) || KERNEL_TEMP_BASE <= v2laddr(task),
		"Invalid Task");

	return task;
}

// -----------------------------------------------------------------------------
// Менеджер распределения слотов для задач.

#define SLOT_INVALID	0xffffffff

static
bool StubTaskSlotRelease(unsigned int slot)
{
	STUB_ASSERT (slot >= STUB_MAX_TASK_COUNT, "Invalid slot");
	STUB_ASSERT (GDT[GDT_TASK_BASE + slot].raw == 0, "Empty slot release");

	if ((GDT[GDT_TASK_BASE + slot].segment.flagslo & DESCRIPTOR_TYPE) ==
			DESCRIPTOR_TASK_BUSY)
	{
		return false;	// Задача занята
	}

	STUB_ASSERT((GDT[GDT_TASK_BASE + slot].segment.flagslo & DESCRIPTOR_TYPE) !=
			DESCRIPTOR_TASK, "Invalid slot type");

	tss_t *tss = StubGetTaskContextBySlot(slot);
	STUB_ASSERT(tss->slot != slot, "Invalid slot");
	tss->slot = SLOT_INVALID;

	GDT[GDT_TASK_BASE + slot].raw = 0;
	task_time[slot] = 0;

	return true;
}

static
void StubTaskSlotUse(tss_t *tss)
{
	if (tss->slot == SLOT_INVALID) {
		unsigned int slot = SLOT_INVALID;
		tick_t oldest = TIMESTAMP_FUTURE;
		bool slot_used = false;

		for (int i = 0; i < STUB_MAX_TASK_COUNT; i++) {
			if (GDT[GDT_TASK_BASE + i].raw == 0) {
				slot = i;
				slot_used = false;
				break;
			}

			if (task_time[i] < oldest) {
				slot = i;
				oldest = task_time[i];
				slot_used = true;
			}
		}

		STUB_ASSERT (slot == SLOT_INVALID, "Impossible, no slot!");
		if (slot_used) {
			StubTaskSlotRelease(slot);
		}

		tss->slot = slot;
		StubSetSegmentTask (slot, v2laddr(tss),
			offsetof (tss_t, iomap) + tss->iomap_size);
	}

	task_time[tss->slot] = StubGetTimestampCounter();
}

// -----------------------------------------------------------------------------
// Контекст задачи

// task сюда передается исключительно для обратной ссылки.
void *StubTaskContextCreate (const Task *task, const uaddr_t eip)
{
	tss_t *context = StubMemoryAlloc (sizeof (tss_t));
	if (context == nullptr)
		return nullptr;

	StubMemoryClear (context, sizeof (tss_t));

	// Последняя страница памяти
	context->esp0 = 0;
	context->ss0 = KERNEL_DATA_SELECTOR;

	// Прерывания разрешены.
	context->eflags = EFLAGS_INTERRUPT_ENABLE;

	// Приложения всегда стартуют из пользовательског опространства.
	// TODO: А как же быть с ядерными тасками? нужен флаг.
	context->eip = eip;
	context->cs = USER_CODE_SELECTOR;

	context->es = USER_DATA_SELECTOR;
	context->ss = USER_DATA_SELECTOR;
	context->ds = USER_DATA_SELECTOR;
	context->fs = USER_DATA_SELECTOR;
	context->gs = USER_DATA_SELECTOR;

	context->esp = l2uaddr(USER_STACK_BASE + USER_STACK_SIZE - sizeof(struct StubStackFrame));

	context->iomap_offset = offsetof (tss_t, iomap);

	context->task = (Task *)task;	// const_cast<Task *>(task)
	context->slot = SLOT_INVALID;

	return context;
}

bool StubTaskContextDestroy(const Task *task)
{
	tss_t *context = task->context;
	if (context->slot != SLOT_INVALID) {
		if (!StubTaskSlotRelease (context->slot))
			return false;	// Задача занята
	}

	StubMemoryFree (context);
	return true;
}

void StubTaskContextSetPDir (const Task *task, const PageInfo *pdir)
{
	STUB_ASSERT (task == nullptr, "Missing task");
	tss_t *context = task->context;

	STUB_ASSERT (context == nullptr, "Missing TSS");
	STUB_ASSERT (pdir->paddr >= 0x100000000ULL, "Big paddr for IA32 cr3");

	context->cr3 = pdir->paddr;
}

Task *StubGetCurrentTask ()
{
	unsigned int selector = StubGetCurrentTaskSelector();

	unsigned int di = selector / sizeof (descriptor_t);
	if (di < GDT_TASK_BASE || GDT_TASK_BASE + STUB_MAX_TASK_COUNT <= di)
		return nullptr;	// Возможно CPU... но не Task.

	return StubGetTaskBySlot(di - GDT_TASK_BASE);
}

void StubTaskExecute (const Task *task)
{
	tss_t *context = task->context;
	StubTaskSlotUse (context);
	StubTaskSwitch (StubGetSelectorTask(context->slot));
}

// -----------------------------------------------------------------------------
// Здесь логика для CPU.
// TODO: Может быть стоит логически разделить CPU и Task'и?
// Тем более, что CPU будут проще.

// static
// uint64_t StubGetCPUHz ()
// {
// 	const uint32_t us = 10000;
//
// 	const tick_t start_tsc = StubGetTimestampCounter();
// 	StubMicroSleep (us);
// 	const tick_t end_tsc = StubGetTimestampCounter();
//
// 	return (end_tsc - start_tsc) * 1000000 / us;
// }

void StubTaskBootstrapCreate ()
{
	// Измеряем скорость проца, ради эксперимента. :)
	// Заодно MicroSleep проверим.
// 	uint64_t hz = 0;
//
// 	for (int i = 0; i < 3; i++) {
// 		const uint64_t chz = StubGetCPUHz();
// 		if (hz < chz)
// 			hz = chz;
// 	}
//
// 	hz /= 1024;
// 	if (hz < 1024 * 10) {
// 		CorePrint ("CPU0: cpu speed: %luKHz. LOL this is a bochs!\n", hz);
// 	} else {
// 		hz /= 1024;
// 		CorePrint ("CPU0: cpu speed: %luMHz\n", hz);
// 	}

	bsp = StubTaskCreate(0, 0);

	// В качестве pdir используется текущий каталог страниц
	paddr_t pdaddr = StubGetCR3();
	bsp->pdir = StubGetPageByPAddr (pdaddr);

	StubTaskWakeup (bsp);

	StubPageFlush ();

	// Предел сегмента ставтся без служебных полей, iomap пароцессору не нужен.
	// TODO: Этому здесь не место. надо тоже в Arch.c
	tss_t *tss = bsp->context;
	StubSetSegmentCPU (0, v2laddr(tss), offsetof (tss_t, iomap_size));
}

// -----------------------------------------------------------------------------
// Интерфейс ядра

// TODO: Этим методам здесь не место, но пока пусть будут тут
int StubWait (id_t id, int event, timeout_t timeout)
{
	const Task *task = StubGetCurrentTask();
	return CoreWait(task, id, event, timeout);
}

int StubFind (const uaddr_t name_ptr __unused__, size_t name_size __unused__,
		const uaddr_t id_ptr __unused__)
{
	STUB_FATAL ("Under constructed");
	return -1;
}

int StubCreate (const int type, const uaddr_t param_ptr,
		const size_t param_size, const uaddr_t id_ptr)
{
	if (param_ptr + param_size > USER_MEMORY_SIZE ||
	    id_ptr + sizeof (id_t) > USER_MEMORY_SIZE)
	{
		return ERROR_INVALIDPARAM;
	}

	laddr_t param = u2laddr(param_ptr);
	if (!StubMemoryReadable (param, param_size))
		return ERROR_INVALIDPARAM;

	laddr_t id = u2laddr(id_ptr);
	const Task *task = StubGetCurrentTask();
	// return CoreCreate (task, type, l2vptr(param), param_size, l2vptr(id));
	int rv = CoreCreate (task, type, l2vptr(param), param_size, l2vptr(id));

	if (rv != SUCCESS) {
		CorePrint ("CoreCreate fail: %u\n", rv);
	}

	return rv;
}

int StubCall (id_t id, uaddr_t param, size_t size, int flags)
{
	if (param + size > USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	laddr_t paddr = u2laddr(param);
	if (!StubMemoryReadable(paddr, size))
		return ERROR_INVALIDPARAM;

	const Task *task = StubGetCurrentTask();
	int rv = CoreCall(task, id, l2vptr(paddr), size, flags);

	if (rv != SUCCESS) {
		CorePrint ("CoreCall fail: %u\n", rv);
	}

	return rv;
}

int StubAttach (const id_t rid, const id_t pid, const int access, const uint32_t specific)
{
	const Task *task = StubGetCurrentTask();

	//return CoreAttach (task, rid, pid, access, specific);
	int rv = CoreAttach (task, rid, pid, access, specific);

	if (rv != SUCCESS)
		CorePrint ("CoreAttach fail: %u\n", rv);

	return rv;
}

int StubDetach (const id_t id, const int flags)
{
	const Task *task = StubGetCurrentTask();
	
	const int rv = CoreDetach(task, id, flags);
	if (rv != SUCCESS) {
		CorePrint("CoreDetach fail: %u\n", rv);
	}
	
	return rv;
}

int StubModify (const id_t id, const int modify_id,
		const uaddr_t param_ptr, const size_t param_size)
{
	if (param_ptr + param_size > USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	laddr_t param = u2laddr(param_ptr);
	if (!StubMemoryReadable (param, param_size))
		return ERROR_INVALIDPARAM;

	const Task *task = StubGetCurrentTask();

	//return CoreModify (task, id, modify_id, l2vptr(param), param_size);

	// Вывод информации о сбое
	const int rv = CoreModify(task, id, modify_id, l2vptr(param), param_size);
	if (rv != SUCCESS) {
		CorePrint("CoreModify(0x%08x) fail: %u\n", modify_id, rv);
	}

	return rv;
}

int StubInfoMemory(void *info, size_t *info_size)
{
// 	struct KernelInfoMemory minfo = {
// 		.MemoryTotal = StubGetMemoryTotal(),
// 		.MemoryUsed = StubGetMemoryUsed(),
// 		.KernelMemoryUsed = StubKernelPagesCnt() * PAGE_SIZE,
// 		.KernelHeapTotal = StubMemoryReserve(),
// 		.KernelHeapUsed = 0,
// 	};

	// Это очень медленный способ опрделения памяти ядра.
	// введен потому, что вышеупомянутые функции врут.
	// Потом я здесь еще проверки вставлю.
	struct KernelInfoMemory minfo2;
	StubCalcMemoryUsage(&minfo2);
	StubCalcHeapUsage(&minfo2);

	return StubInfoValue(info, info_size, &minfo2, sizeof(struct KernelInfoMemory));
}

int StubInfo (id_t id, int info_id, uaddr_t info_ptr, uaddr_t info_size_ptr)
{
	if (info_ptr >= USER_MEMORY_SIZE || info_size_ptr >= USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	if (info_size_ptr == 0)
		return ERROR_INVALIDPARAM;

	void *info = nullptr;
	if (info_ptr != 0)
		info = l2vptr(u2laddr(info_ptr));

	size_t *info_size = nullptr;
	if (info_size_ptr != 0)
		info_size = l2vptr(u2laddr(info_size_ptr));

	switch (info_id) {
		case RESOURCE_INFO_STUB_VERSION: {
			const uint32_t version = 1;
			return StubInfoValue(info, info_size, &version, sizeof(uint32_t));
		}

		case RESOURCE_INFO_MEMORY:
			return StubInfoMemory(info, info_size);

		default:
			break;
	}

	const Task *task = StubGetCurrentTask();
	// return CoreInfo(task, id, info_id, info, info_size);

	int rv = CoreInfo(task, id, info_id, info, info_size);
	if (rv != SUCCESS) {
		CorePrint ("CoreInfo(0x%08x) fail: %u\n", info_id, rv);
	}

	return rv;
}


// -----------------------------------------------------------------------------
// API utility

int StubInfoValue (void *info, size_t *info_size, const void *data, size_t data_size)
{
	if (info_size == nullptr)
		return ERROR_INVALIDPARAM;

	if (info != nullptr)
		StubMemoryCopy(info, data, min(*info_size, data_size));

	*info_size = data_size;
	return SUCCESS;
}

// -----------------------------------------------------------------------------
// Разное низкоуровневое

enum {
	CPU_HAS_TSC		= (1 << 4),
	CPU_HAS_LOCAL_APIC	= (1 << 9),
};

unsigned long StubGetCPUFutures (void);

bool StubCPUHasTSC ()
{
	return isSet(StubGetCPUFutures(), CPU_HAS_TSC);
}

bool StubCPUHasAPIC ()
{
	return isSet(StubGetCPUFutures(), CPU_HAS_LOCAL_APIC);
}

int StubCurrentCPUNumber (void)
{
	int cpuno = 0;

	if (StubCPUHasAPIC()) {
		CorePrint ("APIC temporary is not supported...\n");
	}

	return cpuno;
}

void StubCPUIdle()
{
	// Эта функция работает на текущем CPU. Пока мы поддерживаем только один.
	const unsigned int bsp_selector = StubGetSelectorCPU(0);

	if (StubGetCurrentTaskSelector() == bsp_selector) return;
	StubTaskSwitch(bsp_selector);
}
