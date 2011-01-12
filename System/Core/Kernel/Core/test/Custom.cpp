//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "Kernel.h"
#include "../include/Custom.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCustom)

BOOST_AUTO_TEST_CASE(testCreateCustom)
{
	scoped_ptr<Resource> res(Custom::Create());
	BOOST_REQUIRE(res->asCustom() != 0);
}

BOOST_AUTO_TEST_CASE(testBindPortToCustomInvalidParam)
{
	scoped_ptr<Resource> res(Custom::Create());
	BOOST_REQUIRE_EQUAL(res->Modify(RESOURCE_MODIFY_CUSTOM_IOBIND, 0, 0), ERROR_INVALIDPARAM);
}

BOOST_AUTO_TEST_SUITE_END()
