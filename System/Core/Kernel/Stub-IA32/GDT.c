//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include "GDT.h"

// GDT utility

void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags)
{
	const int di = selector / (int)sizeof(descriptor_t);
	GDT[di] = StubDescriptorGenerate(base, size, flags);
}

// TSS utility

void StubTssSetDescriptor(unsigned int ti, const descriptor_t tssd)
{
	STUB_ASSERT (ti >= STUB_MAX_TASK_COUNT, "Invalid task slot");
	const unsigned int di = GDT_TASK_BASE + ti;
	GDT[di] = tssd;
}

unsigned int StubTssGetSelector(unsigned int ti)
{
	STUB_ASSERT (ti >= STUB_MAX_TASK_COUNT, "Invalid task slot");
	const unsigned int selector = (GDT_TASK_BASE + ti) * sizeof(descriptor_t);
	return selector;
}

