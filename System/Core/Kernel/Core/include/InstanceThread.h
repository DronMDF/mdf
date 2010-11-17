//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"

namespace Core {
	
class Resource;
class Thread;

class InstanceThread : public Instance {
private:
	uint32_t m_event;
	Thread *m_thread;

	InstanceThread(const InstanceThread &);
	InstanceThread &operator =(const InstanceThread &);

public:
	InstanceThread(Resource *resource, uint32_t event, Thread *thread);

	virtual void event(uint32_t eid);
};

}
