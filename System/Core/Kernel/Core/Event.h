//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"
#include "Thread.h"

namespace Core {

class Event {
private:
	List<ResourceThread> m_observers;

public:
	Event();
	virtual ~Event();

	virtual void addObserver(ResourceThread *thread, uint32_t event);
	virtual void Action(uint32_t event);
};

} // namespace Core
