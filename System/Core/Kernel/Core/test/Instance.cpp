//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../Resource.h"
#include "testResource.h"

#include "../InstanceProcess.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInstance)

// TODO: перенести в test/Instance.cpp
BOOST_AUTO_TEST_CASE(testGetResource)
{
	Resource *resource = new testResource(); // Ресурс удаляется удалением инстанции
	Instance instance(resource, 0);
	BOOST_REQUIRE_EQUAL(instance.resource(), resource);
}

BOOST_AUTO_TEST_SUITE_END()
