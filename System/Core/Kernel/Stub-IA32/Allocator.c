//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include "StubLocal.h"

#include "Allocator.h"
#include "Memory.h"

// Третий уровень (каталоги)

void StubAllocatorInitDirectory(AllocDir *dir)
{
	StubMemoryClear(dir, sizeof(AllocDir));
	dir->avail = sizeof(dir->pages) / sizeof(dir->pages[0]) - 1;
}

AllocDir *StubAllocatorAllocDirectory(void *(*getDir)())
{
	AllocDir *dir = getDir();
	StubAllocatorInitDirectory(dir);
	return dir;
}

// Второй уровень - страницы

AllocPage *StubAllocatorNewPage(size_t size, const StubAllocatorAllocFunctions *funcs)
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

void *StubAllocatorAlloc(size_t size, const StubAllocatorAllocFunctions *funcs)
{
	unsigned int qi = GetSizeIndex(size);
	size_t asize = CalcBlockSize(size);
	
	if (asize >= PAGE_SIZE) {
		AllocPage *page = funcs->newPage(asize, funcs);
		return StubAllocatorPageGetBlock(page);
	}

	// TODO: Анализировать стоит только первую страницу, а если в первой
	//	странице нет свободных мест, то реорганизовывать очередь. И в
	//	процессе реорганизации освободить совсем пустые, а самую
	//	свободную из несовсем пустых поставить в голову.
	for (AllocPage *page = funcs->queues[qi]; page != NULL; page = page->next) {
		void *block = StubAllocatorPageGetBlock(page);
		if (block != NULL) {
			return block;
		}
	}
	
	AllocPage *page = funcs->newPage(asize, funcs);
	void *ptr = StubAllocatorPageGetBlock(page);
	
	do {
		page->next = funcs->queues[qi];
	} while (!CAS(&(funcs->queues[qi]), page->next, page));

	return ptr;
}

// Интерфейс модуля

AllocPage *queues[10];

void StubAllocatorInit(void *block)
{
	// Инициализацию можно безусловно зарядить во втором каталоге (4М)
	StubAllocatorInitDirectory(block);
	// Сразу инициализируем страницу, которая используется для индексов
	void *pageptr = (char *)block + PAGE_SIZE;
//	AllocPage *page = StubAllocatorInitBlock(pageptr);

	// TODO: хардкодед размер очереди
//	funcs->queues[2] = page;
}

void *StubAlloc(size_t size)
{
	// Если блок большой - он всегда выделяется через аллокацию
	
	// Двигаемся по очереди и выискиваем там страницу со свободными блоками.
	// Сразу возвращаем, если нашли.

	// Выделяем новую страницу в очередь, если есть место в каталогах
	// Из нее возвращаем первый блок
	
	// Если нет места в каталогах - инициализируем новый каталог
	// добавлям в него страницу и возвращаем из нее первый блок

	// Не очень то линейный алгоритм, помоему здесь много фоллбеков.
	// Поиск блока по странице можно снести сюда.
}

void StubFree(void *ptr)
{
	// По адресу блока определяем адрес каталога,
	AllocDir *dir = (AllocDir *)((uint32_t)ptr & 0xff000000U);
	// по смещению страницы блока в каталоге определяем страницу
	unsigned int pageIdx = ((uint32_t)ptr & 0x00fff000U) >> 12;
	AllocPage *page = dir->pages[pageIdx];
	
	// по смещению в странице определяем бит в мапе и зануляем его
	offset_t blockOffset = (uint32_t)ptr & 0x00000fffU;
	STUB_ASSERT(blockOffset % page->block_size != 0, "Invalid block ptr");

	unsigned int blockIdx = blockOffset / page->block_size;
	uint32_t *map = page->map;	// На будующее можно разрулить доступ

	// освобождение идет от каталога, вообще не затрагивая очереди
	uint32_t mapValue, newValue;
	do {
		mapValue = map[blockIdx / 32];
		newValue = mapValue & ~(1 << (blockIdx % 32));
	} while (!CAS(&(map[blockIdx / 32]), mapValue, newValue));

	// TODO: страницу можно куда нибудь подвинуть в очереди или вообще освободить
}
