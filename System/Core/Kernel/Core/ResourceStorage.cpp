//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include "List.h"
#include "ResourceId.h"
#include "ResourceStorage.h"
#include "CoreLocal.h"

namespace Core {

List<ResourceId> *ResourceStorage::m_resources_list = 0;

ResourceStorage::ResourceStorage()
{
	if (m_resources_list == 0) {
		m_resources_list = new List<ResourceId>(&ResourceId::StorageLink);
	}
}

ResourceId *ResourceStorage::_Find(id_t id) const
{
	for (ResourceId *resource = m_resources_list->getFirst();
	     resource != 0; resource = m_resources_list->getNext(resource))
	{
		if (resource->getId() == id)
			return resource;
	}

	return 0;
}

void ResourceStorage::Register(ResourceId *resource)
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

void ResourceStorage::Unregister(ResourceId *resource)
{
	m_resources_list->Remove(resource);
}

Resource *ResourceStorage::Find(id_t id)
{
	if (ResourceId *rid = _Find(id)) {
		return rid->asResource();
	}

	return 0;
}

} // namespace Core