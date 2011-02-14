//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>
#include <Stub.h>

extern "C" {
#include "../IO.h"
}

BOOST_AUTO_TEST_SUITE(suiteIO)

BOOST_AUTO_TEST_CASE(testReserve)
{
	StubIOInit();
	BOOST_REQUIRE_EQUAL(StubIOReserve(100, 150), SUCCESS);
	BOOST_REQUIRE_EQUAL(StubIOReserve(90, 100), ERROR_BUSY);
}

BOOST_AUTO_TEST_SUITE_END()
