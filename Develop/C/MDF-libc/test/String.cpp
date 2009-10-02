//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <string.h>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(string)

BOOST_AUTO_TEST_CASE(memchr)
{
	const char *buf = "12345";
	BOOST_CHECK(::memchr(buf, '3', 5) == buf + 2);
	BOOST_CHECK(::memchr(buf, 0, 6) == buf + 5);
	BOOST_CHECK(::memchr(buf, '6', 6) == NULL);
}

BOOST_AUTO_TEST_CASE(memcmp)
{
	BOOST_CHECK(::memcmp("12345", "12345", 6) == 0);
	BOOST_CHECK(::memcmp("12345", "62543", 0) == 0);
	BOOST_CHECK(::memcmp("12345", "12343", 6) > 0);
	BOOST_CHECK(::memcmp("12345", "12543", 6) < 0);
}

BOOST_AUTO_TEST_CASE(memset)
{
	char buf[10];
	BOOST_CHECK(::memset(buf, '1', 10) == buf);
	BOOST_CHECK(::memcmp(buf, "1111111111", 10) == 0);

	::memset(buf, '2', 8);
	BOOST_CHECK(::memcmp(buf, "2222222211", 10) == 0);

	::memset(buf, '3', 0);
	BOOST_CHECK(::memcmp(buf, "2222222211", 10) == 0);
}

BOOST_AUTO_TEST_CASE(memcpy)
{
	char dst[9] = {"xxxxxxxx"};
	BOOST_CHECK(::memcpy(dst, "123456", 5) == dst);
	BOOST_CHECK(::memcmp(dst, "12345x", 6) == 0);

	::memset(dst, 'x', 8);
	::memcpy(dst, "123456", 0);
	BOOST_CHECK(::memcmp(dst, "xxxxxxxx", 8) == 0);
}

BOOST_AUTO_TEST_CASE(memmove)
{
	char dst[11] = {"xXXXXXXXXx"};
	BOOST_CHECK(::memmove(dst + 2, "abcdef", 6) == dst + 2);
	BOOST_CHECK(::memcmp(dst, "xXabcdefXx", 10) == 0);

	::memmove(dst + 3, dst + 2, 5);
	BOOST_CHECK(::memcmp(dst, "xXaabcdeXx", 10) == 0);

	::memmove(dst + 2, dst + 4, 4);
	BOOST_CHECK(::memcmp(dst, "xXbcdedeXx", 10) == 0);
}

BOOST_AUTO_TEST_CASE(strcmp)
{
	BOOST_CHECK(::strcmp("12345", "12345") == 0);
	BOOST_CHECK(::strcmp("", "") == 0);

	BOOST_CHECK(::strcmp("12345", "12343") > 0);
	BOOST_CHECK(::strcmp("12345", "12543") < 0);

	BOOST_CHECK(::strcmp("12345", "123") > 0);
	BOOST_CHECK(::strcmp("123", "12345") < 0);
}

BOOST_AUTO_TEST_CASE(strncpy)
{
	char dst[30];
	BOOST_CHECK(::strncpy(dst, "abcdef", 30) == dst);
	BOOST_CHECK(::strcmp(dst, "abcdef") == 0);

	::strncpy(dst, "ghijkl", 3);

	BOOST_CHECK(::memcmp(dst, "ghi\0ef", 7) == 0);

	::strncpy(dst, "mnopqr", 0);
	BOOST_CHECK(::memcmp(dst, "\0hi\0ef", 7) == 0);
}

BOOST_AUTO_TEST_CASE(strcat)
{
	char dst[32] = {"aaaaa"};
	BOOST_CHECK(::strcat(dst, "bbbbb") == dst);
	BOOST_CHECK(::strcmp(dst, "aaaaabbbbb") == 0);

	::strcat(dst, "ccc");
	BOOST_CHECK(::strcmp(dst, "aaaaabbbbbccc") == 0);
}

BOOST_AUTO_TEST_CASE(strchr)
{
	const char *buf = "12345";
	BOOST_CHECK(::strchr(buf, '3') == buf + 2);
	BOOST_CHECK(::strchr(buf, '6') == NULL);
	BOOST_CHECK(::strchr(buf, '\0') == NULL);
}

BOOST_AUTO_TEST_CASE(strcoll)
{
	// тест не требуется, полностью идентично strcmp.
}

BOOST_AUTO_TEST_CASE(strcpy)
{
	char dst[8];
	BOOST_CHECK(::strcpy(dst, "abcdef") == dst);
	BOOST_CHECK(::memcmp(dst, "abcdef", 7) == 0);

	::strcpy(dst, "ghi");
	BOOST_CHECK(::memcmp(dst, "ghi\0ef", 7) == 0);

	::strcpy(dst, "");
	BOOST_CHECK(::memcmp(dst, "\0hi\0ef", 7) == 0);
}

BOOST_AUTO_TEST_CASE(strcspn)
{
	BOOST_CHECK(::strcspn("abcdef", "1234") == 6);
	BOOST_CHECK(::strcspn("abcdef", "12c34") == 2);
	BOOST_CHECK(::strcspn("abcdef", "12c34a") == 0);
}

BOOST_AUTO_TEST_CASE(stderror)
{
	// Не нуждается в тестированиии, может быть вообще перенесу его в отдельный модуль.
}

BOOST_AUTO_TEST_CASE(strlen)
{
	BOOST_CHECK(::strlen("12345") == 5);
	BOOST_CHECK(::strlen("") == 0);
}

BOOST_AUTO_TEST_CASE(strncat)
{
	char dst[32] = {"aaaaa"};

	BOOST_CHECK(::strncat(dst, "bbb", 5) == dst);
	BOOST_CHECK(::strcmp(dst, "aaaaabbb") == 0);

	::strncat(dst, "bb", 2);
	BOOST_CHECK(::strcmp(dst, "aaaaabbbbb") == 0);

	::strncat(dst, "ccccc", 3);
	BOOST_CHECK(::strcmp(dst, "aaaaabbbbbccc") == 0);

	::strncat(dst, "ddddd", 0);
	BOOST_CHECK(::strcmp(dst, "aaaaabbbbbccc") == 0);
}

BOOST_AUTO_TEST_CASE(strncmp)
{
	BOOST_CHECK(::strncmp("12345", "12345", 5) == 0);
	BOOST_CHECK(::strncmp("12345", "12343", 5) > 0);
	BOOST_CHECK(::strncmp("12345", "12543", 5) < 0);
	BOOST_CHECK(::strncmp("12345", "12543", 2) == 0);
	BOOST_CHECK(::strncmp("12345", "123", 5) > 0);
	BOOST_CHECK(::strncmp("123", "12345", 5) < 0);
	BOOST_CHECK(::strncmp("abc\0e", "abc\0\0", 5) == 0);
	BOOST_CHECK(::strncmp("abcdef", "abcdef", 0) == 0);
}

BOOST_AUTO_TEST_CASE(strpbrk)
{
	const char *buf = "abcdef";
	BOOST_CHECK(::strpbrk(buf, "1234") == NULL);
	BOOST_CHECK(::strpbrk(buf, "12c34") == buf + 2);
	BOOST_CHECK(::strpbrk(buf, "12c34a") == buf);
}

BOOST_AUTO_TEST_CASE(strrchr)
{
	const char *buf = "abcabc";
	BOOST_CHECK(::strrchr(buf, '\0') == buf + 6);
	BOOST_CHECK(::strrchr(buf, 'x') == NULL);
	BOOST_CHECK(::strrchr(buf, 'a') == buf + 3);
	BOOST_CHECK(::strrchr(buf, 'c') == buf + 5);
}

BOOST_AUTO_TEST_CASE(strspn)
{
	BOOST_CHECK(::strspn("abcdef", "abc") == 3);
	BOOST_CHECK(::strspn("abcdef", "abcdef") == 6);
	BOOST_CHECK(::strspn("abcdef", "123a") == 1);
	BOOST_CHECK(::strspn("abcdef", "1234") == 0);
}

BOOST_AUTO_TEST_CASE(strstr)
{
	const char *buf = "abcdef";
	BOOST_CHECK(::strstr(buf, "") == buf);
	BOOST_CHECK(::strstr(buf, "cd") == buf + 2);
	BOOST_CHECK(::strstr(buf, "bcde") == buf + 1);
	BOOST_CHECK(::strstr(buf, "abc") == buf);
	BOOST_CHECK(::strstr(buf, "defg") == NULL);
}

BOOST_AUTO_TEST_CASE(strtok)
{
	char str[32] = {"?a???b,,,#c"};
	char *t;

	t = ::strtok(str, "?");
	BOOST_CHECK(::strcmp(t, "a") == 0);

	t = ::strtok(NULL, ",");
	BOOST_CHECK(::strcmp(t, "??b") == 0);

	t = ::strtok(NULL, "#,");
	BOOST_CHECK(::strcmp(t, "c") == 0);

	t = ::strtok(NULL, "?");
	BOOST_CHECK(t == NULL);
}

BOOST_AUTO_TEST_CASE(strxfrm)
{
	// Не тестируем.
}

BOOST_AUTO_TEST_SUITE_END()
