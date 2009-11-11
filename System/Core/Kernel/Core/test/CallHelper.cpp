//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../CallHelper.h"
#include "../Thread.h"

#include "testProcess.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCallHelper)

BOOST_AUTO_TEST_CASE(testFindResourceInKernelMode)
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

BOOST_AUTO_TEST_SUITE_END()
