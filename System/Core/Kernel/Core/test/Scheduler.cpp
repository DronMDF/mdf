//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
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

class testSubScheduler : public SubScheduler {
private:
	testSubScheduler(const testSubScheduler &);
	testSubScheduler &operator=(const testSubScheduler &);

public:
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

struct sched_fixture {
private:
	sched_fixture(const sched_fixture&);
	sched_fixture &operator = (const sched_fixture&);
public:
	testScheduler sched;
	testThread thread;

	testSubScheduler *active;
	testSubScheduler *inactive;
	testSubScheduler *killed;

	sched_fixture() : sched(), thread(), active(new testSubScheduler),
		inactive(new testSubScheduler), killed(new testSubScheduler)
	{
		sched.m_actives = active;
		sched.m_inactives = inactive;
		sched.m_killed = killed;
	}

	virtual ~sched_fixture() {};
};

BOOST_FIXTURE_TEST_CASE(add_active, sched_fixture)
{
	sched.addActiveThread(&thread);
	BOOST_REQUIRE_EQUAL(active->thread, &thread);
	BOOST_REQUIRE(active->add_visited);
}

BOOST_FIXTURE_TEST_CASE(add_inactive, sched_fixture)
{
	sched.addInactiveThread(&thread);
	BOOST_REQUIRE_EQUAL(inactive->thread, &thread);
	BOOST_REQUIRE(inactive->add_visited);
}

BOOST_FIXTURE_TEST_CASE(add_killed, sched_fixture)
{
	sched.addKillThread(&thread);
	BOOST_REQUIRE_EQUAL(killed->thread, &thread);
	BOOST_REQUIRE(killed->add_visited);
}

BOOST_FIXTURE_TEST_CASE(get_empty, sched_fixture)
{
	BOOST_REQUIRE(sched.getThread() == 0);
	BOOST_REQUIRE(killed->get_visited);
	BOOST_REQUIRE(inactive->get_visited);
	BOOST_REQUIRE(active->get_visited);
}

BOOST_FIXTURE_TEST_CASE(get_full, sched_fixture)
{
	sched.addInactiveThread(&thread);
	BOOST_REQUIRE_EQUAL(sched.getThread(), &thread);
	BOOST_REQUIRE(inactive->thread == 0);
	BOOST_REQUIRE(active->add_visited);
	BOOST_REQUIRE(active->thread == 0);
}

BOOST_AUTO_TEST_SUITE_END()
