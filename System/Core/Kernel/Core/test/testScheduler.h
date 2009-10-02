//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "../Scheduler.h"
#include "../SubScheduler.h"

class testScheduler : public Core::Scheduler
{
public:
	using Scheduler::m_actives;
	using Scheduler::m_inactives;
	using Scheduler::m_killed;

	virtual ~testScheduler() {
		delete m_actives;
		delete m_inactives;
		delete m_killed;
	}
};
