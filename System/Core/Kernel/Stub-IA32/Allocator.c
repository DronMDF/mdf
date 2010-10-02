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
	// сперва нам надо выделить map и страницу. выделять их надо из того же 
	// хипа, нужно вызвать функцию аллокации со специальным флагов - чтобы те
	// могли использовать горячий резерв...
	
	// потом обратиться к каталогу, чтобы тот разместил страницу, выделив 
	// ей по ходу базу, поскольку это атомарная операция
	
	return 0;
}

void *StubAllocatorPageGetBlock(AllocPage *page)
{
	const int bpp = max((int)(PAGE_SIZE / page->block_size), 1);
	const int bpm = sizeof(page->map[0]) * 8;
	
	for (int bi = 0; bi < bpp; bi += bpm) {
		const int mi = bi / bpm;
		const uint32_t mv = page->map[mi];
		if (mv == 0xffffffff) continue;
			
		for (int biti = 0; biti < min(bpm, bpp - bi); biti++) {
			const uint32_t bitv = 1U << biti;
			if ((mv & bitv) != 0) continue;
			if (!CAS(&(page->map[mi]), mv, mv | bitv)) {
				// TODO: Нарушение совместного доступа. Можно 
				// было бы попытаться перечитать mv, и пройтись 
				// по нему снова, но пока можно просто пойти дальше.
				break;
			}

			return (void *)(page->base + page->block_size * (size_t)(bi + biti));
		}
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

void *StubAllocatorAlloc(size_t size, AllocPage **queues, const StubAllocatorAllocFunctions *funcs)
{
	unsigned int qi = GetSizeIndex(size);
	size_t asize = CalcBlockSize(size);
	
	if (asize >= PAGE_SIZE) {
		AllocPage *page = funcs->newPage(asize);
		return StubAllocatorPageGetBlock(page);
	}

	// TODO: Анализировать стоит только первую страницу, а если в первой
	//	странице нет свободных мест, то реорганизовывать очередь. И в
	//	процессе реорганизации освободить совсем пустые, а самую
	//	свободную из несовсем пустых поставить в голову.
	for (AllocPage *page = queues[qi]; page != NULL; page = page->next) {
		void *block = StubAllocatorPageGetBlock(page);
		if (block != NULL) {
			return block;
		}
	}
	
	AllocPage *page = funcs->newPage(asize);
	void *ptr = StubAllocatorPageGetBlock(page);
	
	do {
		page->next = queues[qi];
	} while (!CAS(&(queues[qi]), page->next, page));

	return ptr;
}

// Интерфейс модуля

AllocPage *queues[10];

void StubAllocatorInit(void *block)
{
	// Инициализацию можно безусловно зарядить во втором каталоге (4М)
}

void *StubAlloc(size_t size)
{
	const static StubAllocatorAllocFunctions funcs = {
		StubAllocatorNewPage
	};

	return StubAllocatorAlloc(size, queues, &funcs);
}

void StubFree(void *ptr)
{
	// По адресу блока определяем адрес каталога, 
	// по смещению страницы блока в каталоге определяем страницу
	// по смещению в странице определяем бит в мапе и зануляем его
	
	// то есть освобождение идет от каталога, вообще не затрагивая очереди
}
