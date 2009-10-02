//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//


#include <limits.h>
#include <string.h>
#include <ctype.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(ctype)

BOOST_AUTO_TEST_CASE(isdigit)
{
	const char *digit = "0123456789";
	for (int c = 0; c < 256; c++) {
		BOOST_CHECK(::isdigit(c) == (::strchr(digit, c) == NULL) ? 0 : 1);
	}
}

BOOST_AUTO_TEST_CASE(isspace)
{
	const char *spaces = " \f\n\r\t\v";
	for (int c = 0; c < 256; c++) {
		BOOST_CHECK(::isspace(c) == (::strchr(spaces, c) == NULL) ? 0 : 1);
	}
}

BOOST_AUTO_TEST_SUITE_END()
