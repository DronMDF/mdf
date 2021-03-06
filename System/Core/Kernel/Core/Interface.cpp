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
#include "include/Thread.h"
#include "include/CallPoint.h"
#include "include/Custom.h"

using namespace Core;

// -----------------------------------------------------------------------------
// Корочное апи...

extern "C"
int CoreWait (const Task *outask, id_t id, uint32_t event, timeout_t timeout)
{
	Core::Thread *outhread =
		reinterpret_cast<Core::Thread *>(StubTaskGetThread(outask));

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

	Core::Thread *inthread = Scheduler().getThread();
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

static
Process *getProcessByTask(const Task *task)
{
	// TODO: Можно вернуть NullProcess, который ничего не делает и все разрешает
	if (task == 0) return 0;

	const void *thread_ptr = StubTaskGetThread(task);
	STUB_ASSERT (thread_ptr == 0, "No current thread");
	
	const Thread *thread = reinterpret_cast<const Thread *>(thread_ptr);

	Process *process = thread->getProcess();
	STUB_ASSERT(process == 0, "No current process");
	
	return process;
}

extern "C"
int CoreCreate (const Task *task, int type, const void *param, size_t param_size, id_t *id)
{
	Process *process = getProcessByTask(task);

	// TODO: Память должна быть доступна на чтение чтобы не вызывать исключений.
	if (type != RESOURCE_TYPE_CUSTOM && (param == 0 || param_size == 0)) {
		return ERROR_INVALIDPARAM;
	}

	// TODO: Сделать отдельные криейторы на каждый тип
	const struct KernelCreateProcessParam *process_param =
		reinterpret_cast<const struct KernelCreateProcessParam *>(param);
	const struct KernelCreateThreadParam *thread_param =
		reinterpret_cast<const struct KernelCreateThreadParam *>(param);

	Resource *resource = 0;

	switch (type) {
		case RESOURCE_TYPE_REGION:
			resource = Region::Create(param, param_size);
			break;
		case RESOURCE_TYPE_PROCESS:
			if (param_size != sizeof (struct KernelCreateProcessParam))
				return ERROR_INVALIDPARAM;

			resource = new Process(process_param->entry);
			break;
		case RESOURCE_TYPE_THREAD:
			if (param_size != sizeof (struct KernelCreateThreadParam))
				return ERROR_INVALIDPARAM;

			resource = new Thread(process, thread_param->entry);
			break;
		case RESOURCE_TYPE_CALL:
			resource = CallPoint::Create(process, param, param_size);
			break;
		case RESOURCE_TYPE_CUSTOM:
			resource = Custom::Create();
			break;
	}

	if (resource == 0) return ERROR_INVALIDPARAM;
	
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
	// TODO: Хелпер надо убить и разрулить локальными функциями
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
	Core::Thread *thread = 0;

	if (task != 0) {
		void *thread_ptr = StubTaskGetThread(task);
		STUB_ASSERT (thread_ptr == 0, "No thread!");
		thread = reinterpret_cast<Core::Thread *>(thread_ptr);
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
	if (task == 0) {
		Resource *resource = FindResource(id);
		if (resource == 0) return ERROR_INVALIDID;

		return resource->Modify(param_id, param, param_size);
	}
	
	Process *process = getProcessByTask(task);
	return process->ModifyResource (id, param_id, param, param_size);
}

extern "C"
int CoreInfo (const Task *task, id_t id, int info_id, void *info, size_t *info_size)
{
	// TODO: Здесь помогла бы функция getThreadByTask, но потом.
	STUB_ASSERT (task == 0, "No task");

	const void *thread_ptr = StubTaskGetThread(task);
	STUB_ASSERT (thread_ptr == 0, "No thread");

	const Core::Thread *thread =
		reinterpret_cast<const Core::Thread *>(thread_ptr);

	if (info_id == RESOURCE_INFO_THREAD_CURRENT) {
		return thread->Info(RESOURCE_INFO_THREAD_CURRENT, info, info_size);
	}

	const Core::Process *process = thread->getProcess();
	STUB_ASSERT (process == 0, "no process");

	// TODO: Память должна быть доступна процессу для записи чтобы не вызывать фолтов.

	const InstanceProcess *instance = process->FindInstance(id);
	if (instance == 0) {
		// Возможно здесь налажу поиск по всем ресурсам, если понадобится.
		return ERROR_INVALIDID;
	}

	return instance->Info(info_id, info, info_size);
}
