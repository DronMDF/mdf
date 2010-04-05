//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "CoreLocal.h"

#include "Instance.h"
#include "Resource.h"
#include "Region.h"

namespace Core {

Instance::Instance (Resource *resource, uint32_t access, uint32_t param)
	: m_resource(resource),
	  m_access(access),
	  m_addr(0),
	  ProcessLink(),
	  ResourceLink()
{
	if (const ResourceRegion *region = m_resource->asRegion()) {
		// Регионы используют парам в качестве адреса маппинга к процессу.
		m_addr = param;
		STUB_ASSERT ((m_addr - region->offset()) % PAGE_SIZE != 0, 
			     "Unaligned region base");
	}

	m_resource->addInstance(this);
}

Instance::~Instance()
{
	if (m_resource != 0) {
		m_resource->removeInstance(this);
	}
}

int Instance::Modify (int paramid, const void *param, size_t param_size)
{
	if (!isSet(m_access, RESOURCE_ACCESS_MODIFY))
		return ERROR_ACCESS;

	STUB_ASSERT(m_resource == 0, "no resource for instance");

	return m_resource->Modify(paramid, param, param_size);
}

int Instance::Info(int infoid, void *info, size_t *size) const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");

	if (!isSet(m_access, RESOURCE_ACCESS_INFO))
		return ERROR_ACCESS;

	if (infoid == RESOURCE_INFO_REGION_INSTANCE_ADDR) {
		if (m_resource->asRegion() == 0) {
			return ERROR_INVALIDPARAM;
		}
		
		// Адрес преобразовывается в юзерспейс
		const laddr_t uaddr = m_addr - USER_MEMORY_BASE;
		return StubInfoValue(info, size, &uaddr, sizeof(laddr_t));
	}
	
	return m_resource->Info(infoid, info, size);
}

ResourceThread *Instance::Call()
{
	if (!isSet(m_access, RESOURCE_ACCESS_CALL))
		return 0;

	return m_resource->Call();
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

uint32_t Instance::getAccess() const
{
	return m_access;
}

Resource *Instance::resource() const
{
	return m_resource;
}

laddr_t Instance::getAddr() const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");
	STUB_ASSERT(m_resource->asRegion() == 0, "getAddr from no region instance");
	return m_addr;
}

void Instance::setAddr(laddr_t addr)
{
	STUB_ASSERT(m_addr != 0, "region already Mapped");
	STUB_ASSERT(m_resource->asRegion() == 0, "setAddr from no region instance");
	m_addr = addr;
}

void Instance::event(uint32_t eid)
{
	if (eid == RESOURCE_EVENT_DESTROY) {
		STUB_ASSERT(ResourceLink.isLinked(), "Destroy from linked resource");
		m_resource = 0;
	}
}

} // namespace Core
