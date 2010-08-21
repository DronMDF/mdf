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

void *StubAllocatorAlloc(size_t size, AllocPage **queues, AllocPage *(*newPage)(int));

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
BOOST_AUTO_TEST_CASE(testFirstAlloc)
{
	const size_t block_size = 4;
	
	uint32_t map[4096 / block_size / 32] = { 0 };
	AllocPage page4 = { 666, 0, map, block_size };
	AllocPage *pqueues[9] = { &page4, 0 };
	void *block = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page4.base));
	// Первый блок должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 1);

	void *block2 = StubAllocatorAlloc(block_size, pqueues, 0);
	BOOST_REQUIRE_EQUAL(block2, reinterpret_cast<void *>(page4.base + block_size));
	// Второй блок тоже должен быть стать занятым
	BOOST_REQUIRE_EQUAL(map[0], 3);
}

BOOST_AUTO_TEST_SUITE_END()
