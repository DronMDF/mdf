//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "Kernel.h"
#include "Resource.h"
#include "InstanceThread.h"

using namespace Core;

InstanceThread::InstanceThread(Resource *resource, uint32_t event)
	: Instance(resource)
{
}
