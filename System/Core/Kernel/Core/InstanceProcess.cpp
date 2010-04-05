//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "Resource.h"
#include "InstanceProcess.h"

using namespace Core;

InstanceProcess::InstanceProcess(Resource *resource, uint32_t access, uint32_t base)
	: Instance(resource, access, base),
	  ProcessLink()
{
}

ResourceThread *InstanceProcess::Call()
{
	if (!allow(RESOURCE_ACCESS_CALL)) {
		return 0;
	}

	return resource()->Call();
}
