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
	
	InstanceProcess(Resource *resource, uint32_t access, uint32_t base = 0);

	using Instance::getAddr;
	using Instance::setAddr;

	using Instance::Modify;
	int Info(int infoid, void *info, size_t *size) const;
	ResourceThread *Call();
};

} // namespace Core
