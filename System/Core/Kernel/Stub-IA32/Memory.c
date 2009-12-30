//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Stub.h>
#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Memory.h"
#include "List.h"

#define MEMORY_GARBAGED
#define MEMORY_PROTECTOR

// -----------------------------------------------------------------------------
// Утилиты
void StubMemoryCopy (void * const dst, const void * const src, const size_t count)
{
	STUB_ASSERT (src == nullptr, "No source buffer");
	STUB_ASSERT (dst == nullptr, "No destination buffer");

	for (size_t i = 0; i < count; i++)
		((char *)dst)[i] = ((char *)src)[i];
}

bool StubMemoryEqual (const void * const dst, const void * const src, const size_t count)
{
	STUB_ASSERT (src == nullptr, "No source buffer");
	STUB_ASSERT (dst == nullptr, "No destination buffer");

	for (size_t i = 0; i < count; i++) {
		if (((char *)dst)[i] != ((char *)src)[i])
			return false;
	}

	return true;
}

void StubMemoryClear (void *const dst, const size_t count)
{
	STUB_ASSERT (dst == nullptr, "No buffer");

	for (size_t i = 0; i < count; i++)
		((char *)dst)[i] = 0;
}

// Замусоривание памяти - отладочная функция...
void StubMemoryRefuse (void * const dst, const size_t count)
{
	STUB_ASSERT (dst == nullptr, "No buffer");

	for (size_t i = 0; i < count; i++) {
		((unsigned char *)dst)[i] = 0xaa;
	}
}

// -----------------------------------------------------------------------------
// Менеджер памяти

typedef struct _MCB MCB;
typedef LISTITEM(MCB) MCBLink;
typedef struct _MCBHead MCBHead;

STATIC_ASSERT (sizeof (MCBLink) == 12);

struct _MCB {
#ifdef MEMORY_PROTECTOR
	unsigned long prot1;
#endif
	size_t size_prev;
	size_t size;

	// Возможно, для реаллока придется сохранить выравнивание блока

#ifdef MEMORY_PROTECTOR
	unsigned long prot2;
#endif

	MCBLink link[0];
};

#ifdef MEMORY_PROTECTOR
STATIC_ASSERT (sizeof (MCB) == 16);
#else
STATIC_ASSERT (sizeof (MCB) == 8);
#endif

struct _MCBHead {
	MCB *first;
	size_t size;
	LIST(MCB) free;
};

#define MCB_USED	1
// Этим битом маркируется последний и терминатор перед первым.
#define MCB_LAST	2

// Вспомогательные дефайны...
#define isUsed(block)		(((block)->size & MCB_USED) != 0)
#define isPreviousUsed(block)	(((block)->size_prev & MCB_USED) != 0)

#define isFirst(block)		(((block)->size_prev & MCB_LAST) != 0)
#define isLast(block)		(((block)->size & MCB_LAST) != 0)

#define BlockSize(block)	((block)->size & ~(MCB_USED | MCB_LAST))
#define BlockPreviousSize(block)	((block)->size_prev & ~(MCB_USED | MCB_LAST))

#define BLOCK_MINSIZE		(sizeof (MCB) + sizeof (MCBLink))

static
MCB *StubMemoryNextBlock(const MCB * const block)
{
	if (isLast (block))
		return nullptr;

	return (MCB *)((char *)block + BlockSize(block) + sizeof (MCB));
}

static
MCB *StubMemoryPreviousBlock (const MCB * const block)
{
	if (isFirst (block))
		return nullptr;

	return (MCB *)((char *)block - BlockPreviousSize(block) - sizeof (MCB));
}

static
void StubMemoryEnqueue (MCBHead * const head, MCB * const block)
{
	#ifdef MEMORY_PROTECTOR
		block->prot1 = block->prot2 = 0xB10C1D1E;
	#endif

	ListItemLink (head->free, block, link[0]);
}

static
void StubMemoryDequeue (MCBHead * const head, MCB * const block)
{
	ListItemUnlink (head->free, block, link[0]);
}

static
MCB *StubMemoryDequeueAlign (MCBHead * const head, MCB * const block,
	const size_t size, const int align)
{
	STUB_ASSERT (isUsed (block), "Used block");
	STUB_ASSERT (!isAligned(size, 4), "Unaligned size");

	// Блок находится в списке свободных... выровненную часть необходимо отлинковать.

	const laddr_t aligned =	round_up (v2laddr(block->link) + BLOCK_MINSIZE, align);
	const size_t osize = BlockSize (block);

	if (osize < size + (aligned - v2laddr(block->link)))
		return nullptr;

	StubMemoryDequeue (head, block);

	const size_t asize = aligned - v2laddr(block->link) - sizeof (MCB);
	STUB_ASSERT (asize < sizeof (MCBLink), "Illegal underaligned block size");

	const unsigned int last = block->size & MCB_LAST;

	block->size = asize;
	StubMemoryEnqueue (head, block);

	MCB *next = StubMemoryNextBlock (block);
	STUB_ASSERT (next == nullptr, "Lost aligned block");
	next->size = (osize - asize - sizeof (MCB)) | last;
	next->size_prev = asize;

	// Нужно зафиксировать размер нового блока с обоих сторон.
	MCB *nnext = StubMemoryNextBlock (next);
	if (nnext != nullptr) {
		nnext->size_prev = next->size;
	}

	return next;
}

static
void StubMemorySplitBlock (MCBHead * const head, MCB * const block, const size_t size)
{
	STUB_ASSERT (isUsed (block), "Used block");
	STUB_ASSERT (!isAligned(size, 4), "Unaligned size");

	const size_t osize = BlockSize (block);

	if (size + BLOCK_MINSIZE <= osize) {
		unsigned int last = block->size & MCB_LAST;

		block->size = size;
		MCB *next = StubMemoryNextBlock (block);
		next->size = (osize - size - sizeof (MCB)) | last;
		next->size_prev = size;

		MCB *nnext = StubMemoryNextBlock(next);
		if (nnext != 0) {
			nnext->size_prev = next->size;
		}
		
		StubMemoryEnqueue (head, next);
	}
}

static
MCB *StubMemoryDefrag (MCBHead * const head, MCB *block)
{
	STUB_ASSERT (isSet (block->size, MCB_USED), "Used block");

	MCB *next = StubMemoryNextBlock (block);

	if (!isPreviousUsed (block)) {
		MCB *prev = StubMemoryPreviousBlock (block);
		if (prev != nullptr) {
			// Сливаем с предыдущим...
			StubMemoryDequeue (head, prev);
			// Бит MCB_LAST перенесется автоматичски
			prev->size += block->size + sizeof (MCB);
			block = prev;
		}
	}

	// NOTE: эти условия сливать не стоит,
	// next изменяется и его всеравно придется проверять.
	if (next != nullptr && !isUsed(next)) {
		// Сливаем со следующим.
		StubMemoryDequeue (head, next);
		block->size += next->size + sizeof (MCB);
		next = StubMemoryNextBlock (block);
	}

	if (next != nullptr) {
		next->size_prev = block->size;
	}

	return block;
}

static
void __init__ StubMemoryCheck (const MCBHead * const head)
{
	void *end = (char *)(head->first) + head->size;

	MCB *b = head->first;
	STUB_ASSERT (!isFirst(b), "Not first block");

	for (MCB *a = StubMemoryNextBlock (b); a != nullptr;
		b = a, a = StubMemoryNextBlock (b))
	{
		#ifdef MEMORY_PROTECTOR
		STUB_ASSERT (a->prot1 != a->prot2, "Protection corrupted");
		#endif
		STUB_ASSERT (b->size != a->size_prev, "Blocks corrupted");

		if (isLast(a)) {
			STUB_ASSERT ((char *)a + BlockSize (a) + sizeof (MCB) != end, "Illegal Last Block");
		}
	}
}

static
void __init__ StubMemoryInit (MCBHead * const head,
	void * const ptr, const size_t size, MCBHead * const oldhead)
{
	STUB_ASSERT (!isAligned(size, 4), "Unaligned size");

	ListInit (head->free);
	head->first = (MCB *)ptr;
	head->size = size;

	head->first->size_prev = MCB_LAST;

	if (oldhead == nullptr) {
		head->first->size = (size - sizeof (MCB)) | MCB_LAST;
		StubMemoryEnqueue (head, head->first);
		return;
	}

	// Поглощаем вложенный хип.
	StubMemoryCheck (oldhead);

	// Головной блок сразу же поглощает свободный головной блок старого хипа.
	MCB * const nfirst = head->first;
	MCB *ofirst = oldhead->first;
	STUB_ASSERT (v2laddr(nfirst) + BLOCK_MINSIZE > v2laddr(ofirst), "Region intersection");

	if (!isUsed(ofirst)) {
		ofirst = StubMemoryNextBlock (ofirst);
	}

	nfirst->size = v2laddr(ofirst) - v2laddr(nfirst->link);
	STUB_ASSERT(BlockSize(nfirst) % 4 != 0, "Unaligned nfirst->size");

	ofirst->size_prev = nfirst->size;

	// (Хвостовой блок так же поглощает последний свободный блок старого хипа
	MCB *nlast = (MCB *)(v2laddr(oldhead->first) + oldhead->size);

	MCB *olast = oldhead->first;
	while (!isLast(olast)) {
		olast = StubMemoryNextBlock (olast);
	}

	if (!isUsed(olast)) {
		nlast = olast;
		olast = StubMemoryPreviousBlock (nlast);
	}

	STUB_ASSERT (v2laddr (nlast) + BLOCK_MINSIZE >= v2laddr(head->first) + head->size, "Region intersection");

	nlast->size = (v2laddr(head->first) + head->size - v2laddr (nlast->link)) | MCB_LAST;
	STUB_ASSERT (!isAligned(BlockSize(nlast), 4), "Unaligned nlast->size");

	nlast->size_prev = olast->size;

	// Теперь все свободные поместить в новую очередь.
	while (true) {
		if (!isUsed (nlast)) {
			StubMemoryEnqueue (head, nlast);
		}

		if (isFirst (nlast))
			break;

		nlast = StubMemoryPreviousBlock (nlast);
	}

	// А старый хип надо загарбить...
	#ifdef MEMORY_GARBAGED
	StubMemoryRefuse (oldhead, sizeof (MCBHead));
	#endif

	StubMemoryCheck (head);
}

static
void *StubMemoryAllocInternal (MCBHead * const head, size_t size, int align)
{
	if (size < sizeof (MCBLink)) {
		size = sizeof (MCBLink);
	}

	size = round_up(size, 4);

	for (MCB *block = ListItemFirst(head->free); block != nullptr;
		block = ListItemNext(block, link[0]))
	{
		STUB_ASSERT (isUsed(block), "Used block");

		if (BlockSize(block) < size)
			continue;

		if (align > 4 && !isAligned(v2laddr(block->link), align)) {
			MCB *aligned_block =
				StubMemoryDequeueAlign (head, block, size, align);
			if (aligned_block == nullptr)
				continue;

			block = aligned_block;

			STUB_ASSERT (isUsed(block), "Used block");
			STUB_ASSERT (size >= (block->size & ~MCB_LAST), "Small block");
		} else {
			// Блок уже выровнен или не требует выравнивания.
			StubMemoryDequeue (head, block);
		}

		StubMemorySplitBlock (head, block, size);

		block->size |= MCB_USED;

		MCB *next = StubMemoryNextBlock (block);
		if (next != nullptr) {
			next->size_prev = block->size;
		}

		return block->link;
	}

	return nullptr;
}

static
void StubMemoryFreeInternal (MCBHead * const head, void * const ptr)
{
	MCB *block = (MCB *)((char *)ptr - sizeof (MCB));
	block->size &= ~MCB_USED;

#ifdef MEMORY_PROTECTOR
	STUB_ASSERT (block->prot1 != block->prot2, "Protection corrupted");
#endif

	block = StubMemoryDefrag (head, block);

	// TODO: Освободить страницы...

	StubMemoryEnqueue (head, block);
}


#ifndef TEST
static MCBHead *current = nullptr;

static MCBHead mainheap;
static MCBHead temporary __initdata__;

static lock_t memory_lock = 0;

void __init__ StubMemoryInitBoot (void * const ptr, const size_t size)
{
	StubLock (&memory_lock);

	StubMemoryInit (&temporary, ptr, size, nullptr);
	current = &temporary;

#ifdef MEMORY_GARBAGED
	// Память пока вообще не замаплена, так что можно всю гарбить.
	MCB * const b = current->first;
	StubMemoryRefuse (&(b->link[1]), BlockSize (b) - sizeof (MCBLink));
#endif
	StubUnlock (&memory_lock);
}

void __init__ StubMemoryInitWork (void * const ptr, const size_t size)
{
	StubLock (&memory_lock);
	StubMemoryInit (&mainheap, ptr, size, &temporary);
	current = &mainheap;
	StubUnlock (&memory_lock);
}

void *StubMemoryAlloc (const size_t size)
{
	StubLock (&memory_lock);
	void *ptr = StubMemoryAllocInternal (current, size, 0);
	StubUnlock (&memory_lock);
	return ptr;
}

void *StubMemoryAllocAligned (const size_t size, const int align)
{
	StubLock (&memory_lock);
	void *ptr = StubMemoryAllocInternal (current, size, align);
	StubUnlock (&memory_lock);
	return ptr;
}

void StubMemoryFree (void * const ptr)
{
	StubLock (&memory_lock);

#ifdef MEMORY_GARBAGED
	// Освобождаемая память должна быть замаплена...
	MCB *block = (MCB *)((char *)ptr - sizeof (MCB));
	StubMemoryRefuse (block->link, BlockSize (block));
#endif

	StubMemoryFreeInternal (current, ptr);
	StubUnlock (&memory_lock);
}

void StubCalcHeapUsage(struct KernelInfoMemory *info)
{
	info->KernelHeapTotal = 0;
	info->KernelHeapUsed = 0;

	StubLock (&memory_lock);

	for (MCB *b = current->first; b != 0; b = StubMemoryNextBlock(b)) {
		info->KernelHeapTotal += sizeof(MCB) + BlockSize(b);
		info->KernelHeapUsed += sizeof(MCB);
		if (isUsed(b)) info->KernelHeapUsed += BlockSize(b);
	}

	StubUnlock (&memory_lock);
}


sizex_t StubMemoryReserve (void)
{
	struct KernelInfoMemory minfo;
	StubCalcHeapUsage(&minfo);
	return minfo.KernelHeapTotal - minfo.KernelHeapUsed;
}
#endif
