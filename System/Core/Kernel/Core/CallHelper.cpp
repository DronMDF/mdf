//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "CallHelper.h"

#include "Kernel.h"
#include "Resources.h"
#include "ResourceInstance.h"
#include "Thread.h"
#include "Process.h"
#include "Scheduler.h"

#include "CoreLocal.h"

namespace Core {

CallHelper::CallHelper(const Task *task, id_t id, 
		       const void *buffer, size_t buffer_size, int flags)
	: task(task), id(id), buffer(buffer), buffer_size(buffer_size), flags(flags)
{
}

CallHelper::CallHelper(const Task *task)
	: m_caller(getCallerThread(task)), m_called(0)
{
}

CallHelper::~CallHelper()
{
}

ResourceThread *CallHelper::getCallerThread(const Task *task) const
{
	if (task == 0) return 0;
	
	ResourceThread *thread =
		reinterpret_cast<ResourceThread *>(StubTaskGetThread(task));
	STUB_ASSERT(thread == 0, "No current thread");

	return thread;
}

ResourceInstance *CallHelper::getCalledInstance(ResourceThread *thread, id_t id) const
{
	ResourceProcess *process = thread->getProcess();
	STUB_ASSERT(process == 0, "no current process");

	return process->FindInstance(id);
}

// TODO: Логика этой функции немного чрезмерна - в юзермоде она уже заранее
// создает вызываемую нить. Но это происходит из за того, что нет возможности
// получить инстанцию в обоих режимах.
bool CallHelper::checkCalledAccess(id_t id)
{
	if (m_caller == 0) return true;

	if (ResourceInstance *inst = getCalledInstance(m_caller, id)) {
		m_called = inst->Call();
		if (m_called == 0) return false;
	}

	return true;
}

Resource *CallHelper::findCalledResource(id_t id) const
{
	return Core::FindResource(id);
}

bool CallHelper::createCalled(id_t id)
{
	if (m_called != 0) return true;	// Определился на предыдущем шаге
		
	STUB_ASSERT(m_caller != 0, "call checkCalledAccess first");
	if (Resource *resource = Core::FindResource(id)) {
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

	STUB_ASSERT(!m_called->createRequestArea(0, size, access),
		    "Unable to create thread request area");
	STUB_ASSERT(!m_called->copyIn(USER_TXA_BASE, request, size),
		    "Unable to copy txa content");
	return true;
}

bool CallHelper::copyOutRequest(ResourceThread *thread, const void *request,
				size_t request_size, uint32_t access) const
{
	if (request == 0) return true;
	if (request_size == 0) return true;

	if (request_size > USER_TXA_SIZE) return false;

	STUB_ASSERT(!thread->createRequestArea(0, request_size, access),
		    "Unable to create thread request area");
	STUB_ASSERT(!thread->copyIn(USER_TXA_BASE, request, request_size),
		    "Unable to copy txa content");
	return true;
}

void CallHelper::setCopyBack(const void *buffer, size_t size) const
{
	if (m_caller == 0) return;
	if (buffer == 0) return;
	if (size == 0) return;

	laddr_t buffer_addr = reinterpret_cast<laddr_t>(buffer);
	m_called->setCopyBack(m_caller, buffer_addr, size);
}

void CallHelper::setCopyBack(ResourceThread *called, ResourceThread *thread,
	const void *buffer, size_t size) const
{
	if (thread == 0) return;
	if (buffer == 0) return;
	if (size == 0) return;
	
	laddr_t buffer_addr = reinterpret_cast<laddr_t>(buffer);
	called->setCopyBack(thread, buffer_addr, size);
}

void CallHelper::runSinchronized() const
{
	// Текущая нить ждет вечно
	m_caller->Sleep(CLOCK_MAX);
	Scheduler().addInactiveThread(m_caller);

	// Новая нить уведомит когда завершится
	m_called->addObserver(m_caller, RESOURCE_EVENT_DESTROY);

	// Новую нить запускаем.
	m_called->Run();

	// TODO: Нужно установить статус в caller, но пока он всегда SUCCESS,
	// 	Возможно потом появятся всякие TIMEOUT например.
}

void CallHelper::runSinchronized(ResourceThread *caller, ResourceThread *called) const
{
	// Текущая нить ждет вечно
	caller->Sleep(CLOCK_MAX);
	Scheduler().addInactiveThread(caller);

	// Новая нить уведомит когда завершится
	called->addObserver(caller, RESOURCE_EVENT_DESTROY);

	// Новую нить запускаем.
	called->Run();
	
	// TODO: Нужно установить статус в caller, но пока он всегда SUCCESS, 
	// 	Возможно потом появятся всякие TIMEOUT например.
}

int CallHelper::execute()
{
	ResourceThread *caller = getCallerThread(task);
	ResourceThread *called = 0;

	if (caller) {
		// User mode
		if (ResourceInstance *inst = getCalledInstance(caller, id)) {
			called = inst->Call();
			if (called == 0) return ERROR_ACCESS;
		}
	} else {
		// Kernel mode
		if (Resource *resource = findCalledResource(id)) {
			called = resource->Call();
		}
	}
	
	if (called == 0) return ERROR_INVALIDID;

	const uint32_t access = RESOURCE_ACCESS_READ |
		(isSet(flags, RESOURCE_CALL_READONLY) ? 0 : RESOURCE_ACCESS_WRITE);
	if (!copyOutRequest(called, buffer, buffer_size, access)) {
		return ERROR_INVALIDPARAM;
	}

	if (!isSet(flags, RESOURCE_CALL_READONLY)) {
		setCopyBack(called, caller, buffer, buffer_size);
	}

	if (isSet(flags, RESOURCE_CALL_ASYNC)) {
		// вызываемый просто ставится в очередь - управление не передается.
		Scheduler().addActiveThread(called);
	} else {
		STUB_ASSERT(caller == 0, "Fatal in kernel mode");
		runSinchronized(caller, called);
	}

	return SUCCESS;
}

} // namespace Core
