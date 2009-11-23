//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <boost/test/unit_test.hpp>

#include "Types.h"
#include "../Kernel.h"
#include "../CallHelper.h"

#include "testResource.h"
#include "testThread.h"
#include "testProcess.h"

#include "TestHelpers.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCallHelper)

struct testCallHelper : public CallHelper {
	testCallHelper() : CallHelper(0, 0, 0, 0, 0) {}

	using CallHelper::getStatus;
	using CallHelper::createCalledThread;
	using CallHelper::copyOutRequest;
	using CallHelper::setCopyBack;
};
	
BOOST_AUTO_TEST_CASE(testCreateCalledThreadFromProcessInKernelMode)
{
	testCallHelper helper;
	
	// Тестирование стоит делать через процесс, потому что это единственно 
	// верный вариант в режиме ядра
	testProcess process;
	ResourceThread *thread = helper.createCalledThread(0, process.getId());
	BOOST_REQUIRE_EQUAL(helper.getStatus(), SUCCESS);
	
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
}

BOOST_AUTO_TEST_CASE(testCreateCalledThreadInUserMode)
{
	testCallHelper helper;
	
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	const Task *task = reinterpret_cast<Task *>(thread);
	BOOST_REQUIRE_EQUAL(helper.createCalledThread(task, thread->getId()), thread);
	BOOST_REQUIRE_EQUAL(helper.getStatus(), SUCCESS);
}

BOOST_AUTO_TEST_CASE(testCreateCalledThreadInvalidId)
{
	testCallHelper helper;
	const id_t invalid_id = 0xDEAD001D;
	BOOST_REQUIRE(helper.createCalledThread(0, invalid_id) == 0);
	BOOST_REQUIRE_EQUAL(helper.getStatus(), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_CASE(testCreateCalledThreadUncallable)
{
	testCallHelper helper;
	testResource uncallable;
	uncallable.Register();
	BOOST_REQUIRE(helper.createCalledThread(0, uncallable.getId()) == 0);
	BOOST_REQUIRE_EQUAL(helper.getStatus(), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_CASE(testCreateCalledThreadWithoutCallAccess)
{
	testCallHelper helper;
	
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_READ, 0);

	const Task *task = reinterpret_cast<Task *>(thread);
	BOOST_REQUIRE(helper.createCalledThread(task, thread->getId()) == 0);
	BOOST_REQUIRE_EQUAL(helper.getStatus(), ERROR_ACCESS);
}

BOOST_AUTO_TEST_CASE(testCopyOutRequest)
{
	testCallHelper helper;
	testProcess process;
	testThread thread(&process);

	char request[] = "request";
	BOOST_REQUIRE(helper.copyOutRequest(&thread, request, strlen(request), RESOURCE_ACCESS_READ));
	BOOST_REQUIRE_EQUAL(helper.getStatus(), SUCCESS);

	uint32_t access = RESOURCE_ACCESS_READ;
	const PageInstance *pinst = thread.PageFault(USER_TXA_BASE, &access);
	PageInfo *page = StubGetPageByInstance(pinst);
	BOOST_REQUIRE(page != 0);

	const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(request, request + strlen(request),
		m, m + strlen(request));
}

BOOST_AUTO_TEST_CASE(testCopyOutRequestInvalidParam)
{
	testCallHelper helper;
	char request[] = "request";
	BOOST_REQUIRE(!helper.copyOutRequest(0, request, USER_TXA_SIZE + 1, 0));
	BOOST_REQUIRE_EQUAL(helper.getStatus(), ERROR_INVALIDPARAM);
}

BOOST_AUTO_TEST_CASE(testCopyOutRequestValidNullPtr)
{
	testCallHelper helper;
	BOOST_REQUIRE(helper.copyOutRequest(0, 0, 1, 0));
	BOOST_REQUIRE_EQUAL(helper.getStatus(), SUCCESS);
}

BOOST_AUTO_TEST_CASE(testCopyOutRequestValidEmptyBufer)
{
	testCallHelper helper;
	char request[] = "request";
	BOOST_REQUIRE(helper.copyOutRequest(0, request, 0, 0));
	BOOST_REQUIRE_EQUAL(helper.getStatus(), SUCCESS);
}

BOOST_AUTO_TEST_CASE(testSetCopyBack)
{
	testCallHelper helper;
	testThread thread;
	class inlineThread : public testThread, private visit_mock {
	public:
		void setCopyBack(ResourceThread *, laddr_t, size_t) {
			visit();
		}
	} calledthread;
	
	char request[] = "request";
	BOOST_REQUIRE(helper.setCopyBack(&calledthread, &thread, request, sizeof(request)));
}

BOOST_AUTO_TEST_SUITE_END()
