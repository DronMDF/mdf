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

BOOST_AUTO_TEST_CASE(testInvalidId)
{
	const id_t invalid_id = 0xDEAD001D;
	BOOST_REQUIRE_EQUAL(CoreCall(0, invalid_id, 0, 0, 0), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_CASE(testCallProcessAsyncWithoutCaller)
{
	const laddr_t entry = 6666;
	testProcess process(entry);
	
	BOOST_REQUIRE_EQUAL(CoreCall(0, process.getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	testScheduler scheduler;
	ResourceThread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);

	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
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

// Проверить неколлируемый ресурс
BOOST_AUTO_TEST_CASE(testCallUncallable)
{
	testResource uncallable;
	uncallable.Register();
	BOOST_REQUIRE_EQUAL(CoreCall(0, uncallable.getId(), 0, 0, RESOURCE_CALL_ASYNC), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_SUITE_END()
