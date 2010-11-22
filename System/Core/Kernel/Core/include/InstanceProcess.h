//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"
#include "Link.h"

namespace Core {
	
class Resource;
class Thread;

class InstanceProcess : public Instance {
private:
	laddr_t m_addr;
	
public:
	Link<InstanceProcess> ProcessLink;
	
	InstanceProcess(Resource *resource, uint32_t access, laddr_t base);

	laddr_t addr() const;
	void setAddr(laddr_t addr);
	
	bool inBounds(laddr_t addr) const;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);
	
	Thread *Call();
	int Modify(int paramid, const void *param, size_t size);
	int Info(int infoid, void *info, size_t *size) const;

	virtual bool active() const;
};

} // namespace Core
