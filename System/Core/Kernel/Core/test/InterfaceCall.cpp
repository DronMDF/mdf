//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../Kernel.h"
#include "../Core.h"
#include "../Scheduler.h"

#include "testProcess.h"
#include "testThread.h"

using namespace boost;
using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteInterfaceCall)

BOOST_AUTO_TEST_CASE(testInvalidId)
{
	const id_t invalid_id = 0xDEAD001D;
	BOOST_REQUIRE_EQUAL(CoreCall(0, invalid_id, 0, 0, 0), ERROR_INVALIDID);
}

// Этот тест ведет себя нестабильно... не могу понять в чем проблема.
// BOOST_AUTO_TEST_CASE(testCallProcessAsyncWithoutCaller)
// {
// 	const laddr_t entry = 6666;
// 	testProcess process(entry);
// 	
// 	BOOST_REQUIRE_EQUAL(CoreCall(0, process.getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);
// 
// 	ResourceThread *thread = Scheduler().getThread();
// 	BOOST_REQUIRE(thread != 0);
// 
// 	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
// 	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
// }

// BOOST_AUTO_TEST_CASE(testCallThreadAsyncWithoutCaller)
// {
// 	testThread thread;
// 	BOOST_REQUIRE_EQUAL(CoreCall(0, thread.getId(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);
// 	BOOST_REQUIRE(Scheduler().getThread() != &thread);
// }

BOOST_AUTO_TEST_SUITE_END()
