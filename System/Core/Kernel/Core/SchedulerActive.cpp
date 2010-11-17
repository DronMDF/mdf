//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/List.h"
#include "include/Link.h"
#include "include/Memory.h"
#include "include/SubScheduler.h"
#include "include/SchedulerActive.h"
#include "include/CoreLocal.h"

#include "include/Resource.h"
#include "include/Thread.h"

using namespace Core;

// Планировщик активыных нитей.
// Пока он оперирует одной очередью, для тестовых целей этого достаточно.
// Потом будет три очерди - высокоприоритетных, обычных, и пассивных нитей.

// Пассивные нити - это те нити, которые работают только тогда,
// когда других дел нет.

SchedulerActive::SchedulerActive()
	: m_queue(&Thread::ScheduleLink)
{
}

SchedulerActive::~SchedulerActive()
{
	STUB_ASSERT(m_queue.getSize() > 0, "Destroy full scheduler");
}

uint32_t SchedulerActive::getThreadIndex(const Thread *thread) const
{
	STUB_ASSERT(StubGetCurrentClock() < thread->getTimestamp(),
		"Thread timestamp in the future");

	// TODO: В качестве приоритета необходимо учитывать только приоритет
	// а не его класс который, возможно, будет закодирован в этом же значении.
	const uint64_t index = StubGetCurrentClock() -
		thread->getTimestamp() + thread->getPriority();

	return min(index, 0xffffffffU);
}

bool SchedulerActive::checkThreadUrgency(const Thread *thread,
		const Thread *exist) const
{
	STUB_ASSERT(StubGetCurrentClock() < thread->getTimestamp(),
		"New thread timestamp in the future");
	STUB_ASSERT(StubGetCurrentClock() < exist->getTimestamp(),
		"Exist thread timestamp in the future");

	const uint32_t tindex = getThreadIndex(thread);
	const uint32_t eindex = getThreadIndex(exist);

	if (tindex > eindex)
		return true;

	if (tindex == eindex) {
		if (thread->getPriority() > exist->getPriority())
			return true;
	}

// 	CorePrint(" %u+%u ", exist->getPriority(),
// 		uint32_t(StubGetCurrentClock() - exist->getTimestamp()));

	return false;
}

void SchedulerActive::addThread(Thread *thread)
{
	STUB_ASSERT(thread == 0, "thread invalid");

// 	CorePrint("%u Add %u+%u after ",
// 		uint32_t(StubGetCurrentClock()), thread->getPriority(),
// 		uint32_t(StubGetCurrentClock() - thread->getTimestamp()));

	addThreadOrdered(thread, &m_queue);

//	CorePrint("\n");
}

Thread *SchedulerActive::getThread()
{
	if (Thread *thread = m_queue.getFirst()) {
		m_queue.Remove(thread);
		return thread;
	}

	return 0;
}
