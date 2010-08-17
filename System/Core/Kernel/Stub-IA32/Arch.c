//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
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
#include "GDT.h"
#include "TSS.h"

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

// -----------------------------------------------------------------------------
// Низкоуровневые функции из Stublo.S

uint32_t StubGetCurrentTaskSelector();
void StubTaskSwitch (uint32_t selector);

void StubSetGDT (volatile void *gdt, size_t gdt_size);

// -----------------------------------------------------------------------------
// Сегментинг (Сегменты, GDT и слоттинг задач, и контексты задач тоже)

volatile descriptor_t *GDT __deprecated__ = NULL;
static volatile tick_t *task_time = NULL;

static
void StubSetSegmentCPU (unsigned int ci, laddr_t base, size_t size)
{
	STUB_ASSERT (ci >= STUB_MAX_CPU_COUNT, "Invalid CPU no");

	const unsigned int di = GDT_CPU_BASE + ci;
	STUB_ASSERT (GDT[di].raw != 0, "Busy CPU slot");

	GDT[di] = StubDescriptorGenerate(base, size, DESCRIPTOR_TASK | DESCRIPTOR_PL0);
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

	const size_t gdt_size = sizeof (descriptor_t) * GDT_SIZE;

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
	const size_t task_time_size = sizeof(tick_t) * STUB_MAX_TASK_COUNT;
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

static
void StubSetSegmentTask (unsigned int ti, laddr_t base, size_t size)
{
	const int flags = DESCRIPTOR_TASK | DESCRIPTOR_PL0;
	const descriptor_t tssd = StubDescriptorGenerate(base, size, flags);
	StubTssSetDescriptor(ti, tssd);
}

static
tss_t *StubGetTaskContextBySlot (unsigned int slot)
{
	const descriptor_t td = StubTssGetDescriptor(slot);

	tss_t *tss = l2vptr(StubDescriptorGetBase(td));
	STUB_ASSERT (v2laddr(tss) < v2laddr(&__bss_end) || KERNEL_TEMP_BASE <= v2laddr(tss),
		"Invalid TSS");

	STUB_ASSERT (StubDescriptorGetSize(td) != offsetof(tss_t, iomap) + tss->iomap_size,
		"Invalid task selector size");

	// TODO: Можно абстракцию дескриптора наградить методами состояний
	//	StubDescriptorIsBusyTask() и тд...
	unsigned int type = StubDescriptorGetFlags(td) & DESCRIPTOR_TYPE;
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
	const descriptor_t td = StubTssGetDescriptor(slot);
	
	STUB_ASSERT (td.raw == 0, "Empty slot release");

	if ((td.segment.flagslo & DESCRIPTOR_TYPE) == DESCRIPTOR_TASK_BUSY) {
		return false;	// Задача занята
	}

	STUB_ASSERT((td.segment.flagslo & DESCRIPTOR_TYPE) != DESCRIPTOR_TASK, 
		    "Invalid slot type");

	tss_t *tss = StubGetTaskContextBySlot(slot);
	STUB_ASSERT(tss->slot != slot, "Invalid slot");
	tss->slot = SLOT_INVALID;

	StubTssClearDescriptor(slot);
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

		for (unsigned int i = 0; i < STUB_MAX_TASK_COUNT; i++) {
			if (StubTssIsAvail(i)) {
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

	context->cr3 = p2laddr(pdir->paddr);
}

Task *StubGetCurrentTask ()
{
	const unsigned int selector = StubGetCurrentTaskSelector();
	const unsigned int slot = StubTssGetSlot(selector);
	return StubGetTaskBySlot(slot);
}

void StubTaskExecute (const Task *task)
{
	tss_t *context = task->context;
	StubTaskSlotUse (context);
	StubTaskSwitch (StubTssGetSelector(context->slot));
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
