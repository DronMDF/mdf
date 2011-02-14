//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

// В этом файле у меня конечно бардак... ну постепенно буду разгребать

#define nullptr ((void *)0)

#ifndef NULL
	#define NULL ((void *)0)
#endif

#define VIDEO0_PAGE	0xb8000

// Compare and swap
#define CAS(ptr, old, new) __sync_bool_compare_and_swap((ptr), (old), (new))

int StubGetChar (void);
bool StubSetConsole (const char *type);

// Операции с портами ввода-вывода
static inline
void __deprecated__ outbyte (unsigned short port, unsigned char value)
{
	__asm__ __volatile__ (
		"outb %b0, %w1" : :
		"a"(value), "Nd"(port));
}

static inline
unsigned char __deprecated__ inbyte (unsigned short port)
{
        unsigned char value;
        __asm__ __volatile__ (
		"inb %w1, %b0"
		: "=a"(value)
		: "Nd" (port));
        return value;
}

static inline
void __deprecated__ iowait (void)
{
        __asm__ __volatile__ (
		"outb %al, $0x80");
}

// Разные утилитки
void StubBootstrapCaller(laddr_t entry, uint32_t tss_sel);

void __noreturn__ StubBootstrapEntry (void);

// Преобразование типов..
// Инлайны предпочтительнее, ибо обеспечивают более строгий контроль типов.
static inline
paddr_t l2paddr (const laddr_t laddr)
{
	return laddr;
}

static inline
laddr_t p2laddr (const paddr_t paddr)
{
	STUB_ASSERT (paddr >= 0x100000000LL, "Big paddr for laddr");
	return (laddr_t)paddr;
}

static inline
laddr_t v2laddr (const void * const ptr)
{
	return (laddr_t)ptr;
}

static inline
paddr_t v2paddr (const void * const ptr)
{
	return l2paddr(v2laddr(ptr));
}

static inline
void * l2vptr (const laddr_t laddr)
{
	return (void *)laddr;
}

static inline
void * p2vptr (const paddr_t paddr)
{
	return l2vptr(p2laddr(paddr));
}

typedef laddr_t uaddr_t;	// Адрес в пользовательском пространстве.

static inline
laddr_t u2laddr (const uaddr_t uaddr)
{
	STUB_ASSERT (uaddr >= USER_MEMORY_SIZE, "Invalid uaddr");
	if (uaddr == 0) return 0;
	return uaddr + USER_MEMORY_BASE;
}

static inline
uaddr_t l2uaddr (const laddr_t laddr)
{
	if (laddr == 0) return 0;

	STUB_ASSERT (laddr < USER_MEMORY_BASE ||
		laddr >= USER_MEMORY_BASE + USER_MEMORY_SIZE,
		"Invalid laddr for uaddr");

	return laddr - USER_MEMORY_BASE;
}

static inline
size_t x2size (const sizex_t xsize)
{
	STUB_ASSERT (xsize >= 0x100000000LL, "Big xsize for size");
	return (size_t)xsize;
}

// Interrupt
void StubInterruptControllerInit (void);
void StubInterruptUnmask(int irq);
bool StubInterruptIsMasked (int irq);
void StubInterruptAcknowledge (int irq);

// Эти две функции являются дефайнами, потому что могут работать с любыми типами.

// Проверка флагов. Вместо того чтобы постоянно писать (value & flag) != 0
#define isSet(v,f)	(((v) & (f)) == (f))

// Проверка выравнивания.
// Выражения addr % PAGE_SIZE == 0 или addr & (PAGE_SIZE - 1) == 0 крайне ненаглядны.
#define isAligned(v,a)	((v) % (a) == 0)

// Эти функции мне не очень нравятся...
#define	round(v, d)	((d) > 0 ? ((v) - (v) % (d)) : (v))
#define	round_up(v, d)	round((v) + (d) - 1, d)

#define min(a, b)      ((a) < (b) ? (a) : (b))
#define max(a, b)      ((a) > (b) ? (a) : (b))
