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

void *StubAllocatorAlloc(size_t size, AllocPage *page_queue, void *(*newPage)());

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

BOOST_AUTO_TEST_CASE(testFirstAlloc)
{
	uint32_t map[4096 / 4 / 32] = { 0 };
	AllocPage page4 = { 666, 0, map, 4 };
	void *block = StubAllocatorAlloc(4, &page4, 0);
	BOOST_REQUIRE_EQUAL(block, reinterpret_cast<void *>(page4.base));
}

BOOST_AUTO_TEST_SUITE_END()
