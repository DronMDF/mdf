//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Instance.h"
#include "testResource.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstance)

struct testInstance : public Instance {
	testInstance(Resource *r) : Instance(r, 0) {}
};

BOOST_AUTO_TEST_CASE(testGetResource)
{
	Resource *resource = new testResource(); // Ресурс удаляется удалением инстанции
	testInstance instance(resource);
	BOOST_REQUIRE_EQUAL(instance.resource(), resource);
}

BOOST_AUTO_TEST_CASE(testNotActive)
{
	testInstance instance(0);
	BOOST_REQUIRE(!instance.active());
}

BOOST_AUTO_TEST_SUITE_END()
