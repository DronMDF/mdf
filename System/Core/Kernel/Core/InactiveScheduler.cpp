//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "List.h"
#include "Link.h"
#include "Resource.h"
#include "Memory.h"
#include "Thread.h"
#include "SubScheduler.h"
#include "InactiveScheduler.h"

namespace Core {

InactiveScheduler::InactiveScheduler()
	: m_imminent(&ResourceThread::ScheduleLink),
	  m_infinity(&ResourceThread::ScheduleLink)
{
}

InactiveScheduler::~InactiveScheduler()
{
	STUB_ASSERT(m_imminent.getSize() > 0, "Destroy full scheduler");
	STUB_ASSERT(m_infinity.getSize() > 0, "Destroy full scheduler");
}

bool InactiveScheduler::checkThreadUrgency(const ResourceThread *thread,
		const ResourceThread *exist) const
{
	if (thread->getWakeupstamp() <= exist->getWakeupstamp())
		return true;

	return false;
}

void InactiveScheduler::addThread(ResourceThread *thread)
{
	STUB_ASSERT(thread == 0, "thread invalid");

	if (thread->getWakeupstamp() == CLOCK_MAX) {
		m_infinity.Insert(thread);
		return;
	}

	addThreadOrdered(thread, &m_imminent);
}

ResourceThread *InactiveScheduler::getThread()
{
	if (ResourceThread *thread = m_imminent.getFirst()) {
		if (thread->getWakeupstamp() <= StubGetCurrentClock()) {
			m_imminent.Remove(thread);
			return thread;
		}
	}

	return 0;
}

} // namespace Core
