//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#define STUB_FATAL(msg)		StubSoD (msg, __FILE__, __LINE__)
#define STUB_ASSERT(exp, msg)	if (exp) { StubSoD ((msg), __FILE__, __LINE__); } else {}
#define STATIC_ASSERT(exp)	extern char __static_assert[(exp) ? 1 : -1]

// Секции
#define __init__		__attribute__((section (".init.text")))
#define __initdata__		__attribute__((section (".init.data")))

// Атрибуты
#define __unused__		__attribute__((unused))
#define __deprecated__		__attribute__((deprecated))
#define __noreturn__		__attribute__((noreturn))

// Стандартные типы си

typedef signed char int8_t;
typedef unsigned char uint8_t;
typedef signed short int16_t;
typedef unsigned short uint16_t;
typedef signed long int32_t;
typedef unsigned long uint32_t;
typedef signed long long int64_t;
typedef unsigned long long uint64_t;

// Типы платформы
#ifndef __size_t_are_declared
typedef uint32_t size_t;
#define __size_t_are_declared 1
#endif

typedef uint64_t sizex_t;
typedef uint32_t timeout_t;
typedef uint32_t id_t;
typedef uint32_t laddr_t;
typedef uint32_t offset_t;
typedef uint64_t paddr_t;
typedef uint64_t clock_t;

static const clock_t TIMESTAMP_FUTURE = 0xffffffffffffffffULL;
static const clock_t __attribute__((deprecated)) CLOCK_MAX = TIMESTAMP_FUTURE;
static const timeout_t TIMEOUT_INFINITY = 0xffffffffU;

typedef volatile uint32_t lock_t;

static const size_t PAGE_SIZE  = 4096;
static const size_t PDIR_SIZE = PAGE_SIZE * 1024;
static const sizex_t LINEAR_MEMORY_SIZE = 0x100000000ULL;

static const uint32_t LADDR_MASK = 0xfffff000U;
static const uint64_t PADDR_MASK = 0xfffffffffffff000ULL;

static const size_t MEBIBYTE = 1024 * 1024;

// -----------------------------------------------------------------------------
// Распределение памяти ядра
static const size_t KERNEL_MEMORY_SIZE = 256 * MEBIBYTE;	// 256MiB

static const size_t KERNEL_TEMP_SIZE = 4 * MEBIBYTE;
static const size_t KERNEL_PAGETABLE_SIZE = 8 * MEBIBYTE;	// С рассчетом на PAE

static const laddr_t KERNEL_TEMP_BASE =  KERNEL_MEMORY_SIZE - KERNEL_PAGETABLE_SIZE - KERNEL_TEMP_SIZE;

static const laddr_t KERNEL_PAGETABLE_BASE = KERNEL_MEMORY_SIZE - KERNEL_PAGETABLE_SIZE;

static const laddr_t KERNEL_STACK_HOLE = LINEAR_MEMORY_SIZE - PAGE_SIZE * 2;
static const laddr_t KERNEL_STACK_BASE = KERNEL_STACK_HOLE + PAGE_SIZE;
static const size_t KERNEL_STACK_SIZE = LINEAR_MEMORY_SIZE - KERNEL_STACK_HOLE;

STATIC_ASSERT(KERNEL_TEMP_BASE % PDIR_SIZE == 0);
STATIC_ASSERT(KERNEL_PAGETABLE_BASE % PDIR_SIZE == 0);

// -----------------------------------------------------------------------------
// Распределение памяти приложения.
static const laddr_t USER_MEMORY_BASE = KERNEL_MEMORY_SIZE;
static const size_t USER_MEMORY_SIZE = KERNEL_STACK_HOLE - USER_MEMORY_BASE;
// TXA - порядка 4МиБ без небольшого...
static const laddr_t USER_TXA_HOLE = LINEAR_MEMORY_SIZE - PDIR_SIZE + PAGE_SIZE * 16;
static const laddr_t USER_TXA_BASE = USER_TXA_HOLE + PAGE_SIZE;
static const size_t USER_TXA_SIZE = KERNEL_STACK_HOLE - USER_TXA_BASE;
// Стек порядка 12МиБ с небольшим
static const laddr_t USER_STACK_HOLE = LINEAR_MEMORY_SIZE - PDIR_SIZE * 4;
static const laddr_t USER_STACK_BASE = USER_STACK_HOLE + PAGE_SIZE;
static const size_t USER_STACK_SIZE = USER_TXA_HOLE - USER_STACK_BASE;

static const size_t USER_CODE_SIZE = USER_STACK_BASE - USER_MEMORY_BASE;

static const laddr_t USER_PAGETABLE_BASE = KERNEL_PAGETABLE_BASE + USER_MEMORY_BASE / PAGE_SIZE * 4;
static const size_t USER_PAGETABLE_SIZE = USER_MEMORY_SIZE / PAGE_SIZE * 4;

STATIC_ASSERT (USER_MEMORY_BASE % PDIR_SIZE == 0);
STATIC_ASSERT (USER_MEMORY_BASE + USER_MEMORY_SIZE <= LINEAR_MEMORY_SIZE);
STATIC_ASSERT(USER_PAGETABLE_BASE % PAGE_SIZE == 0);

// -----------------------------------------------------------------------------
struct StubStackFrame {
	laddr_t		retmagic;
	id_t		caller;
	laddr_t		txa_ptr;
	size_t		txa_size;
	uint32_t	flags;
};
