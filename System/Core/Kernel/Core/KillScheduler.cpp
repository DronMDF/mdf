//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "List.h"
#include "Memory.h"
#include "Resource.h"
#include "Thread.h"
#include "Process.h"
#include "SubScheduler.h"
#include "KillScheduler.h"

#include "CoreLocal.h"

namespace Core {

KillScheduler::KillScheduler()
	: m_queue(&ResourceThread::ScheduleLink)
{
}

KillScheduler::~KillScheduler()
{
	STUB_ASSERT(m_queue.getSize() > 0, "Destroy full scheduler");
}

void KillScheduler::addThread(ResourceThread *thread)
{
	m_queue.Insert(thread);
}

ResourceThread *KillScheduler::getThread()
{
	for (ResourceThread *thread = m_queue.getFirst(); thread != 0; )
	{
		ResourceThread *dthread = thread;
		thread = m_queue.getNext(thread);

		if (dthread->Deactivate()) {
			m_queue.Remove(dthread);
			CorePrint ("ThreadPtr 0x%08x remove from kill queue\n", dthread);
			dthread->getProcess()->Detach(dthread);
			CorePrint ("ThreadPtr 0x%08x a finish and die\n", dthread);
		}
	}

	return 0;
}

} // namespace Core
