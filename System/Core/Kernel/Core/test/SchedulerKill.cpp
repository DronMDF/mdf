//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/List.h"
#include "../include/Memory.h"
#include "../include/Resource.h"
#include "../include/Process.h"
#include "../include/ResourceThread.h"
#include "../include/SubScheduler.h"
#include "../include/SchedulerKill.h"

#include "testProcess.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(kill_scheduler)

BOOST_AUTO_TEST_CASE(enqueue)
{
	SchedulerKill scheduler;
	class testThread : public ResourceThread {
	public:
		bool visited;
		testThread() : ResourceThread(), visited(false) {}

		virtual bool Deactivate() {
			visited = true;
			return false;
		}
	} thread;

	scheduler.addThread(&thread);
	BOOST_REQUIRE(thread.ScheduleLink.isLinked());

	// Шедулер не возвращает нитей, вместо этого он вызывает для каждой
	// имеющейся нити метод Detach? Deactivate? и если тот возвращает true,
	// то нить уничтожается.
	BOOST_REQUIRE(scheduler.getThread() == 0);
	BOOST_REQUIRE(thread.visited);

	thread.ScheduleLink.Unlink(&thread);
}

BOOST_AUTO_TEST_CASE(destroy)
{
	bool deleted = false;

	class testScheduler : public SchedulerKill {
	public:
		using SchedulerKill::m_queue;
	} scheduler;

	class _testProcess: public testProcess {
		virtual int Detach(Resource *resource) {
			delete resource;
			return SUCCESS;
		};
	} process;

	class testThread : public ResourceThread {
	private:
		bool *m_flag;
		testThread(const testThread &);
		testThread &operator = (const testThread &);
	public:
		testThread(ResourceProcess *process, bool *flag)
			: ResourceThread(process), m_flag(flag) {}
		virtual ~testThread() { *m_flag = true; }
		virtual bool Deactivate() { return true; }
	} *thread = new testThread(&process, &deleted);

	scheduler.addThread(thread);
	BOOST_REQUIRE(scheduler.getThread() == 0);
	BOOST_REQUIRE(deleted);
	BOOST_REQUIRE(scheduler.m_queue.getFirst() == 0);
}

BOOST_AUTO_TEST_SUITE_END()
