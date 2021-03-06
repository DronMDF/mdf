//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/List.h"
#include "include/Memory.h"
#include "include/Resource.h"
#include "include/Scheduler.h"
#include "include/SubScheduler.h"
#include "include/SchedulerActive.h"
#include "include/SchedulerInactive.h"
#include "include/SchedulerKill.h"

using namespace Core;

namespace Core {
	class Thread;
}

// -----------------------------------------------------------------------------
SubScheduler *Scheduler::m_actives = 0;
SubScheduler *Scheduler::m_inactives = 0;
SubScheduler *Scheduler::m_killed = 0;

Scheduler::Scheduler()
{
}

Scheduler::~Scheduler()
{
}

// Эти методы очень похожи, просто оперируют разными очередями.
void Scheduler::addActiveThread(Thread *thread)
{
	if (m_actives == 0) {
		m_actives = new SchedulerActive();
	}

	m_actives->addThread(thread);
}

void Scheduler::addInactiveThread(Thread *thread)
{
	if (m_inactives == 0) {
		m_inactives = new SchedulerInactive();
	}

	m_inactives->addThread(thread);
}

void Scheduler::addKillThread(Thread *thread)
{
	if (m_killed == 0) {
		m_killed = new SchedulerKill();
	}

	m_killed->addThread(thread);
}

Thread *Scheduler::getThread()
{
	if (m_killed != 0) {
		// Из этого планировщика нити не возвращаются никогда!
		STUB_ASSERT(m_killed->getThread() != 0, "Thread from graveyard!");
	}

	if (m_inactives != 0) {
		// Из этого планировщика нити перекачиваются в active
		while (Thread *thread = m_inactives->getThread()) {
			addActiveThread(thread);
		}
	}

	if (m_actives != 0) {
		// Из этого просто выдаем то, что есть.
		return m_actives->getThread();
	}

	return 0;
}
