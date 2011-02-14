//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>
#include <Stub.h>

BOOST_AUTO_TEST_SUITE(suiteIO)

BOOST_AUTO_TEST_CASE(testReserve)
{
	//StubIOMapInit();
	BOOST_REQUIRE_EQUAL(StubIOReserve(100, 150), SUCCESS);
}

BOOST_AUTO_TEST_SUITE_END()
