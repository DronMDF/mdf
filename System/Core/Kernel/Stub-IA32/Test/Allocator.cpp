//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>

namespace {
	extern "C" {
		#include "../Allocator.c"
	}
}

BOOST_AUTO_TEST_SUITE(suiteAllocator)

BOOST_AUTO_TEST_CASE(testBlockSize)
{
	BOOST_REQUIRE_EQUAL(CalcBlockSize(1), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(3), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(4), 4);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(5), 8);
	BOOST_REQUIRE_EQUAL(CalcBlockSize(33), 64);
}

BOOST_AUTO_TEST_CASE(testSizeIndex)
{
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1), 0);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(4), 0);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(8), 1);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(16), 2);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(32), 3);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(64), 4);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(128), 5);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(200), 6);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(512), 7);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1024), 8);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(1025), 9);
	BOOST_REQUIRE_EQUAL(GetSizeIndex(4567), 10);
}

BOOST_AUTO_TEST_SUITE_END()
