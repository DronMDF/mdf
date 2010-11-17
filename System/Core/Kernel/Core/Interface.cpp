//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/CoreLocal.h"
#include "include/Scheduler.h"
#include "include/CallHelper.h"
#include "include/Resources.h"	// Для Core::FindResource

#include "include/InstanceProcess.h"
#include "include/Region.h"
#include "include/Process.h"
#include "include/ResourceThread.h"
#include "include/ResourceCall.h"

using namespace Core;

// -----------------------------------------------------------------------------
// Корочное апи...

extern "C"
int CoreWait (const Task *outask, id_t id, uint32_t event, timeout_t timeout)
{
	Core::ResourceThread *outhread =
		reinterpret_cast<Core::ResourceThread *>(StubTaskGetThread(outask));

	if (outhread != 0) {
		STUB_ASSERT(outhread->ScheduleLink.isLinked(), "Thread in list");

		outhread->setTimestamp(StubGetCurrentClock());

		// Манипуляции с выходящей нитью
		if (timeout > 0) {
			// Нить отправляется в спячку и управление к ней не
			// возвращается. Даже не смотря даже на тот факт что она
			// собирается кого-то ждать.

			if (id != 0) {
				Core::Resource *resource = Core::FindResource (id);

				if (resource == 0)
					return ERROR_INVALIDID;

				// Нить собирается кого-то ждать...
				// Поставим ее в очередь на ожидание к кому-то.
				outhread->Wait(resource, event);
			}

			outhread->Sleep(timeout);
			Scheduler().addInactiveThread(outhread);
		} else {
			Scheduler().addActiveThread(outhread);
		}
	}

	// TODO: Тут еще важно чтобы никто не схватил нить пока она стоит в очереди.
	// Промежуток add-get должен быть атомарным (блокировка?), Или поступить
	// корректнее и не ставить нить в очередь, если ее нечем заменить.

	Core::ResourceThread *inthread = Scheduler().getThread();
	if (inthread != 0) {
		if (inthread != outhread) {
			inthread->Run();
		}
	} else {
		// Нету нитей для выполнения
		StubCPUIdle();
	}

	return SUCCESS;
}

extern "C"
int CoreFind (const char *name __unused__, size_t name_size __unused__, id_t * const id __unused__)
{
	return ERROR_NOTIMPLEMENT;
}

extern "C"
int CoreCreate (const Task *task, int type, const void *param, size_t param_size, id_t *id)
{
	ResourceProcess *process = 0;

	if (task != 0) {
		const void *thread_ptr = StubTaskGetThread(task);
		STUB_ASSERT (thread_ptr == 0, "No thread");
		const ResourceThread *thread =
			reinterpret_cast<const ResourceThread *>(thread_ptr);

		process = thread->getProcess();
		STUB_ASSERT(process == 0, "no current process");
	}

	// TODO: Память должна быть доступна на чтение чтобы не вызывать исключений.
	if (param == 0 || param_size == 0)
		return ERROR_INVALIDPARAM;

	const struct KernelCreateRegionParam *region_param =
		reinterpret_cast<const struct KernelCreateRegionParam *>(param);
	const struct KernelCreateProcessParam *process_param =
		reinterpret_cast<const struct KernelCreateProcessParam *>(param);
	const struct KernelCreateThreadParam *thread_param =
		reinterpret_cast<const struct KernelCreateThreadParam *>(param);

	Resource *resource = 0;

	switch (type) {
		case RESOURCE_TYPE_REGION:
			if (param_size != sizeof (struct KernelCreateRegionParam))
				return ERROR_INVALIDPARAM;

			resource = new Region(region_param->size, region_param->access);
			break;

		case RESOURCE_TYPE_PROCESS:
			if (param_size != sizeof (struct KernelCreateProcessParam))
				return ERROR_INVALIDPARAM;

			resource = new ResourceProcess(process_param->entry);
			break;

		case RESOURCE_TYPE_THREAD:
			if (param_size != sizeof (struct KernelCreateThreadParam))
				return ERROR_INVALIDPARAM;

			resource = new ResourceThread(process, thread_param->entry);
			break;

		case RESOURCE_TYPE_CALL:
			// TODO: Это удобнее через исключения выбросить изнутри.
			if (param_size != sizeof(KernelCreateCallParam))
				return ERROR_INVALIDPARAM;

			resource = ResourceCall::Create(process, param, param_size);
			break;

		case RESOURCE_TYPE_CUSTOM:
			return ERROR_NOTIMPLEMENT;

		default:
			return ERROR_INVALIDPARAM;
	}

	STUB_ASSERT (resource == 0, "Unable to create resource");
	resource->Register();
	*id = resource->id();

	if (process != 0) {
		// Создадим инстанцию в текущем процессе
		process->Attach(resource, RESOURCE_ACCESS_OWNER, 0);
	}

	return SUCCESS;
}

extern "C"
int CoreCall (const Task *task, id_t id, const void *buffer, size_t buffer_size, uint32_t flags)
{
	CallHelper helper(task);

	if (!helper.checkCalledAccess(id)) return ERROR_ACCESS;
	if (!helper.createCalled(id)) return ERROR_INVALIDID;

	uint32_t access = RESOURCE_ACCESS_READ;
	if (!isSet(flags, RESOURCE_CALL_READONLY)) {
		access |= RESOURCE_ACCESS_WRITE;
	}
	
	if (!helper.copyOutRequest(buffer, buffer_size, access)) {
		return ERROR_INVALIDPARAM;
	}

	if (!isSet(flags, RESOURCE_CALL_READONLY)) {
		helper.setCopyBack(buffer, buffer_size);
	}

	if (isSet(flags, RESOURCE_CALL_ASYNC)) {
		helper.runAsinchronized();
	} else {
		helper.runSinchronized();
	}

	return SUCCESS;
}

extern "C"
int CoreAttach (const Task *task, id_t rid, id_t pid, uint32_t access, uint32_t spec)
{
	Core::ResourceThread *thread = 0;

	if (task != 0) {
		void *thread_ptr = StubTaskGetThread(task);
		STUB_ASSERT (thread_ptr == 0, "No thread!");
		thread = reinterpret_cast<Core::ResourceThread *>(thread_ptr);
	}

	// TODO: У текущего процесса должен быть доступ ATTACH к ресурсу res
	Core::Resource *resource = Core::FindResource(rid);
	if (resource == 0)
		return ERROR_INVALIDID;

	Core::Resource *target = 0;
	if (pid != 0) {
		target = Core::FindResource(pid);
		if (target == 0)
			return ERROR_INVALIDID;
	} else if (thread != 0) {
		target = thread->getProcess();
	}
	STUB_ASSERT (target == 0, "No process for attach!");

	return target->Attach(resource, access, spec);
}

extern "C"
int CoreDetach(const Task *task __unused__, id_t id __unused__, int flags __unused__)
{
	return ERROR_NOTIMPLEMENT;
}

extern "C"
int CoreModify (const Task *task, id_t id, int param_id, const void *param, size_t param_size)
{
	// TODO: Память должна быть доступна для чтения чтобы не вызывать фолтов.
	//if (!readable(param, param_size))
	//	return ERROR_INVALIDPARAM;

	if (task == 0) {
		// Прямой доступ к ресурсам - без родителя.
		Core::Resource *resource = Core::FindResource(id);
		if (resource == 0) return ERROR_INVALIDID;

		return resource->Modify(param_id, param, param_size);
	}

	const void *thread_ptr = StubTaskGetThread(task);
	STUB_ASSERT (thread_ptr == 0, "No thread");

	const Core::ResourceThread *thread =
		reinterpret_cast<const Core::ResourceThread *>(thread_ptr);

	// TODO: Можно было бы и в Thread сделать функцию ModifyResource,
	// но пока неясно на кой она может понадобиться. В процессе вот реально
	// понадобилась.

	Core::ResourceProcess *process = thread->getProcess();
	return process->ModifyResource (id, param_id, param, param_size);
}

extern "C"
int CoreInfo (const Task *task, id_t id, int info_id, void *info, size_t *info_size)
{
	STUB_ASSERT (task == 0, "No task");

	const void *thread_ptr = StubTaskGetThread(task);
	STUB_ASSERT (thread_ptr == 0, "No thread");

	const Core::ResourceThread *thread =
		reinterpret_cast<const Core::ResourceThread *>(thread_ptr);

	if (info_id == RESOURCE_INFO_THREAD_CURRENT) {
		return thread->Info(RESOURCE_INFO_THREAD_CURRENT, info, info_size);
	}

	const Core::ResourceProcess *process = thread->getProcess();
	STUB_ASSERT (process == 0, "no process");

	// TODO: Память должна быть доступна процессу для записи чтобы не вызывать фолтов.

	const InstanceProcess *instance = process->FindInstance(id);
	if (instance == 0) {
		// Возможно здесь налажу поиск по всем ресурсам, если понадобится.
		return ERROR_INVALIDID;
	}

	return instance->Info(info_id, info, info_size);
}
