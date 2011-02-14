//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Kernel.h>

uint32_t stubIOMap[65536 / sizeof(uint32_t)];

void StubIOInit()
{
	StubMemoryClear(stubIOMap, sizeof(stubIOMap));
}

int StubIOReserve(unsigned int first, unsigned int last)
{
	if (first > last) {
		return ERROR_INVALIDPARAM;
	}
	
	for (unsigned int i = first; i <= last; i++) {
		const unsigned int idx = i / sizeof(uint32_t);
		const unsigned int offs = i % sizeof(uint32_t);
		if ((stubIOMap[idx] & (1 << offs)) != 0) {
			return ERROR_BUSY;
		}
	}

	for (unsigned int i = first; i <= last; i++) {
		const unsigned int idx = i / sizeof(uint32_t);
		const unsigned int offs = i % sizeof(uint32_t);
		stubIOMap[idx] |= 1 << offs;
	}
	
	return SUCCESS;
}
