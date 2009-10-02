//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

class Resource;

// TODO: Блокировки пока оставим.

class ResourceStorage
{
private:
	static List<ResourceId> *m_resources_list;

	ResourceId *_Find(id_t id) const;

public:
	ResourceStorage();

	void Register(ResourceId *resource);
	void Unregister(ResourceId *resource);

	Resource *Find(id_t id);
};

} // namespace Core
