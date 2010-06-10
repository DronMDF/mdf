//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/InstanceRegion.h"
#include "include/ResourceRegion.h"

using namespace Core;

InstanceRegion::InstanceRegion(Resource *resource, uint32_t access, 
	       offset_t poffset, size_t psize, offset_t skip)
	: Instance(resource, access), m_position(skip), m_offset(poffset), m_size(psize)
{ }

bool InstanceRegion::inBounds(laddr_t addr) const
{
	return (addr >= m_position) && (addr < m_position + m_size);
}

const PageInstance *InstanceRegion::PageFault(laddr_t addr, uint32_t *access)
{
	if (!allow(*access)) {
		*access &= getAccess();
		return 0;
	}
	
	STUB_ASSERT(addr < m_position || addr >= m_position + m_size, "Check bounds first");
	
	if (Resource *res = resource()) {
		if (ResourceRegion *region = res->asRegion()) {
			return region->PageFault(addr - m_position + m_offset, access);
		}
	}
	
	STUB_FATAL("Invalid pagefault");
}
