//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/List.h"
#include "../include/Link.h"
#include "../include/Memory.h"
#include "../include/Resource.h"
#include "../include/Thread.h"
#include "../include/SubScheduler.h"
#include "../include/SchedulerInactive.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(inactive_scheduler)

class testThread : public ResourceThread {
public:
	testThread(timeout_t ts) : ResourceThread() {
		setTimestamp(StubGetCurrentClock() - ts);
		Sleep(ts);
	}
};

class testSchedulerInactive : public SchedulerInactive {
public:
	using SchedulerInactive::checkThreadUrgency;

	void Remove(ResourceThread *thread) {
		BOOST_REQUIRE(thread->ScheduleLink.isLinked(&m_infinity));
		m_infinity.Remove(thread);
	}
};

BOOST_AUTO_TEST_CASE(thread_compare_function)
{
	testSchedulerInactive scheduler;

	testThread thread1(5);
	testThread thread2(10);
	BOOST_REQUIRE(scheduler.checkThreadUrgency(&thread1, &thread2));

	thread1.Sleep(15);
	BOOST_REQUIRE(!scheduler.checkThreadUrgency(&thread1, &thread2));

	// При равенстве первый считается более приоритетным
	// Ибо извлекаться из очереди они будут одновременно
	thread1.Sleep(10);
	BOOST_REQUIRE(scheduler.checkThreadUrgency(&thread1, &thread2));
}

BOOST_AUTO_TEST_CASE(order)
{
	testThread thread1(5);
	testThread thread2(10);
	testSchedulerInactive scheduler;

	// Пустой планировщик естественно ничего не возвращает.
	BOOST_REQUIRE(scheduler.getThread() == 0);

	scheduler.addThread(&thread1);
	// Должен залинковаться в список.
	BOOST_REQUIRE(thread1.ScheduleLink.isLinked());

	scheduler.addThread(&thread2);
	BOOST_REQUIRE(thread2.ScheduleLink.isLinked());

	TestIncrementCurrentClock(15);

	// Первым возвращается самый ранний.
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread1);
	BOOST_REQUIRE(!thread1.ScheduleLink.isLinked());
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread2);
	BOOST_REQUIRE(!thread2.ScheduleLink.isLinked());

	// Не зависимо от порядка запихивания
	scheduler.addThread(&thread2);
	scheduler.addThread(&thread1);
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread1);
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread2);

	// Нити из будующего - ждут.
	thread1.Sleep(5);
	scheduler.addThread(&thread1);
	BOOST_REQUIRE(scheduler.getThread() == 0);

	TestIncrementCurrentClock(10);
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread1);

	// Но если время ожидания бесконечно - не возвращается вообще.
	thread1.Sleep(TIMEOUT_INFINITY);
	scheduler.addThread(&thread1);
	BOOST_REQUIRE(scheduler.getThread() == 0);

	thread1.ScheduleLink.Unlink(&thread1);
}

BOOST_AUTO_TEST_SUITE_END()
