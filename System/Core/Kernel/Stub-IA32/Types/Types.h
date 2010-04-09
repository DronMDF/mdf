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
typedef uint64_t tick_t;

typedef volatile uint32_t lock_t;

// -----------------------------------------------------------------------------
static const tick_t TIMESTAMP_FUTURE = 0xffffffffffffffffULL;
static const timeout_t TIMEOUT_INFINITY = 0xffffffffU;

// Эту штуку вообще надо выкинуть нафиг
static const size_t PDIR_SIZE = 0x400000; //PAGE_SIZE * 1024;

static const uint32_t LADDR_MASK = 0xfffff000U;
static const uint64_t PADDR_MASK = 0xfffffffffffff000ULL;

#define PAGE_SIZE	4096U

#define MEBIBYTE (1024U * 1024U)

// -----------------------------------------------------------------------------
// Распределение памяти

#define KERNEL_MEMORY_SIZE	(256U * MEBIBYTE)

// Неперекрывающиеся области памяти в порядке следования
#define KERNEL_TEMP_SIZE	(4U * MEBIBYTE)
#define KERNEL_PAGETABLE_SIZE	(1048576U * 8U) // 1048576 страниц по 8 байт на дескриптор (PAE)
#define USER_CODE_SIZE		((4096U - 256U - 16U) * MEBIBYTE - PAGE_SIZE)
//	дырка			PAGE_SIZE
#define USER_STACK_SIZE		(12U * MEBIBYTE + 16U * PAGE_SIZE)
//	дырка			PAGE_SIZE
#define USER_TXA_SIZE		(4U * MEBIBYTE - 19U * PAGE_SIZE)
//	дырка			PAGE_SIZE
#define KERNEL_STACK_SIZE 	PAGE_SIZE

// Базовые адреса неперекрывающихся областей
#define KERNEL_TEMP_BASE	(KERNEL_PAGETABLE_BASE - KERNEL_TEMP_SIZE)
#define KERNEL_PAGETABLE_BASE	(KERNEL_MEMORY_SIZE - KERNEL_PAGETABLE_SIZE)
#define USER_MEMORY_BASE	KERNEL_MEMORY_SIZE
#define USER_STACK_BASE		(USER_MEMORY_BASE + USER_CODE_SIZE + PAGE_SIZE)
#define USER_TXA_BASE		(USER_STACK_BASE + USER_STACK_SIZE + PAGE_SIZE)
#define KERNEL_STACK_BASE	(USER_TXA_BASE + USER_TXA_SIZE + PAGE_SIZE)

#define USER_MEMORY_SIZE	(KERNEL_STACK_BASE - USER_MEMORY_BASE - PAGE_SIZE)

// USER_PAGETABLE_* зависит от использования PAE
#define USER_PAGETABLE_BASE	(KERNEL_PAGETABLE_BASE + USER_MEMORY_BASE / PAGE_SIZE * 4U)
#define USER_PAGETABLE_SIZE	(USER_MEMORY_SIZE / PAGE_SIZE * 4U)

// -----------------------------------------------------------------------------
// Распределение памяти приложения.

// -----------------------------------------------------------------------------
struct StubStackFrame {
	laddr_t		retmagic;
	id_t		caller;
	laddr_t		txa_ptr;
	size_t		txa_size;
	uint32_t	flags;
};
