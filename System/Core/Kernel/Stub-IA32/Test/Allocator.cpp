//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>

extern "C" {
#include "../Allocator.h"

// И это все не то, что нужно...
size_t CalcBlockSize(size_t size);
unsigned int GetSizeIndex(size_t size);

AllocDir *StubAllocatorDirectoryAlloc(void *(*newDir)());

void *StubAllocatorAlloc(size_t size, AllocPage **queues, 
			 AllocPage *(*newPage)(size_t));

}

BOOST_AUTO_TEST_SUITE(suiteAllocator)

BOOST_AUTO_TEST_CASE(testBlockSize)
{
	BOOST_REQUIRE_EQUAL(CalcBlockSize(1), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(3), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(4), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(5), 8);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(33), 64);
}

BOOST_AUTO_TEST_CASE(testSizeIndex)
{
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1), 0);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(4), 0);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(8), 1);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(16), 2);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(32), 3);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(64), 4);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(128), 5);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(200), 6);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(512), 7);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1024), 8);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1025), 9);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(4567), 10);
}

// Уровень 3, аллокатор каталогов

static AllocDir testDirectory;

void *testGetDir()
{
	return &testDirectory;
}

BOOST_AUTO_TEST_CASE(testAllocDirectory)
{
	AllocDir *dir = StubAllocatorDirectoryAlloc(testGetDir);
	BOOST_REQUIRE_EQUAL(dir, &testDirectory);

	const size_t pages_per_dir = sizeof(dir->pages) / sizeof(dir->pages[0]);
	BOOST_REQUIRE_EQUAL(dir->avail, pages_per_dir);
	for (size_t i = 0; i < pages_per_dir; i++) {
		BOOST_REQUIRE(dir->pages[i] == 0);
	}
}

// Если в очередях есть доступные страницы - то StubAllocatorAlloc не должен 
// обращаться к newPage вообще...
BOOST_AUTO_TEST_CASE(testAlloc4)
{
	const size_t block_size = 4;
	
	uint32_t map[4096 / block_size / 32] = { 0 };
	AllocPage page4 = { 666, 0, map, block_size };
	AllocPage *pqueues[10] = { &page4, 0 };
	
	void *block = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page4.base));
	// Первый блок должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 1);

	void *block2 = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(page4.base + block_size));
	// Второй блок тоже должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 3);
}

BOOST_AUTO_TEST_CASE(testAlloc8)
{
	const size_t block_size = 8;
	
	uint32_t map[PAGE_SIZE / block_size / 32] = { 0 };
	AllocPage page = { 666, 0, map, block_size };
	AllocPage *pqueues[10] = { 0, &page, 0 };
	
	void *block = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page.base));
	// Первый блок должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 1);

	void *block2 = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(page.base + block_size));
	// Второй блок тоже должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 3);
}

BOOST_AUTO_TEST_CASE(testAllocWalkByQueue)
{
	const size_t block_size = 16;

	const unsigned int map_size = PAGE_SIZE / block_size / 32;
	uint32_t map2[map_size] = { 0 };
	AllocPage page2 = { 0x666, 0, map2, block_size };
	
	uint32_t map1[map_size] = { 0 };
	memset(&map1, 0xff, map_size * sizeof(uint32_t));	// Блок полностью занят
	AllocPage page1 = { 0x999, &page2, map1, block_size };
	
	AllocPage *pqueues[10] = { 0, 0, &page1, 0 };

	void *block = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page2.base));
	// Первый блок второй страницы должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map2[0], 1);
}

AllocPage testPage;
uint32_t testMap[PAGE_SIZE / 4 / 32];

AllocPage *newPage(size_t size) {
	memset(testMap, 0, sizeof(testMap));
	
	testPage.base = 666U * PAGE_SIZE + size;
	testPage.next = 0;
	testPage.map = testMap;
	testPage.block_size = size;
	
	return &testPage;
}

BOOST_AUTO_TEST_CASE(testAlloc4096)
{
	// Блоки такого размера не выискиваются в очередях, а непосредственно 
	// выделяются из пула страниц
	void *block = StubAllocatorAlloc(PAGE_SIZE, 0, newPage);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(667 * PAGE_SIZE));

	// Две страницы!
	void *block2 = StubAllocatorAlloc(PAGE_SIZE + 1, 0, newPage);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(668 * PAGE_SIZE));
}

BOOST_AUTO_TEST_CASE(testNewPageIntoQueue)
{
	const size_t block_size = 32;
	AllocPage *pqueues[10] = { 0 };

	void *block = StubAllocatorAlloc(block_size, pqueues, newPage);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(666 * PAGE_SIZE + block_size));
	BOOST_REQUIRE_EQUAL(pqueues[3], &testPage);
	BOOST_REQUIRE_EQUAL(testPage.map[0], 1);
}

BOOST_AUTO_TEST_SUITE_END()
