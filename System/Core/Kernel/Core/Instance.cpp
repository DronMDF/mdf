//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "CoreLocal.h"

#include "Instance.h"
#include "Resource.h"
#include "ResourceRegion.h"

namespace Core {

Instance::Instance (Resource *resource, uint32_t access)
	: m_resource(resource),
	  m_access(access),
	  m_addr(0),
	  ResourceLink()
{
	m_resource->addInstance(this);
}

Instance::~Instance()
{
	if (m_resource != 0) {
		m_resource->removeInstance(this);
	}
}

laddr_t Instance::addr() const
{
	STUB_ASSERT(m_resource == 0, "No resource for instance");
	STUB_ASSERT(m_resource->asRegion() == 0, "getAddr from no region instance");
	return m_addr;
}

void Instance::setAddr(laddr_t addr)
{
	STUB_ASSERT(m_addr != 0, "Adress already defined");
	STUB_ASSERT(m_resource == 0, "No resource");

	if (const ResourceRegion *region = m_resource->asRegion()) {
		m_addr = addr;
		STUB_ASSERT((m_addr - region->offset()) % PAGE_SIZE != 0,
			    "Unaligned region base");
	}
}

bool Instance::inBounds(laddr_t addr) const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");

	const ResourceRegion *region = m_resource->asRegion();
	if (region == 0)
		return false;	// Не регион

	if (m_addr == 0) {
		CorePrint ("Not mapped region\n");
		return false;	// Не замаплен
	}

	return region->inBounds(addr, m_addr);
}

const PageInstance *Instance::PageFault(laddr_t addr, uint32_t *access)
{
	ResourceRegion *region = m_resource->asRegion();
	STUB_ASSERT (region == 0, "Instance not for region");
	STUB_ASSERT (m_addr == 0, "Region not attached");
	STUB_ASSERT (addr < m_addr || m_addr + region->size() <= addr,
		"Addr out of region bounds");

	if ((m_access & *access) != *access)
		return 0;	// Нет соответствующего доступа

	return region->PageFault (addr - m_addr, access);
}

id_t Instance::id() const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");
	return m_resource->id();
}

Resource *Instance::resource() const
{
	return m_resource;
}

void Instance::event(uint32_t eid)
{
	if (eid == RESOURCE_EVENT_DESTROY) {
		STUB_ASSERT(ResourceLink.isLinked(), "Destroy from linked resource");
		m_resource = 0;
	}
}

bool Instance::allow(uint32_t want) const
{
	return isSet(m_access, want);
}

} // namespace Core