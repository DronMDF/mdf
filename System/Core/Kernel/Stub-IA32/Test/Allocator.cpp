//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>

extern "C" {
#include "../Allocator.h"

AllocDir *StubAllocatorDirectoryAlloc(void *(*newDir)());

// Уровень страниц
void *StubAllocatorPageGetBlock(AllocPage *page);

// Уровень очередей
size_t CalcBlockSize(size_t size);
unsigned int GetSizeIndex(size_t size);

void *StubAllocatorAlloc(size_t size, const StubAllocatorAllocFunctions *funcs);

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

// Уровень 2 - страницы

template<int size>
struct fixturePage {
	enum { 
		MAPSIZE = PAGE_SIZE / size / 32,
		BPM = sizeof(uint32_t) * 8,
		BASE = 0x12345000
	};
	uint32_t map[MAPSIZE > 0 ? MAPSIZE : 1];
	AllocPage page;
	
	fixturePage() {
		page.base = BASE;
		page.next = 0;
		page.map = map;
		page.block_size = size;
		
		::memset(map, 0xff, sizeof(map));
	}
};

BOOST_FIXTURE_TEST_CASE(testGetBlockFromFullPage, fixturePage<16>)
{
	BOOST_REQUIRE(StubAllocatorPageGetBlock(&page) == 0);
}

BOOST_FIXTURE_TEST_CASE(testGetBlockFromBeginPage, fixturePage<32>)
{
	map[0] &= ~1U;
	BOOST_REQUIRE_EQUAL(StubAllocatorPageGetBlock(&page), 
			    reinterpret_cast<void *>(BASE));
}

BOOST_FIXTURE_TEST_CASE(testGetBlockFromEndPage4, fixturePage<4>)
{
	const unsigned int index = (PAGE_SIZE - page.block_size) / page.block_size;
	map[index / BPM] &= ~(1U << (index % BPM));
	BOOST_REQUIRE_EQUAL(StubAllocatorPageGetBlock(&page), 
			    reinterpret_cast<void *>(BASE + PAGE_SIZE - page.block_size));
}

BOOST_FIXTURE_TEST_CASE(testGetBlockFromEndPage, fixturePage<1024>)
{
	map[0] &= ~(1U << 3);	// 4-й бит равен нулю - последняя страница из четырех доступна
	BOOST_REQUIRE_EQUAL(StubAllocatorPageGetBlock(&page), 
			    reinterpret_cast<void *>(BASE + PAGE_SIZE - page.block_size));
}

// Уровень 1, очереди
// Если в очередях есть доступные страницы - то StubAllocatorAlloc не должен 
// обращаться к newPage вообще...
template <int size>
struct fixtureQueue {
	fixturePage<size> page1;
	fixturePage<size> page2;
	AllocPage *queues[10];
	StubAllocatorAllocFunctions funcs;
	
	fixtureQueue() {
		::memset(&queues, 0, sizeof(queues));
		::memset(&funcs, 0, sizeof(funcs));
		funcs.queues = reinterpret_cast<AllocPage **>(&queues);
	}
};

BOOST_FIXTURE_TEST_CASE(testAlloc4, fixtureQueue<4>)
{
	queues[0] = &page1.page;
	::memset(&page1.map, 0, sizeof(page1.map));

	void *block = StubAllocatorAlloc(page1.page.block_size, &funcs);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page1.page.base));
	// Первый блок должен быть стать занятым
	BOOST_REQUIRE_EQUAL(page1.map[0], 1);

	void *block2 = StubAllocatorAlloc(page1.page.block_size, &funcs);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(page1.page.base + page1.page.block_size));
	// Второй блок тоже должен быть стать занятым
	BOOST_REQUIRE_EQUAL(page1.map[0], 3);
}

BOOST_FIXTURE_TEST_CASE(testAlloc8, fixtureQueue<8>)
{
	queues[1] = &page1.page;
	::memset(&page1.map, 0, sizeof(page1.map));
	
	void *block = StubAllocatorAlloc(page1.page.block_size, &funcs);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page1.page.base));
	// Первый блок должен быть стать занятым
	BOOST_REQUIRE_EQUAL(page1.map[0], 1);

	void *block2 = StubAllocatorAlloc(page1.page.block_size, &funcs);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(page1.page.base + page1.page.block_size));
	// Второй блок тоже должен быть стать занятым
	BOOST_REQUIRE_EQUAL(page1.map[0], 3);
}

BOOST_FIXTURE_TEST_CASE(testAllocWalkByQueue, fixtureQueue<16>)
{
	page1.page.next = &page2.page;
	queues[2] = &page1.page;
	::memset(&page2.map, 0, sizeof(page2.map));
	
	void *block = StubAllocatorAlloc(page2.page.block_size, &funcs);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page2.page.base));
	// Первый блок второй страницы должен быть стать занятым
	BOOST_REQUIRE_EQUAL(page2.map[0], 1);
}

AllocPage testPage;
uint32_t testMap[PAGE_SIZE / 4 / 32];

AllocPage *newPage(size_t size, const StubAllocatorAllocFunctions *) {
	memset(testMap, 0, sizeof(testMap));
	
	testPage.base = 666U * PAGE_SIZE + size;
	testPage.next = 0;
	testPage.map = testMap;
	testPage.block_size = size;
	
	return &testPage;
}

BOOST_AUTO_TEST_CASE(testAlloc4096)
{
	StubAllocatorAllocFunctions afuncs = {
		0,
		newPage
	};

	// Блоки такого размера не выискиваются в очередях, а непосредственно 
	// выделяются из пула страниц
	void *block = StubAllocatorAlloc(PAGE_SIZE, &afuncs);
	BOOST_REQUIRE_EQUAL(testPage.block_size, PAGE_SIZE);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(667 * PAGE_SIZE));

	// Две страницы!
	void *block2 = StubAllocatorAlloc(PAGE_SIZE + 1, &afuncs);
	BOOST_REQUIRE_EQUAL(testPage.block_size, PAGE_SIZE * 2);
	BOOST_REQUIRE_EQUAL(testPage.base, 668 * PAGE_SIZE);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(668 * PAGE_SIZE));
}

BOOST_AUTO_TEST_CASE(testNewPageIntoQueue)
{
	const size_t block_size = 32;
	AllocPage *pqueues[10] = { 0 };

	StubAllocatorAllocFunctions afuncs = {
		pqueues,
		newPage
	};

	void *block = StubAllocatorAlloc(block_size, &afuncs);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(666 * PAGE_SIZE + block_size));
	BOOST_REQUIRE_EQUAL(pqueues[3], &testPage);
	BOOST_REQUIRE_EQUAL(testPage.map[0], 1);
}

BOOST_AUTO_TEST_CASE(testAppendPageIntoQueue)
{
	const size_t block_size = 64;
	uint32_t map[PAGE_SIZE / block_size / 32] = { 0 };
	memset(&map, 0xff, sizeof(map));	// Блок полностью занят
	AllocPage page = { 0x665, 0, map, block_size };
	
	AllocPage *pqueues[10] = { 0, 0, 0, 0, &page, 0 };

	StubAllocatorAllocFunctions afuncs = {
		pqueues,
		newPage
	};

	void *block = StubAllocatorAlloc(block_size, &afuncs);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(666 * PAGE_SIZE + block_size));
	BOOST_REQUIRE_EQUAL(pqueues[4], &testPage);
	BOOST_REQUIRE_EQUAL(testPage.map[0], 1);
	BOOST_REQUIRE_EQUAL(testPage.next, &page);
}

BOOST_AUTO_TEST_SUITE_END()
