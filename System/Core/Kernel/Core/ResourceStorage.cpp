//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "include/List.h"
#include "include/ResourceId.h"
#include "include/Storage.h"
#include "include/CoreLocal.h"

using namespace Core;

List<ResourceId> *Storage::m_resources_list = 0;

Storage::Storage()
{
	if (m_resources_list == 0) {
		m_resources_list = new List<ResourceId>(&ResourceId::StorageLink);
	}
}

ResourceId *Storage::_Find(id_t id) const
{
	for (ResourceId *resource = m_resources_list->getFirst();
	     resource != 0; resource = m_resources_list->getNext(resource))
	{
		if (resource->id() == id)
			return resource;
	}

	return 0;
}

void Storage::Register(ResourceId *resource)
{
	// Это весьма прямолинейный способ, но зато он на 100% уникален :)
	while (true) {
		id_t id = CoreRandom();
		if (_Find(id) == 0) {
			resource->setId(id);
			break;
		}
	}

	m_resources_list->Insert(resource);
}

void Storage::Unregister(ResourceId *resource)
{
	m_resources_list->Remove(resource);
}

Resource *Storage::Find(id_t id)
{
	if (ResourceId *rid = _Find(id)) {
		return rid->asResource();
	}

	return 0;
}
