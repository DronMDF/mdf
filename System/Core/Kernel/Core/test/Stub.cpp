//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

// Интерфейс Stub для тестирования

#include <limits.h>
#include <string.h>
#include <stdlib.h>

#include <iostream>
#include <list>

#include <boost/foreach.hpp>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Stub.h"
#include "../Kernel.h"

using namespace std;

struct _PageInfo {
public:
	uint8_t m_page[4096];
	int m_icount;

	_PageInfo() : m_icount(0) { }
};

// Контролируем что тесты не зажрали страниц.
class check {
public:
	check();
	~check();
} _pcheck;

list<PageInfo *> pages;

check::check() {
}

check::~check() {
	if (!pages.empty()) {
		BOOST_FOREACH(const PageInfo *p, pages) {
			cout << "Leave page " << p << endl;
		}
	}

	BOOST_ASSERT(pages.empty());
}

extern "C" {

void StubMemoryCopy (void *dst, const void *src, size_t count)
{
	memcpy (dst, src, count);
}

void StubMemoryClear (void *dst, size_t count)
{
	memset (dst, 0, count);
}

void StubMemoryRefuse (void *dst, size_t count)
{
	memset (dst, 0xfb, count);
}

PageInfo *StubPageAlloc()
{
	PageInfo *page = new PageInfo;
	pages.push_back(page);
// 	cout << "Alloc page " << page << endl;
	return page;
}

void StubPageInstanceDelete(const PageInstance *instance)
{
	PageInfo *page = reinterpret_cast<PageInfo *>(const_cast<PageInstance *>(instance));
	BOOST_ASSERT(page->m_icount > 0);
	page->m_icount--;
	if (page->m_icount > 0) return;

	// Освободить страницу.
	for (list<PageInfo *>::iterator it = pages.begin(); it != pages.end(); ++it)
	{
		if (*it == page) {
//			cout << "Free page " << page << endl;
			delete page;
			pages.erase(it);
			break;
		}
	}
}

PageInfo *StubGetPageByPAddr (paddr_t)
{
	return 0;
}

PageInfo *StubGetFreePageByPaddr (paddr_t)
{
	return 0;
}

const PageInstance *StubGetPageInstance (PageInfo *page, uint32_t, const void *)
{
	BOOST_FOREACH(const PageInfo *p, pages) {
		if (p != page) continue;
		page->m_icount++;
		return reinterpret_cast<PageInstance *>(page);
	}

	BOOST_FAIL("Invalid page");
	return 0;
}

PageInfo *StubGetPageByInstance (const PageInstance *instance)
{
	const PageInfo *page = reinterpret_cast<const PageInfo *>(instance);
	BOOST_FOREACH(const PageInfo *p, pages) {
		if (p != page) continue;
		return const_cast<PageInfo *>(page);
	}

	BOOST_FAIL("Invalid instance");
	return 0;
}

laddr_t StubPageTemporary (PageInfo *page)
{
	BOOST_FOREACH(const PageInfo *p, pages) {
		if (p != page) continue;
		return reinterpret_cast<laddr_t>(page->m_page);
	}

	BOOST_FAIL("Invalid page");
	return 0;
}

void StubPageUntemporary (PageInfo *page)
{
	BOOST_FOREACH(const PageInfo *p, pages) {
		if (p == page) return;
	}

	BOOST_FAIL("Invalid page");
}

void StubPageUntemporaryByLAddr(laddr_t addr)
{
	BOOST_FOREACH(const PageInfo *page, pages) {
		if (reinterpret_cast<uint8_t *>(addr) == page->m_page)
			return;
	}

	BOOST_FAIL("Invalid addr");
}

Task *StubTaskCreate (const laddr_t, void *thread)
{
	return static_cast<Task *>(thread);
}

static int task_destroy_reaction = 0;
bool StubTaskDestroy (Task *)
{
	return task_destroy_reaction;
}

void StubTaskRun (Task *)
{
}

void *StubTaskGetThread (const Task *task)
{
	return const_cast<Task *>(task);
}

void StubSetStackFrame (struct StubStackFrame *frame, id_t caller,
	offset_t txa_offset, size_t txa_size, int flags)
{
	frame->flags = flags;
	frame->txa_size = txa_size;
	frame->txa_ptr = (frame->txa_size != 0) ? (USER_MEMORY_BASE + USER_TXA_BASE + txa_offset) : 0;
	frame->caller = caller,
	frame->retmagic = RETMAGIC;
}
void StubPrintChar (const int c)
{
	cout << static_cast<char>(c);
}

int StubGetChar ()
{
	char ch;
	cin >> ch;
	return ch;
}

// Таймер
clock_t StubGetTimestampCounter ()
{
	return 0;
}

void StubLock (lock_t * const)
{
}

void StubUnlock (lock_t * const)
{
}

static clock_t counter = 100;

clock_t StubGetCurrentClock ()
{
	return counter;
}

// TODO: переместить в Core навсегда.
int StubInfoValue(void *info, size_t *info_size, const void *data, size_t data_size)
{
	if (info_size == NULL)
		return ERROR_INVALIDPARAM;

	if (info != NULL)
		StubMemoryCopy(info, data, min(*info_size, data_size));

	*info_size = data_size;
	return SUCCESS;
}

void StubCPUIdle()
{
}

} // extern "C"

// Вспомогательно-тестовые методы
void TestIncrementCurrentClock(clock_t increment)
{
	counter += increment;
}

void TestSetStubTaskDestroyReaction(int react)
{
	task_destroy_reaction = react;
}
