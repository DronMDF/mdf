//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"

namespace Core {
	
class Resource;

class InstanceThread : public Instance {
public:
	InstanceThread(Resource *resource, uint32_t event);
};

}
