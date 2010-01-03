//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"
#include "Resource.h"
#include "Memory.h"

namespace Core {

class ResourceRegion;
class ResourceInstance;
class ResourceThread;

class ResourceProcess : public Resource
{
private:
	laddr_t m_entry;
	List<ResourceInstance> m_instance_list;

	Memory m_pagetable;

	bool CheckRegionPlace (const ResourceRegion *region, laddr_t base) const;
	laddr_t selectRegionBase (const ResourceRegion *region, uint32_t ubase) const;

public:
	explicit ResourceProcess (laddr_t entry);
	virtual ~ResourceProcess ();

	virtual ResourceProcess *asProcess ();

	virtual ResourceThread *Call ();
	virtual int Attach(Resource *resource, int access, uint32_t spec);
	virtual int Detach(Resource *resource);

	const PageInstance *PageFault (laddr_t addr, uint32_t *access);

	ResourceInstance *FindInstance (id_t id) const;

	int ModifyResource(id_t id, int param_id, const void *param, size_t param_size);
};

} // namespace Core;
