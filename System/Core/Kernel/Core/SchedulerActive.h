//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "SubScheduler.h"

namespace Core {

class SchedulerActive : public SubScheduler
{
private:
	// Очереди пока хватит одной, в окончательной версии сделаю три.
	ThreadList m_queue;

protected:
	uint32_t getThreadIndex(const ResourceThread *thread) const;

	// Возвращает true, если thread более приоритетен, нежели exist.
	// Название не очень удачное но другого что-то не придумал.
	virtual bool checkThreadUrgency(const ResourceThread *thread,
		const ResourceThread *exist) const;

public:
	SchedulerActive();
	virtual ~SchedulerActive();

	virtual void addThread(ResourceThread *thread);
	virtual ResourceThread *getThread();
};

} // namespace Core
