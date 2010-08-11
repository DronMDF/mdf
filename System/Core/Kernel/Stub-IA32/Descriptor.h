//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef union {
	// TODO: стоит отказаться от 4-х битовых полей - варнинги

	struct {
		unsigned int limitlo:16;
		unsigned int baselo:24;
		unsigned int flagslo:8;
		unsigned int limithi:4;
		unsigned int flagshi:4;
		unsigned int basehi:8;
	}  __attribute__ ((packed)) segment;
	struct {
		unsigned int offsetlo:16;
		unsigned int selector:16;
		unsigned int params:4;
		unsigned int reserved:4;
		unsigned int flags:8;
		unsigned int offsethi:16;
	}  __attribute__ ((packed)) gate;
	unsigned long long raw;
} descriptor_t;

STATIC_ASSERT (sizeof (descriptor_t) == 8);

// Старшие 4 бита флагов располагаются в 8-11 битах
// Может не самый оптимальный вариант, но пусть компилятор оптимизирует.

enum DESCRIPTOR_FLAGS {
	DESCRIPTOR_PRESENT	= 0x080,
	DESCRIPTOR_USE32 	= 0x400,
	DESCRIPTOR_GRANULARITY	= 0x800
};

enum DESCRIPTOR_PL {
	DESCRIPTOR_PL0		= 0 << 5,
	DESCRIPTOR_PL1		= 1 << 5,
	DESCRIPTOR_PL2		= 2 << 5,
	DESCRIPTOR_PL3		= 3 << 5
};

// Чтобы со сдвигами не намудрить :)
STATIC_ASSERT (DESCRIPTOR_PL3 == 0x060);

enum DESCRIPTOR_TYPE {
	DESCRIPTOR_TASK		= 0x009,
	DESCRIPTOR_TASK_BUSY	= 0x00b,
	DESCRIPTOR_INTERRUPT	= 0x00e,
	DESCRIPTOR_DATA		= 0x012,
	DESCRIPTOR_CODE		= 0x018,

	DESCRIPTOR_TYPE		= 0x01f	// Маска
};


enum SELECTOR_PL {
	SELECTOR_RPL0	= 0,
	SELECTOR_RPL1	= 1,
	SELECTOR_RPL2	= 2,
	SELECTOR_RPL3	= 3
};

#define KERNEL_CODE_SELECTOR	((1 * sizeof (descriptor_t)) | SELECTOR_RPL0)
#define KERNEL_DATA_SELECTOR	((2 * sizeof (descriptor_t)) | SELECTOR_RPL0)

#define USER_CODE_SELECTOR	((4 * sizeof (descriptor_t)) | SELECTOR_RPL3)
#define USER_DATA_SELECTOR	((5 * sizeof (descriptor_t)) | SELECTOR_RPL3)

descriptor_t StubGenerateSegmentDescriptor(laddr_t base, size_t size, int flags);
size_t StubDescriptorGetSize(const descriptor_t descriptor);
laddr_t StubDescriptorGetBase(descriptor_t descriptor);

void StubSetSegmentDescriptorBySelector(int selector, laddr_t base, size_t size, int flags) __init__;
int StubGetSegmentFlags(int di) __deprecated__;
