//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include "StubLocal.h"
#include "Descriptor.h"

descriptor_t StubDescriptorGenerate(laddr_t base, size_t size, int flags)
{
	descriptor_t descriptor = { .raw = 0 };

	descriptor.segment.baselo = base & 0x00ffffff;
	descriptor.segment.basehi = (unsigned char)((base & 0xff000000) >> 24);

	if (size == 0) {
		// Полный flat
		flags |= DESCRIPTOR_GRANULARITY;
		size = 0x100000000L / PAGE_SIZE;
	} else if (size > 0x100000) {
		flags |= DESCRIPTOR_GRANULARITY;
		size = (size + PAGE_SIZE - 1) / PAGE_SIZE;
	}

	const size_t limit = size - 1;
	descriptor.segment.limitlo =  limit & 0x0ffff;
	descriptor.segment.limithi = (limit & 0xf0000) >> 16;

	flags |= DESCRIPTOR_PRESENT;
	descriptor.segment.flagslo = (unsigned char)(flags & 0x0ff);
	descriptor.segment.flagshi = (flags & 0xf00) >> 8;
	
	return descriptor;
}

laddr_t StubDescriptorGetBase(const descriptor_t descriptor)
{
	return (laddr_t)((descriptor.segment.basehi << 24) | descriptor.segment.baselo);
}

size_t StubDescriptorGetSize(const descriptor_t descriptor)
{
	size_t size = (size_t)((descriptor.segment.limitlo | 
			(descriptor.segment.limithi << 16)) + 1);
	if (isSet(descriptor.segment.flagshi << 8, DESCRIPTOR_GRANULARITY)) {
		size *= PAGE_SIZE;
	}
	return size;
}

int StubDescriptorGetFlags(const descriptor_t descriptor)
{
	int flags = descriptor.segment.flagslo | (descriptor.segment.flagshi << 8);
	return flags & ~(DESCRIPTOR_PRESENT | DESCRIPTOR_GRANULARITY);
}
