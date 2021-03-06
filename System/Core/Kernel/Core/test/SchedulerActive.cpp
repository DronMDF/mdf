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
#include "../include/SchedulerActive.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteSchedulerActive)

class testThread : public Thread {
public:
	testThread(tick_t timestamp, uint32_t priority = 128)
		: Thread()
	{
		setTimestamp(timestamp);
		setPriority(priority);
	}
};

BOOST_AUTO_TEST_CASE(thread_index)
{
	class testScheduler : public SchedulerActive {
		public: using SchedulerActive::getThreadIndex;
	} scheduler;

	testThread thread(StubGetCurrentClock(), 0);
	BOOST_REQUIRE_EQUAL(scheduler.getThreadIndex(&thread), 0);

	thread.setPriority(1);
	BOOST_REQUIRE_EQUAL(scheduler.getThreadIndex(&thread), 1);

	thread.setTimestamp(StubGetCurrentClock() - 5);
	thread.setPriority(0);
	BOOST_REQUIRE_EQUAL(scheduler.getThreadIndex(&thread), 5);

	thread.setPriority(1);
	BOOST_REQUIRE_EQUAL(scheduler.getThreadIndex(&thread), 6);
}

BOOST_AUTO_TEST_CASE(thread_order_function)
{
	class testScheduler : public SchedulerActive {
		public: using SchedulerActive::checkThreadUrgency;
	} scheduler;

	testThread thread1(StubGetCurrentClock(), 0);
	testThread thread2(StubGetCurrentClock(), 1);
	BOOST_REQUIRE(!scheduler.checkThreadUrgency(&thread1, &thread2));
	BOOST_REQUIRE(scheduler.checkThreadUrgency(&thread2, &thread1));

	// При полном равенстве (индекс и приоритет)
	// входящий (первый) тред считается менее приоритетным.
	thread2.setPriority(0);
	BOOST_REQUIRE(!scheduler.checkThreadUrgency(&thread1, &thread2));

	// При равенстве индексов приоритетный считается важнее.
	thread1.setTimestamp(StubGetCurrentClock() - 5);
	thread2.setPriority(5);
	BOOST_REQUIRE(!scheduler.checkThreadUrgency(&thread1, &thread2));
}

BOOST_AUTO_TEST_CASE(normal_priority)
{
	SchedulerActive scheduler;

	// Пустой планировщик естественно ничего не возвращает.
	BOOST_REQUIRE(scheduler.getThread() == 0);

	testThread thread1(StubGetCurrentClock(), 10);
	scheduler.addThread(&thread1);
	// Должен залинковаться в список.
	BOOST_REQUIRE(thread1.ScheduleLink.isLinked());

	testThread thread2(StubGetCurrentClock(), 5);
	scheduler.addThread(&thread2);
	BOOST_REQUIRE(thread2.ScheduleLink.isLinked());

	// Должен вернуть сперва thread1;
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread1);
	BOOST_REQUIRE(!thread1.ScheduleLink.isLinked());

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread2);
	BOOST_REQUIRE(!thread2.ScheduleLink.isLinked());

	scheduler.addThread(&thread2);
	scheduler.addThread(&thread1);
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread1);
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread2);
}

BOOST_AUTO_TEST_SUITE_END()
