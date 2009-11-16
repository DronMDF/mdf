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

namespace Core {

CallHelper::CallHelper(const Task *task, id_t id, 
		       const void *buffer, size_t buffer_size, int flags)
	: m_status(SUCCESS), task(task), id(id),
	  buffer(buffer), buffer_size(buffer_size), flags(flags)
{
}

void CallHelper::setStatus(int status)
{
	m_status = status;
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
		
		if (resource == 0) {
			setStatus(ERROR_INVALIDID);
			return 0;
		}

		ResourceThread *thread = resource->Call();
		if (thread == 0) {
			setStatus(ERROR_INVALIDID);
			return 0;
		}

		return thread;
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
		if (thread == 0) {
			setStatus(ERROR_ACCESS);
			return 0;
		}
	
		return thread;
	}
	
	// TODO: поискать среди глобальных инстанций
	// Не всех, а только доступных публично.
	setStatus(ERROR_INVALIDID);
	return 0;
}

int CallHelper::execute()
{
	ResourceThread *calledthread = createCalledThread(task, id);
	if (calledthread == 0) return getStatus();

	// При указании буффера необходимо оставить ссылку на вызвавший процесс...
	// вызвавший идентификатор кстати будет размещен в стеке.
	
	// Передаем буфер (возврат кстати настраиваем отдельно, если нет флага ридонли)
	if (buffer != 0 && buffer_size != 0) {
		calledthread->setRequest (buffer, buffer_size, flags);
	}

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