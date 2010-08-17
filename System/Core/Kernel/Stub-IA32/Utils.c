//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>

#include "StubLocal.h"

size_t StubStringLength (const char * const str)
{
	STUB_ASSERT (str == nullptr, "No string");

	for (size_t i = 0; ; i++) {
		if (str[i] == '\0')
			return i;
	}

	return 0;
}

void StubStringCopy (char * const ostr, const char *istr)
{
	STUB_ASSERT (istr == nullptr, "Not source string");
	STUB_ASSERT (ostr == nullptr, "Not destination string");

	for (size_t i = 0; ; i++) {
		ostr[i] = istr[i];
		if (ostr[i] == '\0')
			break;
	}
}

bool StubStringEqual (const char * const ostr, const char * const istr, const size_t len)
{
	STUB_ASSERT (istr == nullptr, "Not first string");
	STUB_ASSERT (ostr == nullptr, "Not second string");

	for (size_t i = 0; i < len; i++) {
		if (ostr[i] != istr[i])
			return false;

		if (ostr[i] == '\0')
			break;
	}

	return true;
}
