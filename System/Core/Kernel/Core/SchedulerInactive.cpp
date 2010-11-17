//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/List.h"
#include "include/Link.h"
#include "include/Memory.h"
#include "include/Resource.h"
#include "include/Thread.h"
#include "include/SubScheduler.h"
#include "include/SchedulerInactive.h"

using namespace Core;

SchedulerInactive::SchedulerInactive()
	: m_imminent(&Thread::ScheduleLink),
	  m_infinity(&Thread::ScheduleLink)
{
}

SchedulerInactive::~SchedulerInactive()
{
	STUB_ASSERT(m_imminent.getSize() > 0, "Destroy full scheduler");
	STUB_ASSERT(m_infinity.getSize() > 0, "Destroy full scheduler");
}

bool SchedulerInactive::checkThreadUrgency(const Thread *thread,
		const Thread *exist) const
{
	if (thread->getWakeupstamp() <= exist->getWakeupstamp())
		return true;

	return false;
}

void SchedulerInactive::addThread(Thread *thread)
{
	STUB_ASSERT(thread == 0, "thread invalid");

	if (thread->getWakeupstamp() == TIMESTAMP_FUTURE) {
		m_infinity.Insert(thread);
		return;
	}

	addThreadOrdered(thread, &m_imminent);
}

Thread *SchedulerInactive::getThread()
{
	if (Thread *thread = m_imminent.getFirst()) {
		if (thread->getWakeupstamp() <= StubGetCurrentClock()) {
			m_imminent.Remove(thread);
			return thread;
		}
	}

	return 0;
}
