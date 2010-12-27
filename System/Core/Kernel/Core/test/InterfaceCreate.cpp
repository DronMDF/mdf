//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/Core.h"

BOOST_AUTO_TEST_SUITE(suiteInterfaceCreate)

BOOST_AUTO_TEST_CASE(testCreateCustom)
{
	id_t id = INVALID_ID;
	BOOST_REQUIRE_EQUAL(CoreCreate(0, RESOURCE_TYPE_CUSTOM, 0, 0, &id), SUCCESS);
}

BOOST_AUTO_TEST_SUITE_END()
