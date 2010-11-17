//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Stub.h"
#include "include/Kernel.h"

#include "include/CoreLocal.h"	// TODO: Делить на классы
#include "include/List.h"
#include "include/Memory.h"
#include "include/Scheduler.h"
#include "include/Resources.h"	// TODO: Устаревающее

#include "include/Instance.h"
#include "include/InstanceThread.h"
#include "include/InstanceCopyBack.h"
#include "include/Resource.h"
#include "include/Process.h"
#include "include/ResourceThread.h"

using namespace Core;

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
	  m_event_instance(0),
	  m_copyback_instance(0),
	  ScheduleLink()
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
	  m_event_instance(0),
	  m_copyback_instance(0),
	  ScheduleLink()
{
	STUB_ASSERT(m_task == 0, "Unable to create task");

	// Проинициализировать стек по умолчанию..
	setStack (0, 0, 0);
}

ResourceThread::~ResourceThread()
{
	// Если нить не вышла корректно - доставки результата уже не будет.
	delete m_copyback_instance;
	delete m_event_instance;
	delete m_txa;
}

ResourceThread *ResourceThread::asThread()
{
	return this;
}

void ResourceThread::setStack(offset_t request, size_t request_size, uint32_t access)
{
	struct StubStackFrame stack_frame;

	// TODO: Установить идентификатор вызывающей нити.
	StubSetStackFrame(&stack_frame, 0, request, request_size, access);

	m_stack.copyIn(USER_STACK_SIZE - sizeof(struct StubStackFrame),
		       &stack_frame, sizeof(struct StubStackFrame));
}

void ResourceThread::setRequest(const void *request, size_t size, uint32_t access)
{
	// Запрос всегда находится в текущем линейном пространстве.
	STUB_ASSERT (m_txa != 0, "Request already exist");

	m_txa_offset = 0;

	const bool map_request = !isSet(access, RESOURCE_CALL_COPY);
	
	if (map_request) {
		// При маппинге параметров учитывается смещение.
		m_txa_offset = reinterpret_cast<laddr_t>(request) % PAGE_SIZE;
	}

	STUB_ASSERT (m_txa_offset + size > USER_TXA_SIZE, "Big request");

	// TODO: В связи с новой логикой Memory txa лучше будет
	// представить в виде региона. Причем здесь хранить инстанцию.
	m_txa = new Memory(size, Memory::ALLOC);
	m_txa_access = RESOURCE_ACCESS_READ;
	if (!isSet(access, RESOURCE_CALL_READONLY)) {
		m_txa_access |= RESOURCE_ACCESS_WRITE;
	}

	if (map_request) {
		m_txa->Map(request, size);
	} else {
		m_txa->copyIn(0, request, size);
	}

	// TODO: В стек наверное надо устанавливать смещение в пространстве
	//	пользователя.
	setStack(m_txa_offset, size, access);

	// TODO: Выбросить эту ерунду нафиг
	if (!isSet(access, RESOURCE_CALL_READONLY) && !map_request) {
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

tick_t ResourceThread::getTimestamp() const
{
	return m_timestamp;
}

tick_t ResourceThread::getWakeupstamp() const
{
	return m_wakeupstamp;
}

void ResourceThread::setTimestamp(tick_t timestamp)
{
	m_timestamp = timestamp;
}

void ResourceThread::setProcess(ResourceProcess *process)
{
	m_process = process;
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

void ResourceThread::Sleep(timeout_t timeout)
{
	if (timeout == TIMEOUT_INFINITY) {
		m_wakeupstamp = TIMESTAMP_FUTURE;
	} else {
		m_wakeupstamp = StubGetCurrentClock() + timeout;
	}
}

void ResourceThread::Wait(Resource *resource, uint32_t event)
{
	STUB_ASSERT(resource == this, "Wait youself?");
	m_event_instance = createInstance(resource, event);
	
	// TODO: Здесь же можно поставить на ожидание в планировщике.
}

bool ResourceThread::isActive () const
{
	return false;
}

void ResourceThread::Run()
{
	delete m_event_instance;
	m_event_instance = 0;
	ScheduleLink.Unlink(this);
	
	StubTaskRun(m_task);
}

void ResourceThread::Activate()
{
	// Идею синхронных иннстанций я пока не выносил, поэтому после активации
	// 	инстанция, слушающая события, удаляется.
	delete m_event_instance;
	m_event_instance = 0;
	ScheduleLink.Unlink(this);
	
	Scheduler().addActiveThread(this);
}

bool ResourceThread::Deactivate()
{
	if (StubTaskDestroy(m_task)) {
		m_task = 0;
		return true;
	}

	return false;
}

void ResourceThread::Kill()
{
	event(RESOURCE_EVENT_THREAD_EXIT);

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

const PageInstance *ResourceThread::PageFault(laddr_t addr, uint32_t *access)
{
	if (addr == USER_MEMORY_BASE + RETMAGIC) {
		// Нить вышла корректно
		// Надо в отдельную функцию
		if (m_copyback_instance != 0) {
			// Доставляем результат (находясь в адресном пространстве этой нити!
			m_copyback_instance->copyIn(
				reinterpret_cast<void *>(USER_TXA_BASE + m_txa_offset),
				m_txa->getSize() - m_txa_offset);
			delete m_copyback_instance;
			m_copyback_instance = 0;
		}
		
		Kill();
		STUB_FATAL("Thread is killed.");
	}

	if (m_stack.inBounds(USER_STACK_BASE, addr)) {
		*access &= RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE;
		return m_stack.PageFault(addr - USER_STACK_BASE);
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
		const id_t id = this->id();
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

bool ResourceThread::createRequestArea(ResourceThread *caller,
	laddr_t offset, laddr_t size, uint32_t access)
{
	if (m_txa != 0) return false;
	
	m_txa = new Memory(offset + size, Memory::ALLOC);
	m_txa_offset = offset;
	m_txa_access = access;

	const id_t caller_id = (caller != 0) ? caller->id() : 0;
	
	StubStackFrame stack_frame;
	StubSetStackFrame(&stack_frame, caller_id, m_txa_offset, size, m_txa_access);
	m_stack.copyIn(USER_STACK_SIZE - sizeof(StubStackFrame),
		       &stack_frame, sizeof(StubStackFrame));

	return true;
}

bool ResourceThread::copyIn(laddr_t dst, const void *src, size_t size)
{
	if (m_stack.inBounds(USER_STACK_BASE, dst)) {
		return m_stack.copyIn(dst - USER_STACK_BASE, src, size);
	}

	if (m_txa != 0 && m_txa->inBounds(USER_TXA_BASE, dst)) {
		return m_txa->copyIn(dst - USER_TXA_BASE, src, size);
	}

	return m_process->copyIn(dst, src, size);
}

void ResourceThread::setCopyBack(ResourceThread *thread, laddr_t buffer)
{
	m_copyback_instance = new InstanceCopyBack(thread, buffer);
}

InstanceThread *ResourceThread::createInstance(Resource *resource, uint32_t event)
{
	return new InstanceThread(resource, event, this);
}

extern "C"
const PageInstance *CoreThreadPageFault (const Task *task, laddr_t addr, uint32_t *access)
{
	Core::ResourceThread *thread =
		reinterpret_cast<Core::ResourceThread *>(StubTaskGetThread(task));

	const PageInstance *instance = thread->PageFault(addr, access);

	return instance;
}
