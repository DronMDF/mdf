//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

// TODO: Надо переделывать на юниттесты

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#define TEST
#define __init__

typedef unsigned long laddr_t;

#define v2laddr(ptr) ((laddr_t)ptr)
#define	round(v, d)	((v) - (v) % (d))
#define	round_up(v, d)	round((v) + (d) - 1, d)

#define CorePrint(...) printf (__VA_ARGS__)
#define CoreRandom()	0xfeedbade
#define memgarbage(p, s)	memset(p, 0xaa, s)

#define STUB_ASSERT(exp, msg)	\
while (exp) {			\
	printf ("STUB_ASSERT '%s' at %s:%u\n", msg, __FILE__, __LINE__); \
	assert (exp);		\
}

#define STATIC_ASSERT(exp)	extern char __static_assert[(exp) ? 1 : -1]

#define isSet(v,f)	(((v) & (f)) != 0)
#define isAligned(v,a)	((v) % (a) == 0)

#include "../Memory.c"

static unsigned char heap[256];
static MCBHead old;

#define TESTSZ	(sizeof (MCBLink) * 2)

#define TESTEX(expr)							\
	do { if (!(expr)) {						\
		fprintf (stderr, "%s:%u error: Test failed in function %s.\n",	\
			__FILE__, __LINE__, __FUNCTION__);		\
		exit (EXIT_FAILURE);					\
	} } while (0)

static
bool CheckBlock (const MCB * const block, size_t psize, size_t nsize,
	const MCB * const pblock, const MCB * const nblock, const void * const list)
{
	if (block->size_prev == psize &&
	    block->size == nsize &&
	    ListItemPrev(block, link[0]) == pblock &&
	    ListItemNext(block, link[0]) == nblock &&
	    ListItemList(block, link[0]) == list)
	{
		return true;
	}

	return false;
}

static
void MemoryEnqueueCause1 ()
{
	memset (heap, 0xff, sizeof (heap));

	MCB * const blk0 = (MCB *)heap;

	blk0->size_prev = MCB_LAST;
	blk0->size = TESTSZ;

	ListInit (old.free);
	StubMemoryEnqueue (&old, blk0);

	// Проверка результата.
	const MCB * const blk0l = ListItemFirst (old.free);

	TESTEX (blk0 == blk0l &&
		CheckBlock (blk0, MCB_LAST, TESTSZ, NULL, NULL, &(old.free)));
}

void MemoryEnqueueCause2 ()
{
	MCB * const blk1 = (MCB *)(heap + sizeof (MCB) + TESTSZ);

	blk1->size_prev = TESTSZ;
	blk1->size = TESTSZ | MCB_LAST;

	StubMemoryEnqueue (&old, blk1);

	const MCB * const blk0 = (MCB *)heap;

	TESTEX (blk1 == ListItemFirst (old.free) &&
		CheckBlock (blk1, TESTSZ, TESTSZ | MCB_LAST, NULL, blk0, &(old.free)) &&
		blk0 == ListItemNext(blk1, link[0]) &&
		CheckBlock (blk0, MCB_LAST, TESTSZ, blk1, NULL, &(old.free)));
}

void MemoryDequeueCause1 ()
{
	MCB * const blk0 = (MCB *)heap;
	StubMemoryDequeue (&old, blk0);

	const MCB * const blk1l = ListItemFirst (old.free);
	TESTEX (CheckBlock (blk0, MCB_LAST, TESTSZ, blk1l, NULL, NULL) &&
		CheckBlock (blk1l, TESTSZ, TESTSZ | MCB_LAST, NULL, NULL, &(old.free)));
}

void MemoryNextCause1()
{
	const MCB * const blk0 = (MCB *)heap;
	const MCB * const blk1 = (MCB *)(heap + sizeof (MCB) + TESTSZ);

	TESTEX (StubMemoryNextBlock (blk0) == blk1);
}

void MemoryPrevCause1()
{
	const MCB * const blk0 = (MCB *)heap;
	const MCB * const blk1 = (MCB *)(heap + sizeof (MCB) + TESTSZ);

	TESTEX (StubMemoryPreviousBlock (blk1) == blk0);
}

void MemoryPrevCause2()
{
	const MCB * const blk0 = (MCB *)heap;
	TESTEX (StubMemoryPreviousBlock (blk0) == NULL);
}

void MemoryDefragCause1()
{
	MCB *blk0 = (MCB *)heap;
	TESTEX (StubMemoryDefrag(&old, blk0) == blk0);

	const MCB * const blk1 = (MCB *)(heap + sizeof (MCB) + TESTSZ);
	TESTEX (ListItemFirst (old.free) == NULL &&
		CheckBlock (blk0, MCB_LAST, TESTSZ * 2 + sizeof (MCB) + MCB_LAST,
			blk1, NULL, NULL));
}

void MemorySplitCause1 ()
{
	MCB * const blk0 = (MCB *)heap;

	StubMemorySplitBlock (&old, blk0, sizeof (MCBLink));

	const MCB * const blk1old = (MCB *)(heap + sizeof (MCB) + TESTSZ);
	const MCB * const blk1new = (MCB *)(heap + sizeof (MCB) + sizeof (MCBLink));

	TESTEX (CheckBlock (blk0, MCB_LAST, sizeof (MCBLink), blk1old, NULL, NULL) &&
		ListItemFirst (old.free) == blk1new &&
		CheckBlock (blk1new, sizeof (MCBLink), TESTSZ * 2 - sizeof (MCBLink) + MCB_LAST,
			NULL, NULL, &(old.free)));
}

void MemoryInitCause1()
{
	memset (heap, 0xff, sizeof (heap));

	StubMemoryInit (&old, heap + TESTSZ + sizeof (MCB),
		(TESTSZ + sizeof (MCB)) * 3, NULL);

	const MCB * const blk0 = (MCB *)(heap + TESTSZ + sizeof (MCB));

	TESTEX (ListItemFirst (old.free) == blk0 &&
		CheckBlock (blk0, MCB_LAST,
			TESTSZ * 3 + sizeof (MCB) * 2 + MCB_LAST,
			NULL, NULL, &(old.free)));
}

void MemoryAllocCause1 ()
{
	void *ptr = StubMemoryAllocInternal (&old, TESTSZ, 0);

	MCB * const blk0 = (MCB *)(heap + TESTSZ + sizeof (MCB));
	const MCB * const blk1 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 2);

	memset (blk0->link, 0xaa, TESTSZ);

	TESTEX (ptr == blk0->link &&
		CheckBlock (blk0, MCB_LAST, TESTSZ | MCB_USED,
			(void *)0xaaaaaaaa, (void *)0xaaaaaaaa, (void *)0xaaaaaaaa) &&
		ListItemFirst (old.free) == blk1 &&
		CheckBlock (blk1, TESTSZ | MCB_USED, TESTSZ * 2 + sizeof (MCB) + MCB_LAST,
			NULL, NULL, &(old.free)));
}

void MemoryAllocCause2 ()
{
	void *ptr = StubMemoryAllocInternal (&old, TESTSZ, 0);

	MCB * const blk1 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 2);
	const MCB * const blk2 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 3);

	memset (blk1->link, 0xaa, TESTSZ);

	TESTEX (ptr == blk1->link &&
		CheckBlock (blk1, TESTSZ | MCB_USED, TESTSZ | MCB_USED,
			(void *)0xaaaaaaaa, (void *)0xaaaaaaaa, (void *)0xaaaaaaaa) &&
		ListItemFirst (old.free) == blk2 &&
		CheckBlock (blk2, TESTSZ | MCB_USED, TESTSZ | MCB_LAST, NULL, NULL, &(old.free)));
}

void MemoryFreeCause1 ()
{
	MCB * const blk0 = (MCB *)(heap + TESTSZ + sizeof (MCB));
	const MCB * const blk1 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 2);
	const MCB * const blk2 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 3);

	StubMemoryFreeInternal (&old, blk0->link);

	memset (&(blk0->link[1]), 0xcc, TESTSZ - sizeof (MCBLink));

	TESTEX (ListItemFirst (old.free) == blk0 &&
		CheckBlock (blk0, MCB_LAST, TESTSZ, NULL, blk2, &(old.free)) &&
		CheckBlock (blk1, TESTSZ, TESTSZ | MCB_USED,
			(void *)0xaaaaaaaa, (void *)0xaaaaaaaa, (void *)0xaaaaaaaa) &&
		CheckBlock (blk2, TESTSZ | MCB_USED, TESTSZ | MCB_LAST, blk0, NULL, &(old.free)));
}


void MemoryInitCause2 ()
{
	MCBHead new;

	StubMemoryInit (&new, heap, sizeof (heap), &old);

	const MCB * const blk0 = (MCB *)heap;
	const MCB * const blk1 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 2);
	const MCB * const blk2 = (MCB *)(heap + (TESTSZ + sizeof (MCB)) * 3);

	TESTEX (ListItemFirst (new.free) == blk0 &&
		CheckBlock (blk0, MCB_LAST, TESTSZ * 2 + sizeof (MCB), NULL, blk2, &(new.free)) &&
		CheckBlock (blk1, TESTSZ * 2 + sizeof (MCB), TESTSZ | MCB_USED,
			(void *)0xaaaaaaaa, (void *)0xaaaaaaaa, (void *)0xaaaaaaaa) &&
		CheckBlock (blk2, TESTSZ | MCB_USED, sizeof (heap) - (TESTSZ * 3 + sizeof (MCB) * 4) + MCB_LAST,
			blk0, NULL, &(new.free)));
}

int main(int argc, char **argv)
{
	// Сперва отточить примитивы - Enqueue, Dequeue...
	MemoryEnqueueCause1();
	MemoryEnqueueCause2();

	MemoryDequeueCause1();

	MemoryNextCause1();
	MemoryPrevCause1();
	MemoryPrevCause2();

	MemoryDefragCause1();

	MemorySplitCause1 ();

	// Инициализируем old и немного повозимся с ним
	MemoryInitCause1 ();

	MemoryAllocCause1 ();
	MemoryAllocCause2 ();

	MemoryFreeCause1 ();

	MemoryInitCause2 ();

	return EXIT_SUCCESS;
}
