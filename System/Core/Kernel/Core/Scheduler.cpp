//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "List.h"
#include "Memory.h"
#include "Resource.h"
// Этому исходнику достаточно forward decls, инклюд только ради зависимости.
#include "Thread.h"
#include "Scheduler.h"
#include "SubScheduler.h"
#include "SchedulerActive.h"
#include "SchedulerInactive.h"
#include "KillScheduler.h"

namespace Core {

class ResourceThread;

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
void Scheduler::addActiveThread(ResourceThread *thread)
{
	if (m_actives == 0) {
		m_actives = new SchedulerActive();
	}

	m_actives->addThread(thread);
}

void Scheduler::addInactiveThread(ResourceThread *thread)
{
	if (m_inactives == 0) {
		m_inactives = new SchedulerInactive();
	}

	m_inactives->addThread(thread);
}

void Scheduler::addKillThread(ResourceThread *thread)
{
	if (m_killed == 0) {
		m_killed = new KillScheduler();
	}

	m_killed->addThread(thread);
}

ResourceThread *Scheduler::getThread()
{
	if (m_killed != 0) {
		// Из этого планировщика нити не возвращаются никогда!
		STUB_ASSERT(m_killed->getThread() != 0, "Thread from graveyard!");
	}

	if (m_inactives != 0) {
		// Из этого планировщика нити перекачиваются в active
		while (ResourceThread *thread = m_inactives->getThread()) {
			addActiveThread(thread);
		}
	}

	if (m_actives != 0) {
		// Из этого просто выдаем то, что есть.
		return m_actives->getThread();
	}

	return 0;
}

} // namespace Core
