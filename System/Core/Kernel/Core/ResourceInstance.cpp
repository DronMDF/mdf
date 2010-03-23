//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "CoreLocal.h"

#include "ResourceInstance.h"
#include "Resource.h"
#include "Region.h"

namespace Core {

ResourceInstance::ResourceInstance (Resource *resource, uint32_t access, uint32_t param)
	: m_resource(resource),
	  m_access(access),
	  m_addr(0),
	  ProcessLink()
{
	if (m_resource->asRegion()) {
		// Регионы используют парам в качестве адреса маппинга к процессу.
		m_addr = param;
		STUB_ASSERT (m_addr % PAGE_SIZE != 0, "Unaligned region base");
	}
}

ResourceInstance::~ResourceInstance()
{
	if (m_resource != 0)
		m_resource->DeleteInstance();
}

int ResourceInstance::Modify (int paramid, const void *param, size_t param_size)
{
	if (!isSet(m_access, RESOURCE_ACCESS_MODIFY))
		return ERROR_ACCESS;

	STUB_ASSERT(m_resource == 0, "no resource for instance");

	return m_resource->Modify(paramid, param, param_size);
}

int ResourceInstance::Info (int infoid, void *info, size_t *size) const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");

	if (!isSet(m_access, RESOURCE_ACCESS_INFO))
		return ERROR_ACCESS;

	switch (infoid) {
		case RESOURCE_INFO_REGION_INSTANCE_ADDR:
			if (m_resource->asRegion() != 0) {
				const ResourceRegion *region = m_resource->asRegion();

				// Адрес преобразовывается в юзерспейс
				const laddr_t uaddr = m_addr + region->offset() - USER_MEMORY_BASE;
				return StubInfoValue(info, size, &uaddr, sizeof(laddr_t));
			}

			return ERROR_INVALIDPARAM;

		default:
			break;
	}

	return m_resource->Info(infoid, info, size);
}

ResourceThread *ResourceInstance::Call()
{
	if (!isSet(m_access, RESOURCE_ACCESS_CALL))
		return 0;

	return m_resource->Call();
}

bool ResourceInstance::inBounds(laddr_t addr) const
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

const PageInstance *ResourceInstance::PageFault(laddr_t addr, uint32_t *access)
{
	ResourceRegion *region = m_resource->asRegion();
	STUB_ASSERT (region == 0, "Instance not for region");
	STUB_ASSERT (m_addr == 0, "Region not attached");
	STUB_ASSERT (addr < m_addr + region->offset() ||
		m_addr + region->offset() + region->size() <= addr,
		"Addr out of region bounds");

	if ((m_access & *access) != *access)
		return 0;	// Нет соответствующего доступа

	return region->PageFault (addr - m_addr, access);
}

id_t ResourceInstance::getId() const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");
	return m_resource->getId();
}

uint32_t ResourceInstance::getAccess() const
{
	return m_access;
}

Resource *ResourceInstance::getResource() const
{
	return m_resource;
}

laddr_t ResourceInstance::getAddr() const
{
	STUB_ASSERT(m_resource == 0, "no resource for instance");
	STUB_ASSERT(m_resource->asRegion() == 0, "getAddr from no region instance");
	return m_addr;
}

void ResourceInstance::setAddr(laddr_t addr)
{
	STUB_ASSERT(m_addr != 0, "region already Mapped");
	STUB_ASSERT(m_resource->asRegion() == 0, "setAddr from no region instance");
	m_addr = addr;
}

} // namespace Core
