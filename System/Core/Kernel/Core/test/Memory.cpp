//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <algorithm>
#include <boost/test/unit_test.hpp>
#include "TestHelpers.h"

#include "Types.h"
#include "../Memory.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteMemory)

struct MemFixture1 {
	enum { mem_size = PAGE_SIZE * 3 - PAGE_SIZE / 2 }; // 2.5 страницы;
	char data[mem_size];
	MemFixture1() { fill_random(data, mem_size); }
};

BOOST_FIXTURE_TEST_CASE(testCopyIn, MemFixture1)
{
	Memory mem(mem_size, Memory::ALLOC);
	BOOST_REQUIRE(mem.copyIn(0, data, mem_size));

	for (int i = 0; i < 3; i++) {
		const PageInstance *pinst = mem.getPage(i * PAGE_SIZE);
		PageInfo *page = StubGetPageByInstance(pinst);
		BOOST_REQUIRE(page != 0);
	
		const char *dm = data + i * PAGE_SIZE;
		const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
		const size_t page_size = min(PAGE_SIZE, mem_size - i * PAGE_SIZE);
		BOOST_REQUIRE_EQUAL_COLLECTIONS(dm, dm + page_size, m, m + page_size);
	}
}

BOOST_FIXTURE_TEST_CASE(testCopyOverFull1, MemFixture1)
{
	Memory mem(mem_size, Memory::ALLOC);
	BOOST_REQUIRE(!mem.copyIn(0, data, mem_size + 1));
}

BOOST_FIXTURE_TEST_CASE(testCopyOverFull2, MemFixture1)
{
	Memory mem(mem_size, Memory::ALLOC);
	BOOST_REQUIRE(!mem.copyIn(1, data, mem_size));
}

BOOST_FIXTURE_TEST_CASE(testCopyAccess1, MemFixture1)
{
	Memory mem(mem_size);	// Не создает страницы...
	BOOST_REQUIRE(!mem.copyIn(0, data, mem_size));
}

BOOST_AUTO_TEST_SUITE_END()