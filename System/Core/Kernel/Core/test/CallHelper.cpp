//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Kernel.h"
#include "../CallHelper.h"

#include "testThread.h"
#include "testProcess.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCallHelper)

BOOST_AUTO_TEST_CASE(testCreateCalledProcessInKernelMode)
{
	struct testCallHelper : public CallHelper {
		testCallHelper() : CallHelper(0, 0, 0, 0, 0) {}
		using CallHelper::createCalledThread;
	} helper;
	
	// Тестировано стоит через процесс, потому что это единственно верный 
	// вариант в режиме ядра
	const laddr_t entry = 6666;
	testProcess process(entry);
	ResourceThread *thread = helper.createCalledThread(0, process.getId());
	
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_CASE(testCreateCalledThreadInUserMode)
{
	struct testCallHelper : public CallHelper {
		testCallHelper() : CallHelper(0, 0, 0, 0, 0) {}
		using CallHelper::createCalledThread;
	} helper;
	
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	const Task *task = reinterpret_cast<Task *>(thread);
	BOOST_REQUIRE_EQUAL(helper.createCalledThread(task, thread->getId()), thread);
}

BOOST_AUTO_TEST_SUITE_END()
