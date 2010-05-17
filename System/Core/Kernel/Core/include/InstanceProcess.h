//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"
#include "Link.h"

namespace Core {
	
class Resource;
class ResourceThread;

class InstanceProcess : public Instance {
public:
	Link<InstanceProcess> ProcessLink;
	
	InstanceProcess(Resource *resource, uint32_t access, laddr_t base);

	using Instance::addr;
	using Instance::setAddr;
	
	using Instance::inBounds;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);
	
	ResourceThread *Call();
	int Modify(int paramid, const void *param, size_t size);
	int Info(int infoid, void *info, size_t *size) const;

	virtual bool active() const;
};

} // namespace Core
