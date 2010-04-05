//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "InstanceProcess.h"

using namespace Core;

InstanceProcess::InstanceProcess(Resource *resource, uint32_t access, uint32_t base)
	: Instance(resource, access, base),
	  ProcessLink()
{
}

