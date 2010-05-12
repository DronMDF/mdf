//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Instance.h"

namespace Core {
	
class Resource;

class InstanceRegion : public Instance
{
public:
	InstanceRegion(Resource *resource, uint32_t access, 
		       offset_t poffset, size_t psize, offset_t skip);
};

} // namespace Core
