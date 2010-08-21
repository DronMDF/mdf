//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include "StubLocal.h"

#include "Allocator.h"
#include "Memory.h"

// Третий уровень (каталоги)

AllocDir *StubAllocatorDirectoryAlloc(void *(*getDir)())
{
	AllocDir *dir = getDir();
	StubMemoryClear(dir, sizeof(AllocDir));
	dir->avail = sizeof(dir->pages) / sizeof(dir->pages[0]);
	return dir;
}


// Первый уровень - блоки

void *StubAllocatorAlloc(size_t size, AllocPage **queues, AllocPage *(*newPage)(int))
{
	// Определить свободный блок
	for (unsigned int i = 0; i < PAGE_SIZE / queues[0]->block_size / 32; i++) {
		if (queues[0]->map[i] == 0xffffffff) {
			continue;
		}
		
		for (unsigned int j = 0; j < 32; j++) {
			if ((queues[0]->map[i] & (1U << j)) != 0) {
				continue;
			}
			
			queues[0]->map[i] |= 1U << j;
			const uint32_t offset = (i * 32 + j) * queues[0]->block_size;
			return (void *)(queues[0]->base + offset);
		}
	}
	
	return 0;
}

// По размеру нужно найти очередь, из в которой хранятся соответствующие дескрипторы.
// Очереди у нас начинаются с размера 4
// 4 8 16 32 64 128 256 512 1024 2048 4096
// 0 1  2  3  4   5   6   7    8    9   10	<- индекс очереди

size_t CalcBlockSize(size_t size) 
{
	size--;
	size |= size >> 1;
	size |= size >> 2;
	size |= size >> 4;
	size |= size >> 8;
	size |= size >> 16;
	return max(size + 1, 4);
}

unsigned int GetSizeIndex(size_t size)
{
	for (unsigned int i = 0; i < 10; i++) {
		if (size <= 1U << (i + 2)) {
			return i;
		}
	}
	
	return 10;
}

void StubAllocatorInit(void *block)
{
}

void *StubAlloc(size_t size)
{
	return 0;
}

void StubFree(void *ptr)
{
}
