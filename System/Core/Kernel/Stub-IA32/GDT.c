//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include "StubLocal.h"
#include "GDT.h"
#include "Memory.h"

// GDT utility
extern void __text_end;

volatile descriptor_t GDT[GDT_SIZE] __attribute__((aligned(16)));

void StubSetGDT (volatile void *gdt, size_t gdt_size);

static
void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags)
{
	const int di = selector / (int)sizeof(descriptor_t);
	GDT[di] = StubDescriptorGenerate(base, size, flags);
}

void __init__ StubInitGDT()
{
	StubMemoryClear(&GDT, sizeof(GDT));

	StubSetSegmentDescriptorBySelector(KERNEL_CODE_SELECTOR,
		0, (size_t)&__text_end,
		DESCRIPTOR_CODE | DESCRIPTOR_USE32 | DESCRIPTOR_PL0);
	StubSetSegmentDescriptorBySelector(KERNEL_DATA_SELECTOR, 0, 0,
		DESCRIPTOR_DATA | DESCRIPTOR_USE32 | DESCRIPTOR_PL0);

	StubSetSegmentDescriptorBySelector(USER_CODE_SELECTOR,
		USER_MEMORY_BASE, USER_CODE_SIZE,
		DESCRIPTOR_CODE | DESCRIPTOR_USE32 | DESCRIPTOR_PL3);
	StubSetSegmentDescriptorBySelector(USER_DATA_SELECTOR,
		USER_MEMORY_BASE, USER_MEMORY_SIZE,
		DESCRIPTOR_DATA | DESCRIPTOR_USE32 | DESCRIPTOR_PL3);

	StubSetGDT(&GDT, sizeof(GDT));

	CorePrint("GDT initialized.\n");
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
	if (slot < GDT_TASK_BASE || slot >= GDT_TASK_BASE + STUB_MAX_TASK_COUNT) {
		return SLOT_INVALID;
	}
	return slot - GDT_TASK_BASE;
}
