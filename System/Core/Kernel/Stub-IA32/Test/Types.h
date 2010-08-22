//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <sys/resource.h>

#ifdef __cplusplus
#include <boost/test/unit_test.hpp>
#endif

#define __init__
#define __initdata__

#define __deprecated__ __attribute__((deprecated))
#define __noreturn__ __attribute__((noreturn))

#define STUB_FATAL(msg)		assert(false)
#define STUB_ASSERT(exp, msg)	assert(!(exp))
#define STATIC_ASSERT(exp)	extern char __static_assert[]

typedef unsigned long laddr_t;
typedef uint64_t paddr_t;
typedef unsigned long offset_t;
typedef uint64_t sizex_t;
typedef uint32_t timeout_t;
typedef volatile uint32_t lock_t;
typedef uint64_t tick_t;

#define PAGE_SIZE 4096U
#define PADDR_MASK (~(PAGE_SIZE - 1))

#define KERNEL_TEMP_BASE 0x3000000

#define USER_MEMORY_SIZE 0xd0000000
#define USER_MEMORY_BASE 0x10000000

#include <Stub.h>
