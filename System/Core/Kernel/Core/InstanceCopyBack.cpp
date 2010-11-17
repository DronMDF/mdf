//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/InstanceCopyBack.h"
#include "include/Resource.h"
#include "include/Thread.h"

using namespace Core;

InstanceCopyBack::InstanceCopyBack(Resource *caller, laddr_t dst)
	: Instance(caller, 0), m_dst(dst)
{
}

void InstanceCopyBack::copyIn(const void *src, size_t size) const
{
	if (Resource *resource = this->resource()) {
		if (Thread *thread = resource->asThread()) {
			thread->copyIn(m_dst, src, size);
		}
	}
}
