//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../List.h"
#include "../Link.h"
#include "../Memory.h"
#include "../Resource.h"
#include "../Thread.h"
#include "../SubScheduler.h"
#include "../InactiveScheduler.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(inactive_scheduler)

class testThread : public ResourceThread {
public:
	testThread(clock_t ts) : ResourceThread() {
		setTimestamp(StubGetCurrentClock() - ts);
		Sleep(ts);
	}
};

class testInactiveScheduler : public InactiveScheduler {
public:
	using InactiveScheduler::checkThreadUrgency;

	void Remove(ResourceThread *thread) {
		BOOST_REQUIRE(thread->ScheduleLink.isLinked(&m_infinity));
		m_infinity.Remove(thread);
	}
};

BOOST_AUTO_TEST_CASE(thread_compare_function)
{
	testInactiveScheduler scheduler;

	testThread thread1(5);
	testThread thread2(10);
	BOOST_CHECK(scheduler.checkThreadUrgency(&thread1, &thread2));

	thread1.Sleep(15);
	BOOST_CHECK(!scheduler.checkThreadUrgency(&thread1, &thread2));

	// При равенстве первый считается более приоритетным
	// Ибо извлекаться из очереди они будут одновременно
	thread1.Sleep(10);
	BOOST_CHECK(scheduler.checkThreadUrgency(&thread1, &thread2));
}

BOOST_AUTO_TEST_CASE(order)
{
	testThread thread1(5);
	testThread thread2(10);
	testInactiveScheduler scheduler;

	// Пустой планировщик естественно ничего не возвращает.
	BOOST_CHECK(scheduler.getThread() == 0);

	scheduler.addThread(&thread1);
	// Должен залинковаться в список.
	BOOST_CHECK(thread1.ScheduleLink.isLinked());

	scheduler.addThread(&thread2);
	BOOST_CHECK(thread2.ScheduleLink.isLinked());

	TestIncrementCurrentClock(15);

	// Первым возвращается самый ранний.
	BOOST_CHECK(scheduler.getThread() == &thread1);
	BOOST_CHECK(!thread1.ScheduleLink.isLinked());
	BOOST_CHECK(scheduler.getThread() == &thread2);
	BOOST_CHECK(!thread2.ScheduleLink.isLinked());

	// Не зависимо от порядка запихивания
	scheduler.addThread(&thread2);
	scheduler.addThread(&thread1);
	BOOST_CHECK(scheduler.getThread() == &thread1);
	BOOST_CHECK(scheduler.getThread() == &thread2);

	// Нити из будующего - ждут.
	thread1.Sleep(5);
	scheduler.addThread(&thread1);
	BOOST_CHECK(scheduler.getThread() == 0);

	TestIncrementCurrentClock(10);
	BOOST_CHECK(scheduler.getThread() == &thread1);

	// Но если время ожидания бесконечно - не возвращается вообще.
	thread1.Sleep(CLOCK_MAX);
	scheduler.addThread(&thread1);
	BOOST_CHECK(scheduler.getThread() == 0);

	thread1.ScheduleLink.Unlink(&thread1);
}

BOOST_AUTO_TEST_SUITE_END()
