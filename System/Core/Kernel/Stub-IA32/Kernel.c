//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Page.h"

static int kernelPageCnt = 0;

void __init__ StubInitKernel (void)
{
}

int StubKernelPagesCnt ()
{
	return kernelPageCnt;
}

void StubKernelUsePage (PageInfo *page, laddr_t laddr, int flags)
{
	STUB_ASSERT (page == nullptr, "No page");
	STUB_ASSERT (page->kaddr != 0, "Page already in kernel");
	STUB_ASSERT (laddr >= USER_MEMORY_BASE, "Invalid laddr");
	STUB_ASSERT (!isAligned(laddr, PAGE_SIZE), "Unaligned laddr");

	flags &= ~PFLAG_USER;

	// TODO: Флаг GLOBAL будет притыкаться где-то здесь.
	flags |= PFLAG_KERNEL | PFLAG_SYSTEM;

	if (KERNEL_TEMP_BASE <= laddr && laddr < KERNEL_PAGETABLE_BASE)
		flags |= PFLAG_TEMPORARY;

	page->kaddr = laddr;
	page->flags |= flags;

	// TODO: interlocked
	kernelPageCnt++;
}

void StubKernelUnusePage (PageInfo *page)
{
	// TODO: Необходимо проверить соответствие адреса и страницы.
	STUB_ASSERT(!isSet(page->flags, PFLAG_KERNEL), "Not kernel page");

	// TODO: Это мы так хитро заюзаем StubPageUntemporary
	//	Временный воркэраунд.
	page->flags &= ~PFLAG_KERNEL;
	page->flags |= PFLAG_TEMPORARY;
	StubPageUntemporary(page);

	kernelPageCnt--;

	if (page->instances == 0) StubPageRelease(page);
}

// Эта функция будет очень полезна и потом для освобождения памяти хипа.
void StubKernelDropMemory (laddr_t addr, size_t size)
{
	if (!isAligned(addr, PAGE_SIZE)) {
		const size_t l1 = PAGE_SIZE - addr % PAGE_SIZE;

		if (size <= l1)
			return;	// Нечего отмапливать.

		addr += l1;
		size -= l1;
	}

	while (size >= PAGE_SIZE) {
		paddr_t pa = StubGetPAddrByLAddr(addr);
		PageInfo *page = StubGetPageByPAddr(pa);
		STUB_ASSERT(page->kaddr != addr, "Invalid kernel page");

		StubKernelUnusePage(page);

		addr += PAGE_SIZE;
		size -= PAGE_SIZE;
	}

	StubPageFlush();
}

static
void __init__ StubKernelReservePage (paddr_t paddr, laddr_t laddr, int flags)
{
	PageInfo *page = StubGetPageByPAddr (paddr);
	StubKernelUsePage(page, laddr, flags);
	StubPageAllocByInfo(page);
}

void __init__ StubKernelReservePages (paddr_t paddr, sizex_t size,
	laddr_t laddr, int flags)
{
	CorePrint ("Kernel reserve area 0x%08lx (%lb) at 0x%08x, %s.\n",
		paddr, size, laddr, flags & PFLAG_WRITABLE ? "Read, Write" : "Read");

	if (size == 0) {
		CorePrint ("\t...ignore empty area\n");
		return;
	}

	for (sizex_t offs = 0; offs < size; offs += PAGE_SIZE) {
		StubKernelReservePage (paddr + offs, p2laddr(laddr + offs), flags);
	}
}
