//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
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

// Второй уровень - страницы

AllocPage *StubAllocatorNewPage(size_t size)
{
	return 0;
}

int StubAllocatorFindBlock(AllocPage *page)
{
	const int bpp = (int)(PAGE_SIZE / page->block_size);
	const int bpm = sizeof(page->map[0]) * 8;
	
	CorePrint("bpp: %d, bpm: %d\n", bpp, bpm);
	
	for (int i = 0; i < max(bpp / bpm, 1); i++) {
		CorePrint("map[%d]: 0x%08x\n", i, page->map[i]);
		if (page->map[i] == 0xffffffff) {
			continue;
		}

		for (int j = 0; j < bpm; j++) {
			if ((page->map[i] & (1U << j)) == 0) {
				return (i * bpm + j);
			}
		}
	}

	return -1;
}

void StubAllocatorMarkBlock(AllocPage *page, int idx)
{
	STUB_ASSERT(idx >= (int)(PAGE_SIZE / page->block_size), "Invalid block index");
	page->map[idx / 32] |= 1U << (idx % 32);

//	Неблокирующий алгоритм...
// 	const int index = idx / 32;
// 	do {
// 		const uint32_t old_value = page->map[index];
// 		const uint32_t new_value = old_value | 1U << (idx % 32);
// 	} while (CAS(&(page->map[index]), old_value, new_value));
}

void *StubAllocatorPageGetBlock(AllocPage *page)
{
	// Это надо будет все заинлайнить и привести к нормальному неблокируемому виду
	int idx = StubAllocatorFindBlock(page);
	if (idx >= 0) {
		StubAllocatorMarkBlock(page, idx);
		return (void *)(page->base + page->block_size * (size_t)idx);
	}
	return 0;
}

// Первый уровень - блоки

// По размеру нужно найти очередь, из в которой хранятся соответствующие дескрипторы.
// Очереди у нас начинаются с размера 4
// 4 8 16 32 64 128 256 512 1024 2048 4096
// 0 1  2  3  4   5   6   7    8    9   10	<- индекс очереди

unsigned int GetSizeIndex(size_t size)
{
	for (unsigned int i = 0; i < 10; i++) {
		if (size <= 1U << (i + 2)) {
			return i;
		}
	}
	
	return 10;
}

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

void *StubAllocatorAlloc(size_t size, AllocPage **queues, 
			 AllocPage *(*newPage)(size_t))
{
	unsigned int qi = GetSizeIndex(size);
	size_t asize = CalcBlockSize(size);
	
	if (asize >= PAGE_SIZE) {
		// Блоки такого размера не хранятся в очередях и мапы им 
		// отмечать не надо
		AllocPage *page = newPage(asize);
		return (void *)(page->base);
	}
	
	for (AllocPage *page = queues[qi]; page != NULL; page = page->next) {
		void *block = StubAllocatorPageGetBlock(page);
		if (block != NULL) {
			return block;
		}
	}
	
	AllocPage *page = newPage(asize);
	page->next = queues[qi];
	queues[qi] = page;

	StubAllocatorMarkBlock(page, 0);
	return (void *)(page->base);
}

// Интерфейс модуля

AllocPage *queues[9];

void StubAllocatorInit(void *block)
{
}

void *StubAlloc(size_t size)
{
	return StubAllocatorAlloc(size, queues, StubAllocatorNewPage);
}

void StubFree(void *ptr)
{
}
