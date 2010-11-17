//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/List.h"
#include "../include/Memory.h"
#include "../include/Resource.h"
#include "../include/Thread.h"
#include "../include/SubScheduler.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(sub_scheduler)

class testSubScheduler: public SubScheduler {
private:
	ThreadList m_queue;

public:
	testSubScheduler() : SubScheduler(), m_queue(&Thread::ScheduleLink) {}
	virtual ~testSubScheduler() {}

	virtual void addThread(Thread *thread)
	{
		addThreadOrdered(thread, &m_queue);
	}

	virtual Thread *getThread()
	{
		Thread *thread = m_queue.getFirst();
		if (thread != 0) m_queue.Remove(thread);
		return thread;
	}
};

BOOST_AUTO_TEST_CASE(lifo)
{
	class testLIFOScheduler : public testSubScheduler {
	private:
		virtual bool checkThreadUrgency(const Thread *,
			const Thread *) const
		{
			return true;
		}
	} scheduler;

	class testThread : public Thread {} thread1, thread2;

	scheduler.addThread(&thread1);
	scheduler.addThread(&thread2);

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<Thread *>(&thread2));
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<Thread *>(&thread1));
	BOOST_REQUIRE(scheduler.getThread() == 0);
}

BOOST_AUTO_TEST_CASE(fifo)
{
	class testFIFOScheduler : public testSubScheduler {
	private:
		virtual bool checkThreadUrgency(const Thread *,
			const Thread *) const
		{
			return false;
		}
	} scheduler;

	class testThread : public Thread {} thread1, thread2;

	scheduler.addThread(&thread1);
	scheduler.addThread(&thread2);

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<Thread *>(&thread1));
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<Thread *>(&thread2));
	BOOST_REQUIRE(scheduler.getThread() == 0);
}

BOOST_AUTO_TEST_SUITE_END()
