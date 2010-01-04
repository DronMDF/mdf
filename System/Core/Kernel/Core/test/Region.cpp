//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Region.h"

#include "TestHelpers.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteRegion)

struct RegionFixture1 {
	enum { reg_offset = PAGE_SIZE / 2, reg_size = PAGE_SIZE };
	char data[reg_size];
	RegionFixture1() { fill_random(data, reg_size); }
};

BOOST_FIXTURE_TEST_CASE(testCopyIn, RegionFixture1)
{
	ResourceRegion region(reg_offset, reg_size, 0);
	BOOST_REQUIRE(region.copyIn(reg_offset, data, reg_size));

	uint32_t access = 0;
	const PageInstance *pinst = region.PageFault(reg_offset, &access);
	PageInfo *page = StubGetPageByInstance(pinst);
	BOOST_REQUIRE(page != 0);
	const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(m + reg_offset, m + PAGE_SIZE, data, data + PAGE_SIZE - reg_offset);
	
	const PageInstance *pinst2 = region.PageFault(PAGE_SIZE, &access);
	PageInfo *page2 = StubGetPageByInstance(pinst2);
	BOOST_REQUIRE(page2 != 0);
	const char *m2 = reinterpret_cast<const char *>(StubPageTemporary(page2));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(m2, m2 + reg_offset, data + PAGE_SIZE - reg_offset, data + reg_size);
}

BOOST_FIXTURE_TEST_CASE(testCopyInUnderload, RegionFixture1)
{
	ResourceRegion region(reg_offset, reg_size, 0);
	BOOST_REQUIRE(!region.copyIn(reg_offset - 1, data, reg_size));
}

BOOST_FIXTURE_TEST_CASE(testCopyInOverload, RegionFixture1)
{
	ResourceRegion region(reg_offset, reg_size, 0);
	BOOST_REQUIRE(!region.copyIn(reg_offset + 1, data, reg_size));
}

BOOST_AUTO_TEST_SUITE_END()
