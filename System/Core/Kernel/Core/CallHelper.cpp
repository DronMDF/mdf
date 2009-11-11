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
	: task(task), id(id), buffer(buffer), buffer_size(buffer_size), flags(flags)
{
}

int CallHelper::execute()
{
	ResourceThread *thread = 0;

	// Которую потом коллить через полноценный метод Call.
	ResourceThread *calledthread = 0;

	if (task != 0) {
		void *thread_ptr = StubTaskGetThread(task);
		STUB_ASSERT (thread_ptr == 0, "No thread");
		thread = reinterpret_cast<ResourceThread *>(thread_ptr);

		ResourceProcess *process = thread->getProcess();
		STUB_ASSERT(process == 0, "no current process");

		ResourceInstance *instance = process->FindInstance(id);
		if (instance == 0) {
			// TODO: поискать среди глобальных инстанций
			return ERROR_INVALIDID;
		}

		calledthread = instance->Call();
	} else {
		Core::Resource *resource = Core::FindResource (id);
		if (resource == 0)
			return ERROR_INVALIDID;

		calledthread = resource->Call();
	}

	if (calledthread == 0)
		return ERROR_INVALIDID;

	// При указании буффера необходимо оставить ссылку на вызвавший процесс...
	// вызвавший идентификатор кстати будет размещен в стеке.
	if (buffer != 0 && buffer_size != 0) {
		calledthread->setRequest (buffer, buffer_size, flags);
	}

	if ((flags & RESOURCE_CALL_ASYNC) != 0) {
		Scheduler().addActiveThread(calledthread);
	} else {
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