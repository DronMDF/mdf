//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Stub.h"
#include "Kernel.h"

#include "CoreLocal.h"	// TODO: Делить на классы

#include "List.h"
#include "Memory.h"
#include "Resource.h"
#include "ResourceInstance.h"
#include "Thread.h"
#include "Process.h"
#include "Scheduler.h"

namespace Core {

// TODO: Этот конструктор вообще уже не нужен.
ResourceThread::ResourceThread (ResourceProcess *process)
	: Resource(),
	  m_process(process),
	  m_task(0),
	  m_stack(USER_STACK_SIZE, Memory::ALLOC),
	  m_txa(0),
	  m_txa_offset(0),
	  m_txa_access(0),
	  m_timestamp(0),
	  m_wakeupstamp(0),
	  m_priority(255),
	  m_entry(0),
	  m_event(0),
	  ScheduleLink(),
	  EventLink()
{
}

// TODO: А этот конструктор непонятно зачем использует ссылку на процесс,
// И кроме того зачем-то инициализирует стек, это можно сделать и позже.
ResourceThread::ResourceThread (ResourceProcess *process, laddr_t entry)
	: Resource(),
	  m_process(process),
	  m_task(StubTaskCreate (entry, this)),
	  m_stack(USER_STACK_SIZE, Memory::ALLOC),
	  m_txa(0),
	  m_txa_offset(0),
	  m_txa_access(0),
	  m_timestamp(0),
	  m_wakeupstamp(0),
	  m_priority(255),
	  m_entry(entry),
	  m_event(0),
	  ScheduleLink(),
	  EventLink()
{
	STUB_ASSERT(m_task == 0, "Unable to create task");

	// Проинициализировать стек по умолчанию..
	setStack (0, 0, 0);
}

ResourceThread::~ResourceThread()
{
	delete m_txa;
}

ResourceThread *ResourceThread::asThread ()
{
	return this;
}

void ResourceThread::setStack (offset_t request, size_t request_size, int flags)
{
	struct StubStackFrame stack_frame;

	// TODO: Установить идентификатор вызывающей нити.
	StubSetStackFrame (&stack_frame, 0, request, request_size, flags);

	m_stack.Copy (&stack_frame, sizeof (struct StubStackFrame),
		USER_STACK_SIZE - sizeof (struct StubStackFrame));
}

void ResourceThread::setRequest (const void *request, size_t request_size, int flags)
{
	// Запрос всегда находится в текущем линейном пространстве.
	STUB_ASSERT (m_txa != 0, "Request already exist");

	m_txa_offset = 0;

	if (!isSet(flags, RESOURCE_CALL_COPY)) {
		// При маппинге параметров учитывается смещение.
		m_txa_offset = reinterpret_cast<laddr_t>(request) & ~(PAGE_SIZE - 1); //PFLAG_MASK
	}

	STUB_ASSERT (m_txa_offset + request_size > USER_TXA_SIZE, "Big request");

	// TODO: В связи с новой логикой Memory txa лучше будет
	// представить в виде региона. Причем здесь хранить инстанцию.
	m_txa = new Memory (request_size, Memory::ALLOC);
	m_txa_access = RESOURCE_ACCESS_READ |
		(isSet(flags, RESOURCE_CALL_READONLY) ? 0 : RESOURCE_ACCESS_WRITE);

	if (isSet(flags, RESOURCE_CALL_COPY)) {
		m_txa->Copy (request, request_size);
	} else {
		m_txa->Map (request, request_size);
	}

	// TODO: В стек наверное надо устанавливать смещение в пространстве
	//	пользователя.
	setStack (m_txa_offset, request_size, flags);

	if (!isSet(flags, RESOURCE_CALL_READONLY) &&
		isSet(flags, RESOURCE_CALL_COPY))
	{
		// Зафиксировать место откуда блок поступил, чтобы потом скопировать его обратно.
	}
}

uint32_t ResourceThread::getPriority() const
{
	return m_priority;
}

void ResourceThread::setPriority(uint32_t priority)
{
	m_priority = priority;
}

clock_t ResourceThread::getTimestamp() const
{
	return m_timestamp;
}

clock_t ResourceThread::getWakeupstamp() const
{
	return m_wakeupstamp;
}

void ResourceThread::setTimestamp(clock_t timestamp)
{
	m_timestamp = timestamp;
}

ResourceProcess *ResourceThread::getProcess() const
{
	STUB_ASSERT(m_process == 0, "Thread without process");
	return m_process;
}

laddr_t ResourceThread::getEntry() const
{
	return m_entry;
}

void ResourceThread::Sleep (timeout_t timeout)
{
	m_wakeupstamp = timeout;
	if (m_wakeupstamp != CLOCK_MAX) {
		m_wakeupstamp += StubGetCurrentClock();
	}
}

void ResourceThread::Wait (Resource *resource __unused__, int event __unused__)
{

}

bool ResourceThread::isActive () const
{
	return false;
}

void ResourceThread::Run()
{
	// TODO: Снять нить из очереди ожидания, если она там стоит.
	STUB_ASSERT(ScheduleLink.isLinked(), "Running thread linked by scheduler link");
	STUB_ASSERT(EventLink.isLinked(), "Running thread linked by event link");

	StubTaskRun (m_task);
}

void ResourceThread::Activate()
{
	ScheduleLink.Unlink(this);
	Scheduler().addActiveThread(this);
}

bool ResourceThread::Deactivate()
{
	STUB_ASSERT(getInstancesCount() > 1, "Many instances for thread");

	// должен попытаться удалить Task, если получилось - вернет true
	if (StubTaskDestroy(m_task)) {
		m_task = 0;
		return true;
	}

	return false;
}

void ResourceThread::Kill()
{
	// TODO: вернуть TPC буфер в вызывающий процесс (если надо)

	Scheduler().addKillThread(this);

	// Вызвать очередную нить, или Halt.
	// TODO: А может быть заставить Scheduler хальтить при отсутствии нитей?
	ResourceThread *thread = Scheduler().getThread();
	if (thread != 0) {
		thread->Run();
	} else {
		StubCPUIdle();
	}
}

const PageInstance *ResourceThread::PageFault (laddr_t addr, uint32_t *access)
{
	if (addr == USER_MEMORY_BASE + RETMAGIC) {
		Kill();
		STUB_FATAL("Thread is killed.");
	}

	if (m_stack.inBounds(USER_STACK_BASE, addr)) {
		*access &= RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE;
		return m_stack.PageFault (addr - USER_STACK_BASE);
	}

	if (m_txa && m_txa->inBounds(USER_TXA_BASE, addr, m_txa_offset)) {
		*access &= m_txa_access;
		return m_txa->PageFault(addr - USER_TXA_BASE);
	}

	return m_process->PageFault(addr, access);
}

int ResourceThread::Modify(int param_id, const void *param, size_t param_size)
{
	if (param_id == RESOURCE_MODIFY_THREAD_PRIORITY) {
		if (param_size < sizeof(uint32_t))
			return ERROR_INVALIDPARAM;

		// TODO: Проверить границы допустимого приоритета.
		m_priority = *reinterpret_cast<const uint32_t *>(param);
		return SUCCESS;
	}

	return Resource::Modify(param_id, param, param_size);
}

int ResourceThread::Info(int info_id, void *info, size_t *info_size) const
{
	if (info_id == RESOURCE_INFO_THREAD_CURRENT) {
		const id_t id = getId();
		STUB_ASSERT(id == INVALID_ID, "invalid id");
		return StubInfoValue(info, info_size, &id, sizeof(id_t));
	}

	return Resource::Info(info_id, info, info_size);
}

ResourceThread *ResourceThread::Call ()
{
	// TODO: Указатель должен возвращаться только для неинициализированных нитей.
	// Запущенная нить возвращает 0.
	return this;
}

uint32_t ResourceThread::getEvent() const
{
	return m_event;
}

void ResourceThread::setEvent(uint32_t event)
{
	m_event = event;
}

} // namespace Core

extern "C"
const PageInstance *CoreThreadPageFault (const Task *task, laddr_t addr, uint32_t *access)
{
	Core::ResourceThread *thread =
		reinterpret_cast<Core::ResourceThread *>(StubTaskGetThread(task));

	const PageInstance *instance = thread->PageFault(addr, access);

	return instance;
}

