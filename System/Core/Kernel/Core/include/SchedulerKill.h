//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

class Thread;

class SchedulerKill : public SubScheduler
{
protected:
	ThreadList m_queue;

public:
	SchedulerKill();
	virtual ~SchedulerKill();

	virtual void addThread(Thread *thread);
	virtual Thread *getThread();
};

} // namespace Core
