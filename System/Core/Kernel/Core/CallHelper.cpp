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
	: m_status(SUCCESS), task(task), id(id),
	  buffer(buffer), buffer_size(buffer_size), flags(flags)
{
}

template <typename T>
T CallHelper::returnStatus(int status, T value)
{
	if (value == 0) m_status = status;
	return value;
}

int CallHelper::getStatus() const
{
	return m_status;
}

ResourceThread *CallHelper::createCalledThread(const Task *task, id_t id)
{
	if (task == 0) {
		// Режим ядра - поиск ресурсов осуществляется глобально.
		Core::Resource *resource = Core::FindResource(id);
		if (resource == 0) return returnStatus<ResourceThread *>(ERROR_INVALIDID);

		ResourceThread *thread = resource->Call();
		return returnStatus<ResourceThread *>(ERROR_INVALIDID, thread);
	}
	
	// Режим пользователя - поиск осуществляется от процесса.
	ResourceThread *thread = 
		reinterpret_cast<ResourceThread *>(StubTaskGetThread(task));
	STUB_ASSERT(thread == 0, "No current thread");

	ResourceProcess *process = thread->getProcess();
	STUB_ASSERT(process == 0, "no current process");

	ResourceInstance *instance = process->FindInstance(id);
	if (instance != 0) {
		ResourceThread *thread = instance->Call();
		return returnStatus<ResourceThread *>(ERROR_ACCESS, thread);
	}
	
	// TODO: поискать среди глобальных инстанций
	// Не всех, а только доступных публично.
	return returnStatus<ResourceThread *>(ERROR_INVALIDID);
}

bool CallHelper::copyOutRequest(ResourceThread *thread, const void *request,
				size_t request_size, uint32_t access)
{
	if (request == 0) return true;
	if (request_size == 0) return true;

	if (request_size > USER_TXA_SIZE) return returnStatus<bool>(ERROR_INVALIDPARAM);

	STUB_ASSERT(!thread->createRequestArea(0, request_size, access),
		    "Unable to create thread request area");
	STUB_ASSERT(!thread->copyIn(USER_TXA_BASE, request, request_size),
		    "Unable to copy txa content");
	return true;
}

int CallHelper::execute()
{
	ResourceThread *calledthread = createCalledThread(task, id);
	if (calledthread == 0) return getStatus();

	const uint32_t access = RESOURCE_ACCESS_READ |
		(isSet(flags, RESOURCE_CALL_READONLY) ? 0 : RESOURCE_ACCESS_WRITE);
	if (!copyOutRequest(calledthread, buffer, buffer_size, access)) {
		return getStatus();
	}

	// TODO: Если флаг READONLY не установлен и флаг COPY установлен,
	// и буфер есть, то установить в calledthread CopyBackParams

	if ((flags & RESOURCE_CALL_ASYNC) != 0) {
		// вызываемый просто ставится в очередь - управление не передается.
		Scheduler().addActiveThread(calledthread);
	} else {
		// Синхронный вызов - вызывающий отправляется спать до завершения 
		// работы вызываемого.
		ResourceThread *thread = 
			reinterpret_cast<ResourceThread *>(StubTaskGetThread(task));
		STUB_ASSERT(thread == 0, "Sync call without thread");

		// Текущая нить ждет вечно
		thread->Sleep(CLOCK_MAX);
		Scheduler().addInactiveThread(thread);

		// Новая нить уведомит когда завершится
		calledthread->addObserver(thread, RESOURCE_EVENT_DESTROY);

		// Новую нить запускаем.
		calledthread->Run();
	}

	return SUCCESS;
}

} // namespace Core