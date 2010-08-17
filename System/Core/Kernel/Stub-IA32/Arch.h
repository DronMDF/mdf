//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

uint32_t StubGetSelectorCPU (const unsigned int ci);
uint32_t StubGetSelectorTask (const unsigned int ti);

// Возможности CPU
bool StubCPUHasTSC ();
bool StubCPUHasAPIC ();

int StubCurrentCPUNumber (void);

void *StubTaskContextCreate (const Task *task, const uaddr_t eip);
bool StubTaskContextDestroy(const Task *task);
void StubTaskContextSetPDir (const Task *task, const PageInfo *pdir);
Task *StubGetCurrentTask ();
void StubTaskExecute (const Task *task);

void StubTaskBootstrapCreate();

void StubInitGDT ();
void StubInitIDT ();

tick_t StubGetTSC (void);
