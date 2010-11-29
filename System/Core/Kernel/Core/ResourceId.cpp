//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/ResourceId.h"
#include "include/Storage.h"

using namespace Core;

ResourceId::ResourceId()
	: m_id(0), StorageLink()
{
}

ResourceId::~ResourceId()
{
	if (m_id != 0) {
		Storage().Unregister(this);
	}
}

void ResourceId::Register()
{
	STUB_ASSERT(m_id != 0, "Resource already registered");
	Storage().Register(this);
}

void ResourceId::setId(id_t id)
{
	m_id = id;
}

id_t ResourceId::id (void) const
{
	STUB_ASSERT(m_id == 0, "Resource not registered");
	return m_id;
}
