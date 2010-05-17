//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/Resource.h"
#include "include/ResourceRegion.h"
#include "include/InstanceProcess.h"

using namespace Core;

InstanceProcess::InstanceProcess(Resource *resource, uint32_t access, laddr_t base)
	: Instance(resource, access),
	  ProcessLink()
{
	setAddr(base);
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
		const laddr_t uaddr = addr() - USER_MEMORY_BASE;
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
	
	ResourceRegion *region = resource()->asRegion();
	if (region == 0) return 0;
	if (!allow(*access)) return 0;
	if (this->addr() == 0) return 0;
	if (!inBounds(addr)) return 0;

	return region->PageFault(addr - this->addr(), access);
}



