//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include <limits.h>
#include <stdint.h>

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>

#define __init__
#define __initdata__

#define __unused__	__attribute__((unused))
#define __deprecated__	__attribute__((deprecated))
#define __noreturn__	__attribute__((noreturn))

// Хотелось бы поставить BOOST_REQUIRE_MESSAGE, но при уничтожении статических
// объектов это может вызвать сегфолт.
#define STUB_FATAL(msg)		BOOST_ASSERT(false)
#define STUB_ASSERT(exp, msg)	BOOST_ASSERT(!(exp))
#define STATIC_ASSERT(exp)	BOOST_STATIC_ASSERT(exp)

#define PAGE_SIZE 0x00001000UL
#define LADDR_MASK 0xfffff000UL
#define PADDR_MASK 0xfffffffffffff000ULL

#define USER_MEMORY_BASE 0x100000

#define USER_STACK_SIZE	PAGE_SIZE
#define USER_TXA_SIZE	(PAGE_SIZE * 1024)

#define USER_STACK_BASE	(0xffff0000)
#define USER_TXA_BASE	(USER_STACK_BASE - USER_TXA_SIZE)

#define USER_PAGETABLE_BASE 0
#define USER_PAGETABLE_SIZE 0

typedef unsigned long laddr_t;
typedef uint64_t paddr_t;
typedef unsigned long offset_t;
typedef volatile int lock_t;

typedef uint32_t timeout_t;
typedef uint64_t tick_t;

static const tick_t TIMESTAMP_FUTURE = 0xffffffffffffffffULL;
static const timeout_t TIMEOUT_INFINITY = 0xffffffffU;

struct StubStackFrame {
	uint32_t	flags;
	size_t		txa_size;
	laddr_t		txa_ptr;
	id_t		caller;
	laddr_t		retmagic;
};

// Специальные функции для тестовых целей.
void TestIncrementCurrentClock(tick_t increment);
void TestSetStubTaskDestroyReaction(int react);

