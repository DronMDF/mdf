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
#include "../SubScheduler.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(sub_scheduler)

class testSubScheduler: public SubScheduler {
private:
	ThreadList m_queue;

public:
	testSubScheduler() : SubScheduler(), m_queue(&ResourceThread::ScheduleLink) {}
	virtual ~testSubScheduler() {}

	virtual void addThread(ResourceThread *thread)
	{
		addThreadOrdered(thread, &m_queue);
	}

	virtual ResourceThread *getThread()
	{
		ResourceThread *thread = m_queue.getFirst();
		if (thread != 0) m_queue.Remove(thread);
		return thread;
	}
};

BOOST_AUTO_TEST_CASE(lifo)
{
	class testLIFOScheduler : public testSubScheduler {
	private:
		virtual bool checkThreadUrgency(const ResourceThread *,
			const ResourceThread *) const
		{
			return true;
		}
	} scheduler;

	class testThread : public ResourceThread {} thread1, thread2;

	scheduler.addThread(&thread1);
	scheduler.addThread(&thread2);

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<ResourceThread *>(&thread2));
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<ResourceThread *>(&thread1));
	BOOST_REQUIRE(scheduler.getThread() == 0);
}

BOOST_AUTO_TEST_CASE(fifo)
{
	class testFIFOScheduler : public testSubScheduler {
	private:
		virtual bool checkThreadUrgency(const ResourceThread *,
			const ResourceThread *) const
		{
			return false;
		}
	} scheduler;

	class testThread : public ResourceThread {} thread1, thread2;

	scheduler.addThread(&thread1);
	scheduler.addThread(&thread2);

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<ResourceThread *>(&thread1));
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), static_cast<ResourceThread *>(&thread2));
	BOOST_REQUIRE(scheduler.getThread() == 0);
}

BOOST_AUTO_TEST_SUITE_END();
