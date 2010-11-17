//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/List.h"
#include "include/Memory.h"
#include "include/Resource.h"
#include "include/Process.h"
#include "include/Thread.h"
#include "include/SubScheduler.h"
#include "include/SchedulerKill.h"

#include "include/CoreLocal.h"

using namespace Core;

SchedulerKill::SchedulerKill()
	: m_queue(&Thread::ScheduleLink)
{
}

SchedulerKill::~SchedulerKill()
{
	STUB_ASSERT(m_queue.getSize() > 0, "Destroy full scheduler");
}

void SchedulerKill::addThread(Thread *thread)
{
	m_queue.Insert(thread);
}

Thread *SchedulerKill::getThread()
{
	for (Thread *thread = m_queue.getFirst(); thread != 0; )
	{
		Thread *dthread = thread;
		thread = m_queue.getNext(thread);

		if (dthread->Deactivate()) {
			m_queue.Remove(dthread);
			// TODO: Можно было бы просто удалить нить, но инстанция
			//	подвиснет в процессе.
			STUB_ASSERT(dthread->getProcess()->Detach(dthread) != SUCCESS,
				    "Delete nonprocess thread");
		}
	}

	return 0;
}
