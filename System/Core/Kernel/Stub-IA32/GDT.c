//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Core.h>

#include "GDT.h"
#include "Memory.h"
#include "Time.h"

extern void __text_end;
extern void __bss_end;
void StubSetGDT (volatile void *gdt, size_t gdt_size);

enum GDT_IDX {
	GDT_CPU_BASE	= 2048,
	GDT_CPU_COUNT	= 2048,
	GDT_TASK_BASE	= 4096,
	GDT_TASK_COUNT	= 4096,
	GDT_SIZE	= 8192,
};

STATIC_ASSERT (GDT_CPU_BASE + GDT_CPU_COUNT == GDT_TASK_BASE);
STATIC_ASSERT (GDT_TASK_BASE + GDT_TASK_COUNT == GDT_SIZE);

static
volatile descriptor_t GDT[GDT_SIZE] __attribute__((aligned(16)));
static
volatile tick_t task_time[GDT_TASK_COUNT] __attribute__((aligned(16)));

void StubSetSegmentCPU(unsigned int ci, laddr_t base, size_t size)
{
	STUB_ASSERT (ci >= GDT_CPU_COUNT, "Invalid CPU slot");

	const unsigned int di = GDT_CPU_BASE + ci;
	STUB_ASSERT (GDT[di].raw != 0, "Busy CPU slot");

	GDT[di] = StubDescriptorGenerate(base, size, DESCRIPTOR_TASK | DESCRIPTOR_PL0);
}

unsigned int StubGetSelectorCPU(unsigned int ci)
{
	STUB_ASSERT (ci >= GDT_CPU_COUNT, "Invalid CPU slot");

	const unsigned int selector = (GDT_CPU_BASE + ci) * sizeof(descriptor_t);
	return selector;
}

static
void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags)
{
	const int di = selector / (int)sizeof(descriptor_t);
	GDT[di] = StubDescriptorGenerate(base, size, flags);
}

void StubSetSegmentTask(unsigned int ti, laddr_t base, size_t size)
{
	STUB_ASSERT (ti >= GDT_TASK_COUNT, "Invalid task no");

	const unsigned int di = GDT_TASK_BASE + ti;
	GDT[di] = StubDescriptorGenerate(base, size, DESCRIPTOR_TASK | DESCRIPTOR_PL0);
}

uint32_t StubGetSelectorTask(const unsigned int ti)
{
	STUB_ASSERT (ti >= GDT_TASK_COUNT, "Invalid task no");

	const unsigned int selector = (GDT_TASK_BASE + ti) * sizeof(descriptor_t);
	return selector;
}

void StubInitGDT()
{
	// Нулевой дескриптор - все понятно...
	// Далее два ядерных дескриптора (код и данные)
	// 4 дескриптора отведем для приложений. (помимо кода и данных там может
	// появиться стек, и еще что нибудь..

	// Итого:
	//  0 - 2047 системные дескрипторы...
	// 2048 - 4095 дескрипторы процессоров (тоже нити только системные)
	// 4096 - 8191 дескрипторы для нитей (слоты)

	StubMemoryClear(&GDT, sizeof(GDT));

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

	StubSetGDT (GDT, sizeof(GDT));

	StubMemoryClear(&task_time, sizeof(task_time));

	CorePrint ("GDT initialized.\n");
}

// Контексты задач, тоже не совсем на месте, но пусть пока тут полежат

#define SLOT_INVALID 0xffffffff

tss_t *StubGetTaskContextBySlot (unsigned int slot)
{
	STUB_ASSERT (slot >= GDT_TASK_COUNT, "Invalid slot");

	const unsigned int di = GDT_TASK_BASE + slot;

	tss_t *tss = l2vptr(StubDescriptorGetBase(GDT[di]));
	STUB_ASSERT (v2laddr(tss) < v2laddr(&__bss_end) || KERNEL_TEMP_BASE <= v2laddr(tss),
		"Invalid TSS");

	STUB_ASSERT (StubDescriptorGetSize(GDT[di]) != offsetof(tss_t, iomap) + tss->iomap_size,
		"Invalid task selector size");

	// TODO: Можно абстракцию дескриптора наградить методами состояний
	//	StubDescriptorIsBusyTask() и тд...
	unsigned int type = StubGetSegmentFlags(GDT[di]) & DESCRIPTOR_TYPE;
	STUB_ASSERT (type != DESCRIPTOR_TASK && type != DESCRIPTOR_TASK_BUSY,
		"Invalid task selector type");

	STUB_ASSERT (tss->slot != slot, "Invalid slot in tss");
	return tss;
}

bool StubTaskSlotCanRelease(unsigned int slot)
{
	STUB_ASSERT (slot >= GDT_TASK_COUNT, "Invalid slot");
	const unsigned int si = GDT_TASK_BASE + slot;
	STUB_ASSERT (GDT[si].raw == 0, "Empty slot release");

	if ((GDT[si].segment.flagslo & DESCRIPTOR_TYPE) == DESCRIPTOR_TASK_BUSY) {
		return false;	// Задача занята
	}

	STUB_ASSERT((GDT[si].segment.flagslo & DESCRIPTOR_TYPE) != DESCRIPTOR_TASK,
		    "Invalid slot type");

	return true;
}

void StubTaskSlotUnuse(unsigned int slot)
{
	STUB_ASSERT (slot >= GDT_TASK_COUNT, "Invalid slot");
	GDT[GDT_TASK_BASE + slot].raw = 0;
	task_time[slot] = 0;
}

bool StubTaskSlotRelease(unsigned int slot);

void StubTaskSlotUse(tss_t *tss)
{
	if (tss->slot == SLOT_INVALID) {
		unsigned int slot = SLOT_INVALID;
		tick_t oldest = TIMESTAMP_FUTURE;
		bool slot_used = false;

		for (unsigned int i = 0; i < GDT_TASK_COUNT; i++) {
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

unsigned int StubTaskSlotBySelector(unsigned int selector)
{
	unsigned int di = selector / sizeof(descriptor_t);
	STUB_ASSERT(di < GDT_TASK_BASE || di >= GDT_TASK_BASE + GDT_TASK_COUNT,
		"Invalid task selector");
	return di;
}
