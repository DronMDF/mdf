//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Link.h"

namespace Core {

class Resource;

// Это класс вспомогательный, им оперирует ResourceStorage
class ResourceId
{
protected:
	id_t m_id;

	ResourceId();
	virtual ~ResourceId();

public:
	Link<ResourceId> StorageLink;

	void setId(id_t id);
	id_t id() const;

	virtual Resource *asResource() = 0;

	void Register();
};

} // namespace Core
