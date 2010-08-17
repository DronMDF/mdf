//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// Эта структура - Является ключевой для Stub, поэтому она здесь.
// TODO: А вот структуру PageInfo стоит запрятать поглубже в Page.c
struct TaskContext;

struct _Task {
	int state;

	int flags;

	// Линейные адреса у этих трех страниц фиксированные.
	PageInfo *pdir;		// KERNEL_PAGEDIR
	PageInfo *ptable1023;	// PTABLE1023
	PageInfo *stack0;	// STACK0

	lock_t	lock;

	void *context;
	void *thread;
};

void StubTaskWakeup (Task *task);

Task *StubGetCurrentTask ();
const PageInstance *StubTaskPageFault (laddr_t addr, uint32_t *access);

extern Task *bsp;
