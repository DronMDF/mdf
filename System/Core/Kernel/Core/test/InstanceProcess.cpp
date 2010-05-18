//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/InstanceProcess.h"
#include "../include/ResourceProcess.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstanceProcess)

BOOST_AUTO_TEST_CASE(testActive)
{
	InstanceProcess instance(0, 0, 0);
	BOOST_REQUIRE(instance.active());
}

BOOST_AUTO_TEST_CASE(testPageFaultNoResource)
{
	InstanceProcess instance(0, 0, 0);
	uint32_t access = RESOURCE_ACCESS_READ;
	BOOST_REQUIRE(instance.PageFault(1000, &access) == 0);
}

BOOST_AUTO_TEST_CASE(testPageFaultResourceIsNotRegion)
{
	Resource *notregion = new ResourceProcess(0);
	InstanceProcess instance(notregion, 0, 0);
	uint32_t access = RESOURCE_ACCESS_READ;
	BOOST_REQUIRE(instance.PageFault(1000, &access) == 0);
}


BOOST_AUTO_TEST_SUITE_END()
