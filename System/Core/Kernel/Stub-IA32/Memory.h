//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

struct KernelInfoMemory;

void StubMemoryCopy(void *dst, const void *src, size_t count);
bool StubMemoryEqual(const void *dst, const void *src, size_t count);
void StubMemoryClear(void *dst, size_t count);
void StubMemoryRefuse(void *dst, size_t count);

void StubMemoryInitBoot(void *ptr, size_t size);
void StubMemoryInitWork(void *ptr, size_t size);

void *StubMemoryAlloc(size_t size);
void *StubMemoryAllocAligned(size_t size, unsigned int align);
void StubMemoryFree(void *ptr);

void StubCalcHeapUsage(struct KernelInfoMemory *info);
sizex_t StubMemoryReserve();
