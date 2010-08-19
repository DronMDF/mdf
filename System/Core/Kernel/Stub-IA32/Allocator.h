//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct {
} AllocPage;

typedef struct {
	uint32_t avail;
	AllocPage *pages[1023];
} AllocDir;

STATIC_ASSERT(sizeof(AllocDir) == 4096);

