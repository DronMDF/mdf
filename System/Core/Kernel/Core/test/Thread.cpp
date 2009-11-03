//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <limits.h>
#include <iostream>
#include <iomanip>
#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../List.h"
#include "../Memory.h"
#include "../Resource.h"
#include "../ResourceInstance.h"
#include "../Thread.h"
#include "../Process.h"
#include "../Core.h"
#include "../Kernel.h"

#include "testThread.h"
#include "testProcess.h"
#include "testScheduler.h"

using namespace std;
using namespace Core;

BOOST_AUTO_TEST_SUITE(thread)

struct create_fixture {
	testProcess process;
	testThread current_thread;

	id_t id;
	const Task *task;

	create_fixture()
		: process(),
		  current_thread(&process),
		  id(INVALID_ID),
		  task(reinterpret_cast<Task *>(&current_thread))
	{
		struct KernelCreateThreadParam param = { 0 };
		const int rv = CoreCreate(task, RESOURCE_TYPE_THREAD,
					  &param, sizeof(param), &id);
		BOOST_REQUIRE_EQUAL(rv, SUCCESS);
	}

	virtual ~create_fixture() {};
};

BOOST_FIXTURE_TEST_CASE(create, create_fixture)
{
	ResourceInstance *instance = process.FindInstance(id);
	BOOST_REQUIRE(instance != 0);

	Resource *resource = instance->getResource();
	BOOST_REQUIRE(resource != 0);
	BOOST_REQUIRE_EQUAL(resource->getId(), id);

	ResourceThread *thread = resource->asThread();
	BOOST_REQUIRE(thread != 0);
}

BOOST_FIXTURE_TEST_CASE(call, create_fixture)
{
	BOOST_REQUIRE_EQUAL(current_thread.Call(), &current_thread);

	// Этот метод проходит, но после того происходит попытка удаления непустого списка.
	// Видимо новая нить подвисает?

	uint32_t param = 0xDEADC0DE;
	//BOOST_WARN(CoreCall(task, id, &param, sizeof(uint32_t),
	//	RESOURCE_CALL_ASYNC | RESOURCE_CALL_COPY) == SUCCESS);

	// TODO: теперь надо взять нить, через пейджфолт достать из нее страницу
	// TXA, и сравнить что там есть.

	// Нить можно взять из планировщика.
	// она должна соответствовать новой нити фикстуры.
}

BOOST_AUTO_TEST_CASE(timestamp)
{
	testThread thread;

	thread.setTimestamp(10);
	BOOST_REQUIRE_EQUAL(thread.getTimestamp(), 10);

	thread.Sleep(5);
	// Таймстамп не должен меняться - таймстамп показывает время последнего вызова.
	BOOST_REQUIRE_EQUAL(thread.getTimestamp(), 10);
	BOOST_REQUIRE_GT(thread.getWakeupstamp(), StubGetCurrentClock());
	BOOST_REQUIRE_NE(thread.getWakeupstamp(), CLOCK_MAX);

	thread.Sleep(CLOCK_MAX);
	BOOST_REQUIRE_EQUAL(thread.getWakeupstamp(), CLOCK_MAX);
}

BOOST_AUTO_TEST_CASE(info_current)
{
	testThread thread;
	id_t id = INVALID_ID;
	size_t id_size = sizeof(id_t);

	const int rv = thread.Info(RESOURCE_INFO_THREAD_CURRENT, &id, &id_size);
	BOOST_REQUIRE_EQUAL(rv, SUCCESS);
	BOOST_REQUIRE_EQUAL(id_size, sizeof(id_t));
	BOOST_REQUIRE_EQUAL(id, thread.getId());

	id = INVALID_ID;
	const int rv2 = CoreInfo(reinterpret_cast<Task *>(&thread),
		0, RESOURCE_INFO_THREAD_CURRENT, &id, &id_size);
	BOOST_REQUIRE_EQUAL(rv2, SUCCESS);
	BOOST_REQUIRE_EQUAL(id_size, sizeof(id_t));
	BOOST_REQUIRE_EQUAL(id, thread.getId());
}

BOOST_AUTO_TEST_CASE(modify_priority)
{
	testThread thread;
	const uint32_t priority = 666;

	const int rv = thread.Modify(RESOURCE_MODIFY_THREAD_PRIORITY,
				     &priority, sizeof(uint32_t));
	BOOST_REQUIRE_EQUAL(rv, SUCCESS);
	BOOST_REQUIRE_EQUAL(thread.getPriority(), priority);

	// TODO: приоритетный класс можно передать вторым словом...

	// Провести эту проверку через CoreModify - сложновато,
	// надо процесс с текущей нитью и инстанцией.
	// В принципе здесь ничего экстраординарного.
}

BOOST_AUTO_TEST_CASE(kill)
{
	// Для начала проверим что Kill вызывается.
	struct inThread : public testThread {
		virtual void Kill() { throw runtime_error("killed"); }
	} thread;

	BOOST_REQUIRE_THROW(thread.PageFault(USER_MEMORY_BASE + RETMAGIC, 0),
		runtime_error);

	// По команде Kill процесс должен:
	// 1. вернуть TPC буфер в вызывающий процесс (если надо)
	// 2. Встать в очередь на уничтожение
	// 3. Вызвать очередную нить, или Halt.
	// И как это можно протестировать? непонятно.

}

BOOST_AUTO_TEST_CASE(deactivate)
{
	struct inThread : public testThread {
		using ResourceThread::m_task;
	} thread;

	// При вызове метода Deactivate должен произойти вызов StubTaskDestroy,
	// И если тот вернул true, то поле m_task необходимо занулить.
	TestSetStubTaskDestroyReaction(1);
	thread.m_task = reinterpret_cast<Task *>(1);
	BOOST_REQUIRE(thread.Deactivate());
	BOOST_REQUIRE(thread.m_task == 0);

	TestSetStubTaskDestroyReaction(0);
	thread.m_task = reinterpret_cast<Task *>(1);
	BOOST_REQUIRE(!thread.Deactivate());
	BOOST_REQUIRE(thread.m_task != 0);
}

BOOST_AUTO_TEST_CASE(activate)
{
	testThread thread;
	testScheduler scheduler;

	BOOST_REQUIRE(scheduler.m_actives == 0);
	BOOST_REQUIRE(scheduler.m_inactives == 0);
	BOOST_REQUIRE(scheduler.m_killed == 0);
	
	thread.Sleep(CLOCK_MAX);
	scheduler.addInactiveThread(&thread);

	thread.Activate();

	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread);
}

BOOST_AUTO_TEST_SUITE_END()
