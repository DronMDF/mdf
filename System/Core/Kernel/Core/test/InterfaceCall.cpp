//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <iostream>
#include <boost/test/unit_test.hpp>
#include <boost/scoped_ptr.hpp>

#include "Types.h"
#include "../include/Kernel.h"
#include "../include/Core.h"
#include "../include/CallPoint.h"

#include "testResource.h"
#include "testProcess.h"
#include "testThread.h"
#include "testScheduler.h"
#include "testSubScheduler.h"

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
	
	BOOST_REQUIRE_EQUAL(CoreCall(0, process.id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	Thread *thread = scheduler.getThread();
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
	Thread *thread = new testThread(&process);
	process.Attach(thread, RESOURCE_ACCESS_CALL, 0);

	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(thread),
			thread->id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	Thread *st = scheduler.getThread();
	BOOST_REQUIRE_EQUAL(st, thread);
}

BOOST_AUTO_TEST_CASE(testCallCallAsyncByProcessInstance)
{
	testScheduler scheduler;

	const laddr_t entry = 6666;

	testProcess process;
	Resource *call = CallPoint::Create(&process, &entry, sizeof(laddr_t));
	call->Register();
	process.Attach(call, RESOURCE_ACCESS_CALL, 0);

	testThread task(&process);
	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(&task),
			call->id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	Thread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_CASE(testCallProcessAsyncByProcessInstance)
{
	testScheduler scheduler;

	const laddr_t entry = 6666;

	testProcess process;
	Process *calledprocess = new testProcess(entry);
	process.Attach(calledprocess, RESOURCE_ACCESS_CALL, 0);
	
	testThread task(&process);
	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(&task),
			calledprocess->id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	Thread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), calledprocess);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_CASE(testCallParametersDeliver)
{
	testScheduler scheduler;
	testProcess process(0);
	
	string test_msg = "test";
	BOOST_REQUIRE_EQUAL(CoreCall(0, process.id(), 
		test_msg.c_str(), test_msg.size(), 
		RESOURCE_CALL_ASYNC | RESOURCE_CALL_COPY), SUCCESS);

	Thread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);

	uint32_t access = RESOURCE_ACCESS_READ;
	const PageInstance *pinst = thread->PageFault(USER_TXA_BASE, &access);
	PageInfo *page = StubGetPageByInstance(pinst);
	BOOST_REQUIRE(page != 0);
	
	const char *m = reinterpret_cast<const char *>(StubPageTemporary(page));
	BOOST_REQUIRE_EQUAL_COLLECTIONS(test_msg.begin(), test_msg.end(),
		m, m + test_msg.size());
}

BOOST_AUTO_TEST_CASE(testCallSync)
{
	// Все ломается из за евентов, которые зависают в коллере.
// 	testSubScheduler *inactive = new testSubScheduler;
// 	testScheduler scheduler;
// 	scheduler.m_inactives = inactive; // Освободиться при уничтожении шедулера.
// 
// 	testThread task;
// 	testProcess process;	// Процесс должен умирать раньше чем нить.
// 	task.setProcess(&process);
// 	
// 	BOOST_REQUIRE_EQUAL(CoreCall(reinterpret_cast<Task *>(&task),
// 			process.id(), 0, 0, 0), SUCCESS);
// 
// //	TODO: Обработчик уже вызван... и я не могу это отследить... пока...
// // 	Thread *thread = scheduler.getThread();
// // 	BOOST_REQUIRE(thread != 0);
// // 	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
// // 	
// // 	BOOST_REQUIRE(scheduler.getThread() == 0);
// 	
// 	// TODO: Хотел ту нить прибить и эта по идее должна была бы оказаться в 
// 	// очереди активных, но это не сработало потому что активизация слишком 
// 	// сильно все пронизывает. :( Пришлось вставить свой шедулер.
// 	BOOST_REQUIRE(inactive->thread == &task);
// 	BOOST_REQUIRE_EQUAL(task.getWakeupstamp(), CLOCK_MAX);
}

// -----------------------------------------------------------------------------
// Несчастливые маршруты (ошибки)

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
	BOOST_REQUIRE_EQUAL(CoreCall(0, uncallable.id(), 0, 0, RESOURCE_CALL_ASYNC), ERROR_INVALIDID);
}

// -----------------------------------------------------------------------------
// Два следующих теста немного неправдоподобны.
// Клиенты так делать не могут, а ядру это не за чем.

BOOST_AUTO_TEST_CASE(testCallThreadAsyncWithoutCaller)
{
	testThread thread;
	BOOST_REQUIRE_EQUAL(CoreCall(0, thread.id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	testScheduler scheduler;
	BOOST_REQUIRE_EQUAL(scheduler.getThread(), &thread);
}

BOOST_AUTO_TEST_CASE(testCallCallAsyncWithoutCaller)
{
	testScheduler scheduler;
	
	testProcess process;
	const laddr_t entry = 6666;
	Resource *call = CallPoint::Create(&process, &entry, sizeof(laddr_t));
	call->Register();

	BOOST_REQUIRE_EQUAL(CoreCall(0, call->id(), 0, 0, RESOURCE_CALL_ASYNC), SUCCESS);

	Thread *thread = scheduler.getThread();
	BOOST_REQUIRE(thread != 0);
	BOOST_REQUIRE_EQUAL(thread->getProcess(), &process);
	BOOST_REQUIRE_EQUAL(thread->getEntry(), entry);
}

BOOST_AUTO_TEST_SUITE_END()
