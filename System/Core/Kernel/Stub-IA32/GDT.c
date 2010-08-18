//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include "GDT.h"

// GDT utility

void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags)
{
	const int di = selector / (int)sizeof(descriptor_t);
	GDT[di] = StubDescriptorGenerate(base, size, flags);
}


// CPU utility

void StubCpuSetDescriptor(unsigned int slot, const descriptor_t desc)
{
	STUB_ASSERT(slot >= STUB_MAX_CPU_COUNT, "Invalid CPU slot");
	STUB_ASSERT(GDT[GDT_CPU_BASE + slot].raw != 0, "Busy CPU slot");
	GDT[GDT_CPU_BASE + slot] = desc;
}

// TSS utility

void StubTssSetDescriptor(unsigned int slot, const descriptor_t desc)
{
	STUB_ASSERT(slot >= STUB_MAX_TASK_COUNT, "Invalid TSS slot");
	GDT[GDT_TASK_BASE + slot] = desc;
}

descriptor_t StubTssGetDescriptor(unsigned int slot)
{
	STUB_ASSERT(slot >= STUB_MAX_TASK_COUNT, "Invalid TSS slot");
	return GDT[GDT_TASK_BASE + slot];
}

void StubTssClearDescriptor(unsigned int slot)
{
	STUB_ASSERT(slot >= STUB_MAX_TASK_COUNT, "Invalid TSS slot");
	GDT[GDT_TASK_BASE + slot].raw = 0;
}

bool StubTssIsAvail(unsigned int slot)
{
	STUB_ASSERT(slot >= STUB_MAX_TASK_COUNT, "Invalid TSS slot");
	return GDT[GDT_TASK_BASE + slot].raw == 0;
}

unsigned int StubTssGetSelector(unsigned int slot)
{
	STUB_ASSERT (slot >= STUB_MAX_TASK_COUNT, "Invalid TSS slot");
	return (GDT_TASK_BASE + slot) * sizeof(descriptor_t);
}

unsigned int StubTssGetSlot(unsigned int selector)
{
	unsigned int slot = selector / sizeof (descriptor_t);
	STUB_ASSERT(slot < GDT_TASK_BASE || slot >= GDT_TASK_BASE + STUB_MAX_TASK_COUNT,
		    "Invalid TSS selector");
	return slot - GDT_TASK_BASE;
}
