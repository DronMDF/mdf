//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
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
		m_actives = 0;
		delete m_inactives;
		m_inactives = 0;
		delete m_killed;
		m_killed = 0;
	}
};
