//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include "StubLocal.h"
#include "Descriptor.h"

extern volatile descriptor_t *GDT;

descriptor_t StubGenerateSegmentDescriptor(laddr_t base, size_t size, int flags)
{
	descriptor_t descriptor = { .raw = 0 };

	descriptor.segment.baselo = base & 0x00ffffff;
	descriptor.segment.basehi = (base & 0xff000000) >> 24;

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
	descriptor.segment.flagslo =  flags & 0x0ff;
	descriptor.segment.flagshi = (flags & 0xf00) >> 8;
	
	return descriptor;
}

void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags)
{
	const unsigned int di = selector / sizeof (descriptor_t);
	GDT[di] = StubGenerateSegmentDescriptor(base, size, flags);
}

laddr_t StubGetSegmentBase(int di)
{
	return (GDT[di].segment.basehi << 24) | GDT[di].segment.baselo;
}

size_t StubDescriptorGetSize(const descriptor_t descriptor)
{
	size_t size = (descriptor.segment.limitlo | (descriptor.segment.limithi << 16)) + 1;
	if (isSet(descriptor.segment.flagshi << 8, DESCRIPTOR_GRANULARITY)) {
		size *= PAGE_SIZE;
	}
	return size;
}

size_t StubGetSegmentSize(int di)
{
	size_t size = (GDT[di].segment.limitlo | (GDT[di].segment.limithi << 16)) + 1;
	if (isSet(GDT[di].segment.flagshi, DESCRIPTOR_GRANULARITY >> 8))
		size *= PAGE_SIZE;
	return size;
}

int StubGetSegmentFlags(int di)
{
	return GDT[di].segment.flagslo | (GDT[di].segment.flagshi << 8);
}
