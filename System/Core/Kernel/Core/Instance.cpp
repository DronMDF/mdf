//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/Kernel.h"
#include "include/CoreLocal.h"

#include "include/Instance.h"
#include "include/Resource.h"
#include "include/Region.h"

using namespace Core;

Instance::Instance (Resource *resource, uint32_t access)
	: m_resource(resource),
	  m_access(access),
	  m_addr(0),
	  ResourceLink()
{
	// Конечно странно, что инстанция конструируется без ресурса. Но в
	// тестах будет полезно (а может быть мигрирующие инстанции изобретуться)
	if (m_resource) {
		m_resource->addInstance(this);
	}
}

Instance::~Instance()
{
	if (m_resource != 0) {
		m_resource->removeInstance(this);
		m_resource = 0;
	}
}

id_t Instance::id() const
{
	return m_resource != 0 ? m_resource->id() : INVALID_ID;
}

Resource *Instance::resource() const
{
	return m_resource;
}

void Instance::event(uint32_t eid)
{
	STUB_ASSERT(m_resource == 0, "No resource for instance");
	if (eid == RESOURCE_EVENT_DESTROY) {
		// События исходят от самого ресурса. нет смысла пытаться отвязаться
		// Но отлинковаться надо
		ResourceLink.Unlink(this);
		m_resource = 0;
	}
}

bool Instance::allow(uint32_t access) const
{
	return isSet(m_access, access);
}

bool Instance::active() const
{
	return false;
}
