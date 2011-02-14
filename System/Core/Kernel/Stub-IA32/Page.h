//
// Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

struct KernelInfoMemory;

// Менеджер страниц
enum PAGE_FLAGS {
	// Стандартные флаги страниц IA32
	PFLAG_PRESENT	= (1 << 0),

	PFLAG_READABLE	= (0 << 1),
	PFLAG_WRITABLE	= (1 << 1),

	PFLAG_SYSTEM	= (0 << 2),
	PFLAG_USER	= (1 << 2),

	PFLAG_GLOBAL	= (1 << 8),

	PFLAG_MASK	= 0x00000fff,

	// Дополнительные высокоуровневые флаги

	// Страница имеет линейный адрес в пространстве ядра
	PFLAG_KERNEL	= (1 << 12),

	// Временно в ядре
	PFLAG_TEMPORARY = (1 << 13),

	// Жестко связана с физическим адресом. Нельзя копировать.
	PFLAG_STATIC	= (1 << 14),

	// Страница в списке свободных.
	PFLAG_FREE	= (1 << 15),
};

typedef struct _PageRegion	PageRegion;

struct _PageInfo {
	paddr_t		paddr;	// Физический адрес
	laddr_t 	kaddr;	// Адрес в пространстве ядра
	uint32_t	flags;

	// Специализированный список в данном случае предпочтительнее.
	PageInfo 	*next, *prev;

	PageInstance	*instances; // Список инстанций

	uint32_t	reserved;
} __attribute__ ((packed));

// Через размер void это надо выразить ради тестирования на EM64T
// Хотя жэто можно просто перенести в юниттесты и в зависимости от разрядности проверять.
STATIC_ASSERT(sizeof(struct _PageInfo) == 32);
STATIC_ASSERT(sizeof(PageInfo) == 32);

struct _PageInstance {
	uint32_t flags;
	PageInfo *page;
	const void *resource;
	PageInstance *next;
};

struct _PageRegion {
	paddr_t		base;
	sizex_t		size;
	uint32_t	flags;
	PageInfo	*pages;
};

void StubPageRelease (PageInfo *page);

void StubInitPage (void);
PageInfo *StubGetPageByPAddr (paddr_t addr);
void StubCreatePageRegion (paddr_t base, sizex_t size, uint32_t flags);
void StubPageInitMode ();
bool StubMemoryReadable (laddr_t addr, size_t size);
paddr_t StubPageGetPAddr (laddr_t laddr);
void StubPageUnmap (PageInfo *page, laddr_t addr);
paddr_t StubGetPAddrByLAddr (laddr_t addr);
unsigned long StubPageDescriptor (paddr_t addr, unsigned int flags);
void StubPageInitPDir (const laddr_t ldir);
void StubPageTaskInit (Task *task);
void StubPageTaskDestroy (Task *task);

// Эти низкоуровневые примитивы находятся в Stublo.s
void StubPageFlush ();
bool StubIsPaged (void);

void StubInitCR3 (laddr_t cr3);
laddr_t StubGetCR3 ();

sizex_t StubGetMemoryTotal();
sizex_t StubGetMemoryUsed();

void StubCalcMemoryUsage(struct KernelInfoMemory *info);

void StubPageAllocByInfo(PageInfo *page);

