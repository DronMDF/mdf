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

#include "../ResourceInstance.h"

#include "TestHelpers.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCallHelper)

struct testCallHelper : public CallHelper {
	testCallHelper() : CallHelper(0, 0, 0, 0, 0) {}

	using CallHelper::getStatus;

	using CallHelper::getCallerThread;
	using CallHelper::findCalledResource;
	using CallHelper::getCalledInstance;
	
	using CallHelper::copyOutRequest;
	using CallHelper::setCopyBack;
};

BOOST_AUTO_TEST_CASE(testGetCallerThreadInKernelMode)
{
	testCallHelper helper;
	BOOST_REQUIRE(helper.getCallerThread(0) == 0);
}

BOOST_AUTO_TEST_CASE(testGetCallerThreadInUserMode)
{
	testCallHelper helper;
	testThread thread;
	const Task *task = reinterpret_cast<Task *>(&thread);
	BOOST_REQUIRE_EQUAL(helper.getCallerThread(task), &thread);
}

BOOST_AUTO_TEST_CASE(testFindCalledResource)
{
	testCallHelper helper;
	testProcess process;
	BOOST_REQUIRE(helper.findCalledResource(process.getId()) == &process);
}

BOOST_AUTO_TEST_CASE(testFindCalledResourceInvalidId)
{
	testCallHelper helper;
	BOOST_REQUIRE(helper.findCalledResource(0xDEAD001D) == 0);
}

BOOST_AUTO_TEST_CASE(testGetCalledInstance)
{
	testCallHelper helper;

	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	ResourceInstance *inst = helper.getCalledInstance(thread, thread->getId());
	BOOST_REQUIRE(inst != 0);
	BOOST_REQUIRE(inst->getResource() == thread);
	BOOST_REQUIRE_EQUAL(inst->Call(), thread);
}

BOOST_AUTO_TEST_CASE(testCallUncallable)
{
	testResource uncallable;
	uncallable.Register();
	
	CallHelper helper(0, uncallable.getId(), 0, 0, 0);
	BOOST_REQUIRE_EQUAL(helper.execute(), ERROR_INVALIDID);
}

BOOST_AUTO_TEST_CASE(testCallWithoutCallAccess)
{
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_READ, 0);

	const Task *task = reinterpret_cast<Task *>(thread);

	CallHelper helper(task, thread->getId(), 0, 0, 0);
	BOOST_REQUIRE_EQUAL(helper.execute(), ERROR_ACCESS);
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
	
	class inlineThread : public testThread, private visit_mock {
	public:
		void setCopyBack(ResourceThread *thread, laddr_t buffer, size_t size) {
			visit();
			BOOST_REQUIRE_EQUAL(thread, reinterpret_cast<ResourceThread *>(0x105EAD));
			BOOST_REQUIRE_EQUAL(buffer, 0xADD0000);
			BOOST_REQUIRE_EQUAL(size, 20);
		}
	} calledthread;

	// Параметры разнообразные для контроля и изоляции..
	BOOST_REQUIRE(helper.setCopyBack(&calledthread,
		reinterpret_cast<ResourceThread *>(0x105EAD),
		reinterpret_cast<void *>(0xADD0000), 20));
}

BOOST_AUTO_TEST_SUITE_END()
