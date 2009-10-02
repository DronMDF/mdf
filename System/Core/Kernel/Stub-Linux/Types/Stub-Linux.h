//
// Copyright (c) 2000-2008 Валяев Андрей (dron@infosec.ru)
// All rights reserved.
//
// Created: 05/02/08 14:06:06
//

#pragma once

#define STUB_CPP

#include <time.h>
#include <sys/types.h>	// для id_t
#include <stddef.h>	// для size_t

typedef unsigned long timeout_t;

#define __init__
#define __initdata__

#define StubClockCount()	clock()
void StubPrint (const char *format, ...);
