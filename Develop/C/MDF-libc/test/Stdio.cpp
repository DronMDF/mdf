//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <string.h>
#include <stdio.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(string)

BOOST_AUTO_TEST_CASE(snprintf)
{
	// Здесь тестируем ограничения.
	char buf[32];
	BOOST_CHECK(::snprintf(buf, 10, "xxx xxx xxx") == 9);
	BOOST_CHECK(::strcmp(buf, "xxx xxx x") == 0);

	BOOST_CHECK(::snprintf(buf, 10, "xxx %s xxx", "yyy yyy") == 9);
	BOOST_CHECK(::strcmp(buf, "xxx yyy y") == 0);

	BOOST_CHECK(::snprintf(buf, 10, "xxx %u xxx", 1234567) == 9);
	BOOST_CHECK(::strcmp(buf, "xxx 12345") == 0);
}

BOOST_AUTO_TEST_CASE(sprintf)
{
	char buf[32];
	BOOST_CHECK(::sprintf(buf, "xxx %s xxx", "xx") == 10);
	BOOST_CHECK(::strcmp(buf, "xxx xx xxx") == 0);

	BOOST_CHECK(::sprintf(buf, "xxx %u xxx", 10) == 10);
	BOOST_CHECK(::strcmp(buf, "xxx 10 xxx") == 0);

	BOOST_CHECK(::sprintf(buf, "xxx %c xxx", '?') == 9);
	BOOST_CHECK(::strcmp(buf, "xxx ? xxx") == 0);

	BOOST_CHECK(::sprintf(buf, "xxx %u xxx", 0) == 9);
	BOOST_CHECK(::strcmp(buf, "xxx 0 xxx") == 0);
}

BOOST_AUTO_TEST_CASE(sscanf)
{
	unsigned int u, u2;
	char buf[32];

	BOOST_CHECK(::sscanf("test", "test") == 0);

	BOOST_CHECK(::sscanf("  543 test", "%u", &u) == 1);
	BOOST_CHECK(u == 543);

	BOOST_CHECK(::sscanf("'test'", "'%[^']", buf) == 1);
	BOOST_CHECK(::strcmp(buf, "test") == 0);

	BOOST_CHECK(::sscanf("Register?prefix='Console://'&tpc=223",
		"Register?prefix='%[^']'&tpc=%u", buf, &u) == 2);
	BOOST_CHECK(::strcmp(buf, "Console://") == 0);
	BOOST_CHECK(u == 223);

	BOOST_CHECK(::sscanf("read?file='/etc/inittab'&bytes=106&offset=45",
		"read?file='%[^']'&bytes=%u&offset=%u", buf, &u, &u2) == 3);
	BOOST_CHECK(::strcmp(buf, "/etc/inittab") == 0);
	BOOST_CHECK(u == 106);
	BOOST_CHECK(u2 == 45);
}

BOOST_AUTO_TEST_CASE(vsnprintf)
{
	char buf[32];
	va_list args;

	BOOST_CHECK(::vsnprintf(buf, 32, "xxx", args) == 3);
	BOOST_CHECK(::strcmp(buf, "xxx") == 0);

	BOOST_CHECK(::vsnprintf(buf, 32, "xxx %% xxx", args) == 9);
	BOOST_CHECK(::strcmp(buf, "xxx % xxx") == 0);

	BOOST_CHECK(::vsnprintf(buf, 10, "xxx xxx xxx", args) == 9);
	BOOST_CHECK(::strcmp(buf, "xxx xxx x") == 0);
}

BOOST_AUTO_TEST_CASE(vsprintf)
{
	// Эта функция ничем не отличается от vsnprintf
}

BOOST_AUTO_TEST_SUITE_END()
