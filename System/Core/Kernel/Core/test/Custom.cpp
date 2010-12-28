//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../include/Custom.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCustom)

BOOST_AUTO_TEST_CASE(testCreateCustom)
{
	scoped_ptr<Resource> res(Custom::Create());
	// Если ресурс вдруг окажется не Custom - указатели разъедутся (доходчиво ли?)
	BOOST_REQUIRE_EQUAL(res->asCustom(), res->asResource());
}

BOOST_AUTO_TEST_SUITE_END()
