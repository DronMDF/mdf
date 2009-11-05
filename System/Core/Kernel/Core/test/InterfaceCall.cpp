//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../Kernel.h"
#include "../Core.h"
#include "../Call.h"

#include "testResource.h"
#include "testScheduler.h"
#include "testProcess.h"
#include "testThread.h"

using namespace std;
using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInterfaceCall)

// Таким образом поступает ядро при создании новых процессов из модулей.
BOOST_AUTO_TEST_CASE(testCallProcessAsyncWithoutCaller)
{
	testScheduler scheduler;
	
	const laddr_t entry = 6666;
	testProcess process(entry);
	
	BOOST_REQUIRE_EQUAL(CoreCall(0, process.getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	ResourceThread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);

	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

// Приложения обязаны указывать Task, и могут обращаться только к инстанциям
// процесса или к глобальным инстанциям (TODO)
BOOST_AUTO_TEST_CASE(testCallThreadAsyncByProcessInstance)
{
	testScheduler scheduler;
	
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(thread),
			thread->getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	ResourceThread *st = scheduler.getThread();
	BOOST_REQUIRE_EQUAL(st, thread);
}

BOOST_AUTO_TEST_CASE(testCallCallAsyncByProcessInstance)
{
	testScheduler scheduler;

	const laddr_t entry = 6666;

	testProcess process;
	Resource *call = ResourceCall::Create(&process, &entry, sizeof(laddr_t));
	call->Register();
	process.Attach(call, RESOURCE_ACCESS_CALL, 0);

	testThread task(&process);
	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(&task),
			call->getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	ResourceThread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_CASE(testCallProcessAsyncByProcessInstance)
{
	// TODO
}

// Два следующих теста немного неправдоподобны.
// Клиенты так делать не могут, а ядру это не за чем.

BOOST_AUTO_TEST_CASE(testCallThreadAsyncWithoutCaller)
{
	testThread thread;
	BOOST_REQUIRE_EQUAL(CoreCall(0, thread.getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	testScheduler scheduler;
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread);
}

BOOST_AUTO_TEST_CASE(testCallCallAsyncWithoutCaller)
{
	testScheduler scheduler;
	
	testProcess process;
	const laddr_t entry = 6666;
	Resource *call = ResourceCall::Create(&process, &entry, sizeof(laddr_t));
	call->Register();

	BOOST_REQUIRE_EQUAL(CoreCall(0, call->getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	ResourceThread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_CASE(testInvalidId)
{
	const id_t invalid_id = 0xDEAD001D;
	BOOST_REQUIRE_EQUAL(CoreCall(0, invalid_id, 0, 0, 0), ERROR_INVALIDID);
}

// Проверить неколлируемый ресурс
BOOST_AUTO_TEST_CASE(testCallUncallable)
{
	testResource uncallable;
	uncallable.Register();
	BOOST_REQUIRE_EQUAL(CoreCall(0, uncallable.getId(), 0, 0, RESOURCE_CALL_ASYNC), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_SUITE_END()
