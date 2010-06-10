//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/ResourceRegion.h"
#include "../include/InstanceRegion.h"
#include "TestHelpers.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstanceRegion)

enum {	
	INSTANCE_OFFSET = 8000,
	INSTANCE_ACCESS = RESOURCE_ACCESS_READ,
	
	MOTHER_SIZE = 10000,
	MOTHER_OFFSET = 1000,
	MOTHER_WINDOW = 3000,
	MOTHER_ACCESS = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE
};

template <typename R = ResourceRegion>
struct fixtureInstanceRegion {
	InstanceRegion instance;
	fixtureInstanceRegion()
		: instance(new R(MOTHER_SIZE, MOTHER_ACCESS), 
			   INSTANCE_ACCESS, MOTHER_OFFSET, MOTHER_WINDOW, INSTANCE_OFFSET)
	{ }
};

BOOST_FIXTURE_TEST_CASE(testInBounds, fixtureInstanceRegion<>)
{
	BOOST_REQUIRE(!instance.inBounds(INSTANCE_OFFSET - 1));
	BOOST_REQUIRE(!instance.inBounds(INSTANCE_OFFSET + MOTHER_WINDOW));
	BOOST_REQUIRE(instance.inBounds(INSTANCE_OFFSET));
	BOOST_REQUIRE(instance.inBounds(INSTANCE_OFFSET + MOTHER_WINDOW - 1));
}

BOOST_FIXTURE_TEST_CASE(testPageFaultAccessDeny, fixtureInstanceRegion<>)
{
	uint32_t access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE;
	BOOST_REQUIRE(instance.PageFault(INSTANCE_OFFSET, &access) == 0);
	BOOST_REQUIRE_EQUAL(access, uint32_t(INSTANCE_ACCESS));
}

struct MockRegion : public ResourceRegion, private visit_mock {
	static PageInstance page;
	MockRegion(size_t size, uint32_t access) : ResourceRegion(size, access) {}
	virtual const PageInstance *PageFault(offset_t offset, uint32_t *) {
		visit();
		BOOST_REQUIRE_EQUAL(offset, offset_t(MOTHER_OFFSET));
		return &page;
	}
};

PageInstance MockRegion::page;

BOOST_FIXTURE_TEST_CASE(testPageFault, fixtureInstanceRegion<MockRegion>)
{
	uint32_t access = RESOURCE_ACCESS_READ;
	BOOST_REQUIRE_EQUAL(instance.PageFault(INSTANCE_OFFSET, &access), &MockRegion::page);
}

BOOST_AUTO_TEST_SUITE_END()
