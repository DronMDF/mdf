//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct AllocPage_ {
	laddr_t base;
	struct AllocPage_ *next;
	uint32_t *map;
	size_t block_size;
} AllocPage;

STATIC_ASSERT(sizeof(AllocPage) == 16);

typedef struct {
	uint32_t avail;
	AllocPage *pages[1023];
} AllocDir;

STATIC_ASSERT(sizeof(AllocDir) == 4096);

typedef struct StubAllocatorAllocFunctions_ {
	AllocPage *(*newPage)(size_t);
} StubAllocatorAllocFunctions;
