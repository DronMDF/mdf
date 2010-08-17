//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Descriptor.h"

#define KERNEL_CODE_SELECTOR	((1 * sizeof (descriptor_t)) | SELECTOR_RPL0)
#define KERNEL_DATA_SELECTOR	((2 * sizeof (descriptor_t)) | SELECTOR_RPL0)

#define USER_CODE_SELECTOR	((4 * sizeof (descriptor_t)) | SELECTOR_RPL3)
#define USER_DATA_SELECTOR	((5 * sizeof (descriptor_t)) | SELECTOR_RPL3)

enum GDT_IDX {
	GDT_CPU_BASE	= 2048,
	GDT_TASK_BASE	= 4096,
	GDT_SIZE	= 8192,
};

#define STUB_MAX_TASK_COUNT	4096
#define STUB_MAX_CPU_COUNT	2048

STATIC_ASSERT (GDT_CPU_BASE + STUB_MAX_CPU_COUNT == GDT_TASK_BASE);
STATIC_ASSERT (GDT_TASK_BASE + STUB_MAX_TASK_COUNT == GDT_SIZE);

extern volatile descriptor_t *GDT __deprecated__;

void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags) __init__;

void StubTssSetDescriptor(unsigned int ti, const descriptor_t tssd);
