//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Memory.h"
#include "Resource.h"

namespace Core {

class InstanceThread;
class InstanceCopyBack;

class Thread : public Resource
{
private:
	Process *m_process;

protected:
	Task *m_task;

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
	tick_t m_timestamp;
	tick_t m_wakeupstamp;

	uint32_t m_priority;

	laddr_t m_entry;

	Instance *m_event_instance;
	InstanceCopyBack *m_copyback_instance;

private:
	Thread (const Thread &);
	Thread & operator = (const Thread &);

	void setStack(offset_t request, size_t request_size, uint32_t access);

	InstanceThread *createInstance(Resource *resource, uint32_t event);
	
protected:
	explicit Thread(Process *process = 0);

	virtual void Kill();

public:
	Link<Thread> ScheduleLink;

	Thread(Process *process, laddr_t entry);
	virtual ~Thread();

	virtual Thread *asThread ();

	void setRequest(const void *request, size_t size, uint32_t access);

	uint32_t getPriority () const;
	void setPriority (uint32_t priority);

	tick_t getTimestamp() const;
	void setTimestamp(tick_t timestamp);
	tick_t getWakeupstamp() const;

	laddr_t getEntry() const;

	void setProcess(Process *process);
	Process *getProcess() const;

	void Sleep(timeout_t timeout);
	void Wait(Resource *resource, uint32_t event);

	bool isActive () const;

	virtual void Run();

	virtual void Activate();
	virtual bool Deactivate();

	const PageInstance *PageFault (laddr_t addr, uint32_t *access);

	virtual Thread *Call ();

	virtual int Modify(int param_id, const void *param, size_t param_size);
	virtual int Info (int info_id, void *info, size_t *info_size) const;

	bool createRequestArea(Thread *caller,
			       laddr_t offset, laddr_t size, uint32_t access);
	virtual bool copyIn(laddr_t dst, const void *src, size_t size);

	virtual void setCopyBack(Thread *thread, laddr_t buffer);
};

} // namespace Core
