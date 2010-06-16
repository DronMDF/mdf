//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <stdexcept>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/ResourceRegion.h"

#include "TestHelpers.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteRegion)

struct testRegion : public ResourceRegion {
	testRegion(size_t size) : ResourceRegion(size, 0) {}
	using ResourceRegion::bindRegion;
};

struct fixtureRegionParam {
	size_t size() const { return 5432; }
};

BOOST_FIXTURE_TEST_CASE(testCreate, fixtureRegionParam)
{
	ResourceRegion region(size(), 0);
	BOOST_REQUIRE_EQUAL(region.size(), size());
	BOOST_REQUIRE_EQUAL(region.offset(), 0);
}

BOOST_FIXTURE_TEST_CASE(testRegionBinding, fixtureRegionParam)
{
	ResourceRegion parent(size(), 0);
	testRegion child(size() / 2);
	child.bindRegion(&parent, size() / 2, size() / 2, 0);
	BOOST_REQUIRE_EQUAL(child.offset(), size() / 2);
}

struct RegionFixture1 {
	enum { SKIP = PAGE_SIZE / 2, SIZE = PAGE_SIZE };
	char data[SIZE];
	RegionFixture1() { generate(data, data + SIZE, rand); }
};

BOOST_FIXTURE_TEST_CASE(testCopyIn, RegionFixture1)
{
	ResourceRegion region(SKIP + SIZE, 0);
	BOOST_REQUIRE(region.copyIn(SKIP, data, SIZE));

	uint32_t access = 0;
	const PageInstance *pinst = region.PageFault(SKIP, &access);
	PageInfo *page = StubGetPageByInstance(pinst);
	BOOST_REQUIRE(page != 0);
	const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(m + SKIP, m + PAGE_SIZE, data, data + PAGE_SIZE - SKIP);
	
	const PageInstance *pinst2 = region.PageFault(PAGE_SIZE, &access);
	PageInfo *page2 = StubGetPageByInstance(pinst2);
	BOOST_REQUIRE(page2 != 0);
	const char *m2 = reinterpret_cast<const char *>(StubPageTemporary(page2));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(m2, m2 + SKIP, data + PAGE_SIZE - SKIP, data + SIZE);
}

BOOST_FIXTURE_TEST_CASE(testCopyInOverload, RegionFixture1)
{
	ResourceRegion region(SKIP + SIZE, 0);
	BOOST_REQUIRE(!region.copyIn(SKIP + 1, data, SIZE));
}

// BOOST_FIXTURE_TEST_CASE(testBindPhysicalErrors, fixtureRegionParam)
// {
// 	struct testRegion : public ResourceRegion, private order_mock<3> {
// 		testRegion(size_t size) : ResourceRegion(size, 0) {}
// 		virtual Memory *getMemory() { 
// 			order_next(); 
// 			throw runtime_error("called"); 
// 		}
// 		using ResourceRegion::bindPhysical;
// 	} region(SIZE);
// 	
// 	BOOST_REQUIRE_EQUAL(region.bindPhysical(OFFSET, SIZE, PAGE_SIZE), ERROR_OVERSIZE);
// 	
// 	// Эти проверки должны пройти
// 	BOOST_REQUIRE_THROW(region.bindPhysical(OFFSET, SIZE, 0), runtime_error);
// 	BOOST_REQUIRE_THROW(region.bindPhysical(OFFSET, SIZE - PAGE_SIZE, PAGE_SIZE), runtime_error);
// }

// BOOST_FIXTURE_TEST_CASE(testBindPhysicalMemory, fixtureRegionParam)
// {
// 	struct testMemory : public Memory, private visit_mock {
// 		int m_offset, m_size;
// 		testMemory(int offset, int size)
// 			: Memory(offset + size), m_offset(offset), m_size(size)
// 		{}
// 		int bindPhysical(offset_t poffset, size_t psize, offset_t skip) {
// 			visit();
// 			BOOST_REQUIRE_EQUAL(poffset, m_offset);
// 			BOOST_REQUIRE_EQUAL(psize, m_size);
// 			BOOST_REQUIRE_EQUAL(skip, m_offset);
// 			return SUCCESS;
// 		}
// 	};
// 
// 	struct testRegion : public ResourceRegion {
// 		testRegion(int offset, int size)
// 			: ResourceRegion(size, 0)
// 		{
// 			m_memory = new testMemory(offset, size);
// 		}
// 		using ResourceRegion::bindPhysical;
// 	} region(OFFSET, SIZE);
// 
// 	BOOST_REQUIRE_EQUAL(region.bindPhysical(OFFSET, SIZE, 0), SUCCESS);
// 	BOOST_REQUIRE_EQUAL(region.offset(), offset_t(OFFSET));
// }

enum {
	MOTHER_SIZE = 3000,
	MOTHER_OFFSET = 1000,
	WINDOW_SIZE = 2000,
	REGION_SIZE = 10000,
	BIND_OFFSET = 5000
};

struct testFoltedRegion : public ResourceRegion, private visit_mock {
	static PageInstance page;
	testFoltedRegion() : ResourceRegion(MOTHER_SIZE, RESOURCE_ACCESS_READ) {}
	virtual const PageInstance *PageFault(offset_t offset, uint32_t *) {
		visit(); return (offset == MOTHER_OFFSET) ? &page : 0;
	}
};

PageInstance testFoltedRegion::page;

BOOST_AUTO_TEST_CASE(testBindRegionPageFault)
{
	// TODO:
	// Создать тестовый регион
	ResourceRegion region(REGION_SIZE, RESOURCE_ACCESS_READ);
	// Забиндить его к подставному региону с отладкой.
	ResourceRegion *mother = new testFoltedRegion();
	BOOST_REQUIRE_EQUAL(region.bindRegion(mother, MOTHER_OFFSET, WINDOW_SIZE, BIND_OFFSET), SUCCESS);
	
	// Вызвать пейджфолт и посмотреть как это будет.
	uint32_t access = 0;
	BOOST_REQUIRE_EQUAL(region.PageFault(BIND_OFFSET, &access), &testFoltedRegion::page);
}

BOOST_AUTO_TEST_SUITE_END()
