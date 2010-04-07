//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Memory.h"
#include "Resource.h"

namespace Core {

class ResourceProcess;

class ResourceThread : public Resource
{
private:
	ResourceProcess *m_process;

protected:
	Task *m_task;

	// TODO: Размер обратного копирования необходимо хранить. потому что
	//	форвард может изменить размер запроса, но копибек останется
	//	неизменным.

	// TODO: Еще открыт вопрос о том, стоит ли доставлять блок в процесс,
	//	если вызывающая нить уже умерла? тогда надо хранить и pid.

	// Параметры обратного копирования TXA;
	id_t m_copyback_id;
	laddr_t m_copyback_addr;
	
private:
	// TODO: Здесь надо ввести новый тип, основанный на мемори, но иначе
	//	выделяющий память. сверху вниз, а не всю сразу, как memory.
	//	(Имеется ввиду память для указателей на сраничные инстанции).
	Memory	m_stack;

protected:
	Memory	*m_txa;	// thread exchange area (опциональная)
	offset_t m_txa_offset;
	uint32_t m_txa_access;

private:
	clock_t m_timestamp;
	clock_t m_wakeupstamp;

	uint32_t m_priority;

	laddr_t m_entry;

	uint32_t m_event;

private:
	ResourceThread (const ResourceThread &);
	ResourceThread & operator = (const ResourceThread &);

	void setStack(offset_t request, size_t request_size, uint32_t access);

protected:
	explicit ResourceThread (ResourceProcess *process = 0);

	virtual void Kill();

public:
	Link<ResourceThread> ScheduleLink;
	Link<ResourceThread> EventLink;

	ResourceThread (ResourceProcess *process, laddr_t entry);
	virtual ~ResourceThread();

	virtual ResourceThread *asThread ();

	void setRequest(const void *request, size_t size, uint32_t access);

	uint32_t getPriority () const;
	void setPriority (uint32_t priority);

	clock_t getTimestamp() const;
	void setTimestamp(clock_t timestamp);
	clock_t getWakeupstamp() const;

	laddr_t getEntry() const;

	void setProcess(ResourceProcess *process);
	ResourceProcess *getProcess() const;

	void Sleep(timeout_t timeout);
	void Wait (Resource *resource, int event);

	bool isActive () const;

	virtual void Run();

	virtual void Activate();
	virtual bool Deactivate();

	const PageInstance *PageFault (laddr_t addr, uint32_t *access);

	virtual ResourceThread *Call ();

	virtual int Modify(int param_id, const void *param, size_t param_size);
	virtual int Info (int info_id, void *info, size_t *info_size) const;

	void setEvent(uint32_t event);
	uint32_t getEvent() const;

	bool createRequestArea(ResourceThread *caller,
			       laddr_t offset, laddr_t size, uint32_t access);
	bool copyIn(laddr_t dst, const void *src, size_t size);

	virtual void setCopyBack(ResourceThread *thread, laddr_t buffer);
};

} // namespace Core
