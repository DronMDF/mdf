//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/CallHelper.h"

#include "include/Kernel.h"
#include "include/Scheduler.h"
#include "include/CoreLocal.h"

#include "include/InstanceProcess.h"
#include "include/Resources.h"
#include "include/Process.h"
#include "include/Thread.h"

using namespace Core;

CallHelper::CallHelper(const Task *task)
	: m_caller(getCallerThread(task)), m_called(0)
{
}

CallHelper::~CallHelper()
{
}

Thread *CallHelper::getCallerThread(const Task *task) const
{
	if (task == 0) return 0;
	
	Thread *thread =
		reinterpret_cast<Thread *>(StubTaskGetThread(task));
	STUB_ASSERT(thread == 0, "No current thread");

	return thread;
}

InstanceProcess *CallHelper::getCalledInstance(Thread *thread, id_t id) const
{
	Process *process = thread->getProcess();
	STUB_ASSERT(process == 0, "no current process");

	return process->FindInstance(id);
}

// TODO: Логика этой функции немного чрезмерна - в юзермоде она уже заранее
// создает вызываемую нить. Но это происходит из за того, что нет возможности
// получить инстанцию в обоих режимах.
bool CallHelper::checkCalledAccess(id_t id)
{
	if (m_caller == 0) return true;

	if (InstanceProcess *inst = getCalledInstance(m_caller, id)) {
		m_called = inst->Call();
		if (m_called == 0) return false;
	}

	return true;
}

bool CallHelper::createCalled(id_t id)
{
	if (m_called != 0) return true;		// Определился на предыдущем шаге
	if (m_caller != 0) return false;	// Не определился на предыдущем шаге.
		
	if (Resource *resource = FindResource(id)) {
		m_called = resource->Call();
	}

	return m_called != 0;
}

bool CallHelper::copyOutRequest(const void *request, size_t size,
				uint32_t access) const
{
	STUB_ASSERT(m_called == 0, "Missing called thread");

	if (request == 0) return true;
	if (size == 0) return true;

	if (size > USER_TXA_SIZE) return false;

	STUB_ASSERT(!m_called->createRequestArea(m_caller, 0, size, access),
		    "Unable to create thread request area");
	STUB_ASSERT(!m_called->copyIn(USER_TXA_BASE, request, size),
		    "Unable to copy txa content");
	return true;
}

void CallHelper::setCopyBack(const void *buffer, size_t size) const
{
	if (m_caller == 0) return;
	if (buffer == 0) return;
	if (size == 0) return;

	m_called->setCopyBack(m_caller, reinterpret_cast<laddr_t>(buffer));
}

void CallHelper::runAsinchronized() const
{
	// вызываемый просто ставится в очередь - управление не передается.
	Scheduler().addActiveThread(m_called);
}

void CallHelper::runSinchronized() const
{
	STUB_ASSERT(m_caller == 0, "Fatal in kernel mode");

	// Текущая нить ждет вечно
	m_caller->Sleep(TIMEOUT_INFINITY);
	Scheduler().addInactiveThread(m_caller);
	
	// Новая нить уведомит когда завершится
	m_caller->Wait(m_called, RESOURCE_EVENT_THREAD_EXIT);
	
	// Новую нить запускаем.
	m_called->Run();

	// TODO: Нужно установить статус в caller, но пока он всегда SUCCESS,
	// 	Возможно потом появятся всякие TIMEOUT например.
}
