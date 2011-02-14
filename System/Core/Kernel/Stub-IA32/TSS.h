//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#define offsetof(type, field)  ((unsigned long)(&(((type *)0)->field)))

typedef struct {
	unsigned long link;
	unsigned long esp0, ss0;
	unsigned long esp1, ss1;
	unsigned long esp2, ss2;
	unsigned long cr3;
	unsigned long eip;
	unsigned long eflags;
	unsigned long eax;
	unsigned long ecx;
	unsigned long edx;
	unsigned long ebx;
	unsigned long esp;
	unsigned long ebp;
	unsigned long esi;
	unsigned long edi;
	unsigned long es;
	unsigned long cs;
	unsigned long ss;
	unsigned long ds;
	unsigned long fs;
	unsigned long gs;
	unsigned long ldt;
	unsigned short trace;
	unsigned short iomap_offset;

	size_t iomap_size;
	Task *task;
	unsigned int slot;

	unsigned char iomap[];
} __attribute__ ((packed)) tss_t;

STATIC_ASSERT(offsetof(tss_t, iomap_size) == 104);

// Биты eflags
enum {
	EFLAGS_INTERRUPT_ENABLE = (1 << 9),
};

