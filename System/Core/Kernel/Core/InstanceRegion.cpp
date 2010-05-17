//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/InstanceRegion.h"

using namespace Core;

InstanceRegion::InstanceRegion(Resource *resource, uint32_t access, 
	       offset_t poffset, size_t psize, offset_t skip)
	: Instance(resource, access)
{
}

const PageInstance *InstanceRegion::PageFault(laddr_t addr, uint32_t *access)
{
	return 0;
}
