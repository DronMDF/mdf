//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

class ResourceId;
class Resource;

// TODO: Блокировки пока оставим.

class Storage
{
private:
	static List<ResourceId> *m_resources_list;

	ResourceId *_Find(id_t id) const;

public:
	Storage();

	void Register(ResourceId *resource);
	void Unregister(ResourceId *resource);

	Resource *Find(id_t id);
};

} // namespace Core
