//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

size_t StubStringLength(const char *str);
void StubStringCopy(char *ostr, const char *istr);
bool StubStringEqual (const char *ostr, const char *istr, size_t len);
