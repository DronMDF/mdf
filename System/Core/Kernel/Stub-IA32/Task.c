//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Arch.h"
#include "Task.h"
#include "Page.h"

// -----------------------------------------------------------------------------
// Управление задачами...

// Пока задача у нас одна, потом это возможно будет массив CPU-тасков.
Task *bsp = 0;

// Временно размещу здесь...
#define PTABLE1023 (KERNEL_PAGETABLE + \
	((0x100000000ULL - 0x400000) / PAGE_SIZE) * sizeof (unsigned long))

Task *StubTaskCreate (const uaddr_t eip, void *thread)
{
	Task *task = StubMemoryAlloc (sizeof (Task));
	if (task == nullptr)
		return nullptr;

	StubMemoryClear (task, sizeof (Task));

	task->context = StubTaskContextCreate(task, eip);
	if (task->context == nullptr) {
		StubMemoryFree (task);
		return nullptr;
	}

	task->thread = thread;
	return task;
}

bool StubTaskDestroy (Task *task)
{
	STUB_ASSERT (task == nullptr, "Null task");

	if (!StubTaskContextDestroy(task))
		return false;

	StubPageTaskDestroy (task);
	StubMemoryFree (task);
	return true;
}

// Прокачивает задачу недостающими страницами...
void StubTaskWakeup (Task *task)
{
	// Эта функция создает минимально необходимые для запуска нити страницы.
	// В IA32 без PAE их всего три.
	StubPageTaskInit (task);

	StubTaskContextSetPDir (task, task->pdir);
}

void *StubTaskGetThread (const Task *task)
{
	if (task == nullptr)
		return nullptr;

	return task->thread;
}

void StubSetStackFrame (struct StubStackFrame *frame, id_t caller,
	offset_t txa_offset, size_t txa_size, int flags)
{
	frame->flags		= flags;
	frame->txa_size 	= txa_size;
	frame->txa_ptr		= l2uaddr(USER_TXA_BASE + txa_offset);
	frame->caller		= caller,
	frame->retmagic		= RETMAGIC;
}

void StubTaskRun (Task *task)
{
	STUB_ASSERT (task->context == nullptr, "Uninitialized TSS");

	StubTaskWakeup (task);
	StubTaskExecute (task);
}

const PageInstance *StubTaskPageFault (laddr_t addr, uint32_t *access)
{
	STUB_ASSERT (addr >= KERNEL_STACK_BASE, "User Memory overrun");
	STUB_ASSERT (addr < USER_PAGETABLE_BASE, "User Memory underrun");

	Task *current = StubGetCurrentTask();
	STUB_ASSERT (current == nullptr, "No user task");

	CorePrint("PageFault 0x%08x (0x%08x)\n", current, addr);
	
	const PageInstance *instance = CoreThreadPageFault (current, addr, access);

	// Может быть и нарушение доступа.
	return instance;
}
