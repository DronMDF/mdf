//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"
#include "Link.h"

namespace Core {
	
class Resource;

class InstanceProcess : public Instance {
public:
	Link<InstanceProcess> ProcessLink;
	
	InstanceProcess(Resource *resource, uint32_t access, uint32_t base = 0);

	using Instance::getAddr;
	using Instance::setAddr;

	using Instance::Modify;
	using Instance::Info;
	using Instance::Call;
};

} // namespace Core
