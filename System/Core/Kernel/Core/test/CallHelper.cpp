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
#include "testScheduler.h"
#include "testSubScheduler.h"

#include "../ResourceInstance.h"

#include "TestHelpers.h"

using namespace Core;

BOOST_AUTO_TEST_SUITE(suiteCallHelper)

struct testCallHelper : public CallHelper {
	testCallHelper(const Task *task = 0) : CallHelper(task) {}

	using CallHelper::getCallerThread;
	using CallHelper::getCalledInstance;
	
	using CallHelper::copyOutRequest;
	using CallHelper::setCopyBack;

	using CallHelper::runSinchronized;

	using CallHelper::m_caller;
	using CallHelper::m_called;
};

BOOST_AUTO_TEST_CASE(testCreateHelperInKernelMode)
{
	testCallHelper helper(0);
	BOOST_REQUIRE(helper.m_caller == 0);
}

BOOST_AUTO_TEST_CASE(testCreateHelperInUserMode)
{
	testThread thread;
	const Task *task = reinterpret_cast<Task *>(&thread);
	testCallHelper helper(task);
	BOOST_REQUIRE_EQUAL(helper.m_caller, &thread);
}

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

BOOST_AUTO_TEST_CASE(testCheckCalledAccessInKernelMode)
{
	CallHelper helper(0);
	testThread called;
	BOOST_REQUIRE(helper.checkCalledAccess(called.getId()));
}

BOOST_AUTO_TEST_CASE(testCheckCalledAccessInUserMode)
{
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	struct testCallHelper : public CallHelper, private visit_mock {
		testCallHelper() : CallHelper(0) {}
		ResourceInstance *getCalledInstance(ResourceThread *thread, id_t id) const {
			visit();
			return CallHelper::getCalledInstance(thread, id);
		}
		using CallHelper::m_caller;
	} helper;
	helper.m_caller = thread;

	BOOST_REQUIRE(helper.checkCalledAccess(thread->getId()));
}

BOOST_AUTO_TEST_CASE(testCheckNoAccess)
{
	testProcess process;
	ResourceThread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_READ, 0);

	CallHelper helper(reinterpret_cast<Task *>(thread));
	BOOST_REQUIRE(!helper.checkCalledAccess(thread->getId()));
}

BOOST_AUTO_TEST_CASE(testCreateCalledAlready)
{
	testCallHelper helper(0);
	testThread caller;
	helper.m_called = &caller;
	BOOST_REQUIRE(helper.createCalled(0));
}

BOOST_AUTO_TEST_CASE(testCreateCalled)
{
	testCallHelper helper(0);
	testThread thread;
	BOOST_REQUIRE(helper.createCalled(thread.getId()));
	BOOST_REQUIRE_EQUAL(helper.m_called, &thread);
}

BOOST_AUTO_TEST_CASE(testCreateUncalled)
{
	testResource uncallable;
	uncallable.Register();

	CallHelper helper(0);
	BOOST_REQUIRE(!helper.createCalled(uncallable.getId()));
}

BOOST_AUTO_TEST_CASE(testCopyOutRequest)
{
	testProcess process;
	testThread thread(&process);

	testCallHelper helper;
	helper.m_called = &thread;

	char request[] = "request";
	BOOST_REQUIRE(helper.copyOutRequest(request, strlen(request), RESOURCE_ACCESS_READ));

	uint32_t access = RESOURCE_ACCESS_READ;
	const PageInstance *pinst = thread.PageFault(USER_TXA_BASE, &access);
	PageInfo *page = StubGetPageByInstance(pinst);
	BOOST_REQUIRE(page != 0);

	const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(request, request + strlen(request),
		m, m + strlen(request));
}

// BOOST_AUTO_TEST_CASE(testCopyOutRequestInvalidParam)
// {
// 	testCallHelper helper;
// 	char request[] = "request";
// 	BOOST_REQUIRE(!helper.copyOutRequest(0, request, USER_TXA_SIZE + 1, 0));
// }
// 
// BOOST_AUTO_TEST_CASE(testCopyOutRequestValidNullPtr)
// {
// 	testCallHelper helper;
// 	BOOST_REQUIRE(helper.copyOutRequest(0, 0, 1, 0));
// }
// 
// BOOST_AUTO_TEST_CASE(testCopyOutRequestValidEmptyBufer)
// {
// 	testCallHelper helper;
// 	char request[] = "request";
// 	BOOST_REQUIRE(helper.copyOutRequest(0, request, 0, 0));
// }

BOOST_AUTO_TEST_CASE(testSetCopyBack)
{
	class inlineThread : public testThread, private visit_mock {
	public:
		void setCopyBack(ResourceThread *thread, laddr_t buffer, size_t size) {
			visit();
			BOOST_REQUIRE_EQUAL(thread, reinterpret_cast<ResourceThread *>(0x105EAD));
			BOOST_REQUIRE_EQUAL(buffer, 0xADD0000);
			BOOST_REQUIRE_EQUAL(size, 20);
		}
	} thread;

	testCallHelper helper;
	helper.m_caller = reinterpret_cast<ResourceThread *>(0x105EAD);
	helper.m_called = &thread;

	helper.setCopyBack(reinterpret_cast<void *>(0xADD0000), 20);
}

BOOST_AUTO_TEST_CASE(testSetCopyBackValidity)
{
	class inlineThread : public testThread {
	public:
		void setCopyBack(ResourceThread *, laddr_t, size_t) {
			throw 0;
		}
	} thread;

	testCallHelper helper;
	helper.m_called = &thread;
	helper.m_caller = 0;
	BOOST_REQUIRE_NO_THROW(helper.setCopyBack(reinterpret_cast<void *>(0xADD0000), 20));

	helper.m_caller = reinterpret_cast<ResourceThread *>(0x105EAD);
	BOOST_REQUIRE_NO_THROW(helper.setCopyBack(0, 20));
	BOOST_REQUIRE_NO_THROW(helper.setCopyBack(reinterpret_cast<void *>(0xADD0000), 0));
}

BOOST_AUTO_TEST_CASE(testRunAsinchronized)
{
	testSubScheduler *subsched = new testSubScheduler;

	testScheduler scheduler;
	scheduler.m_actives = subsched;

	testThread called;

	testCallHelper helper;
	helper.m_called = &called;

	helper.runAsinchronized();
	BOOST_REQUIRE_EQUAL(subsched->thread, &called);
}

BOOST_AUTO_TEST_CASE(testRunSinchronized)
{
	testSubScheduler *subsched = new testSubScheduler;
	
	testScheduler scheduler;
	scheduler.m_inactives = subsched;

	testThread caller;

	class inlineThread : public testThread, private order_mock<2> {
	private:
		ResourceThread *m_caller;
	public:
		inlineThread(ResourceThread *thread) : m_caller(thread) {}
		void addObserver(ResourceThread *thread, uint32_t event) {
			order(1);
			BOOST_REQUIRE_EQUAL(thread, m_caller);
			BOOST_REQUIRE_EQUAL(event, RESOURCE_EVENT_DESTROY);
		}
		void Run() {
			order(2);
		}
	} called(&caller);

	testCallHelper helper;
	helper.m_caller = &caller;
	helper.m_called = &called;

	helper.runSinchronized();
	BOOST_REQUIRE_EQUAL(caller.getWakeupstamp(), CLOCK_MAX);
	BOOST_REQUIRE_EQUAL(subsched->thread, &caller);
}

BOOST_AUTO_TEST_SUITE_END()
