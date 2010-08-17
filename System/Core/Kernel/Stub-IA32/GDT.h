//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Descriptor.h"
#include "TSS.h"

#define KERNEL_CODE_SELECTOR	((1 * sizeof (descriptor_t)) | SELECTOR_RPL0)
#define KERNEL_DATA_SELECTOR	((2 * sizeof (descriptor_t)) | SELECTOR_RPL0)

#define USER_CODE_SELECTOR	((4 * sizeof (descriptor_t)) | SELECTOR_RPL3)
#define USER_DATA_SELECTOR	((5 * sizeof (descriptor_t)) | SELECTOR_RPL3)

void StubInitGDT() __init__;

void StubSetSegmentCPU(unsigned int ci, laddr_t base, size_t size);
unsigned int StubGetSelectorCPU(unsigned int ci);

tss_t *StubGetTaskContextBySlot(unsigned int slot);
bool StubTaskSlotCanRelease(unsigned int slot);
void StubTaskSlotUnuse(unsigned int slot);
void StubTaskSlotUse(tss_t *tss);
unsigned int StubTaskSlotBySelector(unsigned int selector);
