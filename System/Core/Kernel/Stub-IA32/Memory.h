//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

struct KernelInfoMemoryStat;

void StubMemoryCopy(void *dst, const void *src, size_t count);
bool StubMemoryEqual(const void *dst, const void *src, size_t count);
void StubMemoryClear(void *dst, size_t count);
void StubMemoryRefuse(void *dst, size_t count);

void StubMemoryInitBoot(void *ptr, size_t size);
void StubMemoryInitWork(void *ptr, size_t size);

void *StubMemoryAlloc(size_t size);
void *StubMemoryAllocAligned(size_t size, int align);
void StubMemoryFree(void *ptr);

void StubCalcHeapUsage(struct KernelInfoMemory *info);
sizex_t StubMemoryReserve();
