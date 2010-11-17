//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "SubScheduler.h"

namespace Core {

class SchedulerInactive : public SubScheduler
{
private:
	ThreadList m_imminent;

protected:
	// TODO: Надо сделать возможночть отвязывать объект от очереди через Link
	//	Тогда эту очередь в приват.
	ThreadList m_infinity;

	virtual bool checkThreadUrgency(const Thread *thread,
		const Thread *exist) const;

public:
	SchedulerInactive();
	virtual ~SchedulerInactive();

	virtual void addThread(Thread *thread);
	virtual Thread *getThread();
};

} // namespace Core
