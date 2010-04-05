//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"

#include "List.h"
#include "Link.h"
#include "Memory.h"
#include "Resource.h"
#include "ResourceThread.h"
#include "Event.h"
#include "Scheduler.h"

namespace Core {

Event::Event()
	: m_observers(&ResourceThread::EventLink)
{
}

Event::~Event()
{
	Action(RESOURCE_EVENT_DESTROY);
}

void Event::addObserver(ResourceThread *thread, uint32_t event)
{
	// TODO: Необходимо сделать накопление событий, хотя бы однотипных.
	// и нить будет сразу же активироваться при попытке дождаться их.

	STUB_ASSERT(thread->EventLink.isLinked(), "Thread already wait event");

	thread->setEvent(event);
	m_observers.Insert(thread);
}

void Event::Action(uint32_t event)
{
	for (ResourceThread *thread = m_observers.getFirst(); thread != 0; )
	{
		ResourceThread *nthread = m_observers.getNext(thread);

		if (thread->getEvent() == event || event == RESOURCE_EVENT_DESTROY) {
			m_observers.Remove(thread);
			thread->setEvent(event);
			thread->Activate();
		}

		thread = nthread;
	}
}

} // namespace Core
