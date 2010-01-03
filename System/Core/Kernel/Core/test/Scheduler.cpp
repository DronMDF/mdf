//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>

#include "Types.h"

#include "../Memory.h"
#include "../Resource.h"
#include "../Thread.h"
#include "../List.h"
#include "../Scheduler.h"
#include "../SubScheduler.h"

#include "testThread.h"
#include "testScheduler.h"

using namespace Core;

// Новая реализация планировщика.
BOOST_AUTO_TEST_SUITE(scheduler_new)

struct testSubScheduler : public SubScheduler {
	ResourceThread *thread;

	bool add_visited;
	bool get_visited;

	testSubScheduler() : thread(0), add_visited(false), get_visited(false)
	{}

	virtual ResourceThread *getThread() {
		get_visited = true;
		ResourceThread *t = thread;
		thread = 0;	// Нити отдаем с концами.
		return t;
	};

	virtual void addThread(ResourceThread *t) {
		thread = t;
		add_visited = true;
	}
};

struct sched_fixture : public testScheduler {
	testThread thread;

	sched_fixture() : thread() {
		m_actives = new testSubScheduler;
		m_inactives = new testSubScheduler;
		m_killed = new testSubScheduler;
	}
	void CUSTOM_REQUIRE_ADD_VISITED(const SubScheduler *ss) const {
		BOOST_REQUIRE(dynamic_cast<const testSubScheduler *>(ss)->add_visited);
	}
	void CUSTOM_REQUIRE_GET_VISITED(const SubScheduler *ss) const {
		BOOST_REQUIRE(dynamic_cast<const testSubScheduler *>(ss)->get_visited);
	}
	void CUSTOM_REQUIRE_THREAD_PRESENT(const SubScheduler *ss) const {
		BOOST_REQUIRE_EQUAL(dynamic_cast<const testSubScheduler *>(ss)->thread, &thread);
	}
	void CUSTOM_REQUIRE_NO_THREAD(const SubScheduler *ss) const {
		BOOST_REQUIRE(dynamic_cast<const testSubScheduler *>(ss)->thread == 0);
	}
};

BOOST_FIXTURE_TEST_CASE(add_active, sched_fixture)
{
	addActiveThread(&thread);
	CUSTOM_REQUIRE_THREAD_PRESENT(m_actives);
	CUSTOM_REQUIRE_ADD_VISITED(m_actives);
}

BOOST_FIXTURE_TEST_CASE(add_inactive, sched_fixture)
{
	addInactiveThread(&thread);
	CUSTOM_REQUIRE_THREAD_PRESENT(m_inactives);
	CUSTOM_REQUIRE_ADD_VISITED(m_inactives);
}

BOOST_FIXTURE_TEST_CASE(add_killed, sched_fixture)
{
	addKillThread(&thread);
	CUSTOM_REQUIRE_THREAD_PRESENT(m_killed);
	CUSTOM_REQUIRE_ADD_VISITED(m_killed);
}

BOOST_FIXTURE_TEST_CASE(get_empty, sched_fixture)
{
	BOOST_REQUIRE(getThread() == 0);
	CUSTOM_REQUIRE_GET_VISITED(m_actives);
	CUSTOM_REQUIRE_GET_VISITED(m_inactives);
	CUSTOM_REQUIRE_GET_VISITED(m_killed);
}

BOOST_FIXTURE_TEST_CASE(get_full, sched_fixture)
{
	addInactiveThread(&thread);
	BOOST_REQUIRE_EQUAL(getThread(), &thread);
	CUSTOM_REQUIRE_NO_THREAD(m_inactives);
	CUSTOM_REQUIRE_ADD_VISITED(m_actives);
	CUSTOM_REQUIRE_NO_THREAD(m_actives);
}

BOOST_AUTO_TEST_SUITE_END()
