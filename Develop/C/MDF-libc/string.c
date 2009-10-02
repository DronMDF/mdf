//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "stddef.h"
#include "stdint.h"
#include "string.h"

void *memchr(const void *s, int c, size_t n)
{
	unsigned char *_s = (unsigned char *)s;

	for (size_t i = 0; i < n; i++) {
		if (_s[i] == (unsigned char)c)
			return &(_s[i]);
	}

	return NULL;
}

int memcmp(const void *s1, const void *s2, size_t n)
{
	const unsigned char *_s1 = (unsigned char *)s1;
	const unsigned char *_s2 = (unsigned char *)s2;

	for (size_t i = 0; i < n; i++) {
		if (_s1[i] != _s2[i])
			return _s1[i] - _s2[i];
	}

	return 0;
}

void *memcpy(void *s1, const void *s2, size_t n)
{
	unsigned char *_s1 = (unsigned char *)s1;
	unsigned char *_s2 = (unsigned char *)s2;

	for (size_t i = 0; i < n; i++) {
		_s1[i] = _s2[i];
	}

	return s1;
}

void *memmove(void *s1, const void *s2, size_t n)
{
	if (s1 < s2)
		return memcpy (s1, s2, n);

	if (s1 > s2) {
		unsigned char *_s1 = (unsigned char *)s1;
		unsigned char *_s2 = (unsigned char *)s2;

		for (int i = n - 1; i >= 0; i--) {
			_s1[i] = _s2[i];
		}
	}

	return s1;
}

void *memset(void *s, int c, size_t n)
{
	unsigned char *_s = (unsigned char *)s;

	for (size_t i = 0; i < n; i++) {
		_s[i] = (unsigned char)c;
	}

	return s;
}

size_t strlen(const char *s)
{
	for (int i = 0; ; i++) {
		if (s[i] == 0)
			return i;
	}

	return 0;	// Не должна сюда попадать (там же цикл бесконечный).
}

char *strncpy(char *s1, const char *s2, size_t n)
{
	size_t len = strlen (s2);
	if (n < len)
		len = n;

	memcpy (s1, s2, len);

	s1[len] = 0;
	return s1;
}

char *strncat(char *s1, const char *s2, size_t n)
{
	strncpy (s1 + strlen(s1), s2, n);
	return s1;
}

int strncmp(const char *s1, const char *s2, size_t n)
{
	size_t len = strlen(s1) + 1;
	if (n < len)
		len = n;

	return memcmp(s1, s2, len);
}

char *strcat(char *s1, const char *s2)
{
	return strncat(s1, s2, SIZE_MAX);
}

char *strchr(const char *s, int c)
{
	return memchr(s, c, strlen(s));
}

int strcmp(const char *s1, const char *s2)
{
	return strncmp(s1, s2, SIZE_MAX);
}

int strcoll(const char *s1, const char *s2)
{
	return strcmp(s1, s2);
}

char *strcpy(char *s1, const char *s2)
{
	return strncpy(s1, s2, SIZE_MAX);
}

size_t strcspn(const char *s1, const char *s2)
{
	for (size_t i = 0; i < strlen(s1); i++) {
		for (size_t j = 0; j < strlen(s2); j++) {
			if (s1[i] == s2[j])
				return i;
		}
	}

	return strlen(s1);
}

const char *sstrerror(int errno __attribute__((unused)))
{
	return "Unknown error";
}

char *strpbrk(const char *s1, const char *s2)
{
	size_t pos = strcspn(s1, s2);

	if (pos == strlen(s1))
		return NULL;

	return (char *)s1 + pos; // разконстанчивание ;(
}

char *strrchr(const char *s, int c)
{
	for (int i = strlen(s); i >= 0; i--) {
		if (s[i] == c)
			return (char *)&(s[i]);	// разконстанчивание
	}

	return NULL;
}

size_t strspn(const char *s1, const char *s2)
{
	for (size_t i = 0; i < strlen(s1); i++) {
		int f = 0;
		for (size_t j = 0; j < strlen(s2); j++) {
			if (s1[i] == s2[j])
				f++;
		}

		if (f == 0)
			return i;
	}

	return strlen(s1);
}

char *strstr(const char *s1, const char *s2)
{
	for (char *s = (char *)s1; *s != 0; s++) { // Разконстанчивание
		if (strncmp(s, s2, strlen(s2)) == 0)
			return s;
	}

	return NULL;
}

char *strtok(char *s1, const char *s2)
{
	static char *ts = NULL;

	if (s1 != NULL)
		ts = s1;

	if (ts == NULL || *ts == 0)
		return NULL;

	ts += strspn (ts, s2);
	if (*ts == 0)
		return NULL;

	char *r = ts;
	ts = strpbrk (ts, s2);

	if (ts) {
		*ts = 0;
		ts++;
	}

	return r;
}

size_t strxfrm(char *s1, const char *s2, size_t n)
{
	strncpy (s1, s2, n);
	return strlen (s1);
}
