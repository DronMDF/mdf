//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct _PageInfo PageInfo;
typedef struct _PageInstance PageInstance;
typedef struct _Task Task;

// TODO: Надо вообще выкинуть его отсюда.
struct StubStackFrame;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __cplusplus
typedef enum {
	false = 0,
	true = 1,
} bool;
#endif

void StubSoD (const char *msg, const char *file, int line) __noreturn__;

void *StubMemoryAlloc (const size_t size);
void StubMemoryFree (void * const ptr);

void StubMemoryCopy (void *dst, const void *src, size_t count);
void StubMemoryClear (void *dst, size_t count);
void StubMemoryRefuse (void *dst, size_t count);

PageInfo *StubPageAlloc ();
void StubPageInstanceDelete(const PageInstance *instance);

PageInfo *StubGetPageByPAddr (paddr_t addr);
PageInfo *StubGetFreePageByPaddr (paddr_t paddr);

const PageInstance *StubGetPageInstance (PageInfo *Page, uint32_t access, const void *resource);
PageInfo *StubGetPageByInstance (const PageInstance *instance);

laddr_t StubPageTemporary (PageInfo *page);
void StubPageUntemporary (PageInfo *page);
void StubPageUntemporaryByLAddr (laddr_t addr);

// -----------------------------------------------------------------------------
// Задачи.

Task *StubTaskCreate (const laddr_t eip, void *thread);
bool StubTaskDestroy(Task *task);
void StubTaskRun (Task *task);
void *StubTaskGetThread (const Task *task);

// -----------------------------------------------------------------------------
// Стековые фреймы.

// TODO: А что здесь делает стекфрейм??? в Stub, все в Stub...
// Спецальное магическое значение адреса возврата.
#define RETMAGIC	666

void StubSetStackFrame (struct StubStackFrame *frame, id_t caller,
	offset_t txa_offset, size_t txa_size, uint32_t access);

// -----------------------------------------------------------------------------
// Вводо-вывод
void StubPrintChar (const int c);
int StubGetChar (void);

// TODO: Таймстамп каунтер процессора - исключить надо.
clock_t StubGetTimestampCounter ();

void StubLock (lock_t * const lock);
void StubUnlock (lock_t * const lock);

clock_t StubGetCurrentClock ();

// TODO: необходимо заменить на CoreInfoValue
int StubInfoValue(void *info, size_t *info_size, const void *data, size_t data_size);

void StubCPUIdle();

#ifdef __cplusplus
} // extern "C"
#endif
