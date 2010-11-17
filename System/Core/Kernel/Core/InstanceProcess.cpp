//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/Resource.h"
#include "include/Region.h"
#include "include/InstanceProcess.h"

using namespace Core;

InstanceProcess::InstanceProcess(Resource *resource, uint32_t access, laddr_t base)
	: Instance(resource, access), m_addr(base), ProcessLink()
{
}

ResourceThread *InstanceProcess::Call()
{
	if (!allow(RESOURCE_ACCESS_CALL)) return 0;
	return resource()->Call();
}

int InstanceProcess::Modify(int paramid, const void *param, size_t size)
{
	if (!allow(RESOURCE_ACCESS_MODIFY)) return ERROR_ACCESS;

	Resource *resource = this->resource();
	if (resource == 0) return ERROR_INVALIDPARAM;
	
	return resource->Modify(paramid, param, size);
}

int InstanceProcess::Info(int infoid, void *info, size_t *size) const
{
	if (!allow(RESOURCE_ACCESS_INFO)) return ERROR_ACCESS;

	Resource *resource = this->resource();
	if (resource == 0) return ERROR_INVALIDPARAM;

	if (infoid == RESOURCE_INFO_REGION_INSTANCE_ADDR) {
		if (resource->asRegion() == 0) return ERROR_INVALIDPARAM;

		// Адрес преобразовывается в юзерспейс
		const laddr_t uaddr = m_addr - USER_MEMORY_BASE;
		return StubInfoValue(info, size, &uaddr, sizeof(laddr_t));
	}

	// TODO: От родительской инстанции никакая информация не требуется?
	return resource->Info(infoid, info, size);
}

bool InstanceProcess::active() const
{
	return true;
}

const PageInstance *InstanceProcess::PageFault(laddr_t addr, uint32_t *access)
{
	if (resource() == 0) return 0;
	
	Region *region = resource()->asRegion();
	if (region == 0) return 0;
	if (!allow(*access)) {
		*access &= getAccess();
		return 0;
	}
	if (m_addr == 0) return 0;
	if (!inBounds(addr)) return 0;

	return region->PageFault(addr - m_addr, access);
}

bool InstanceProcess::inBounds(laddr_t addr) const
{
	// TODO: А что эта функция делает в Process???
	if (resource() == 0) return false;

	const Region *region = resource()->asRegion();
	if (region == 0) return false;	// Не регион
	if (m_addr == 0) return false;	// Не замаплен
		
	return region->inBounds(addr, m_addr);
}

laddr_t InstanceProcess::addr() const
{
	STUB_ASSERT(resource() == 0, "No resource for instance");
	STUB_ASSERT(resource()->asRegion() == 0, "getAddr from no region instance");
	return m_addr;
}

void InstanceProcess::setAddr(laddr_t addr)
{
	STUB_ASSERT(m_addr != 0, "Adress already defined");
	if (resource() == 0) return;
	
	if (const Region *region = resource()->asRegion()) {
		m_addr = addr;
		STUB_ASSERT((m_addr - region->offset()) % PAGE_SIZE != 0,
			    "Unaligned region base");
	}
}
