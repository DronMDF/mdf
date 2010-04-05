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
class ResourceThread;
class InstanceProcess;

class ResourceProcess : public Resource
{
private:
	laddr_t m_entry;
	List<Instance> m_instance_list;

	Memory m_pagetable;

	bool CheckRegionPlace (const ResourceRegion *region, laddr_t base) const;
	laddr_t selectRegionBase (const ResourceRegion *region, uint32_t ubase) const;

	/// Создаем инстанцию на ресурс
	InstanceProcess *createInstance(Resource *resource, uint32_t access, uint32_t param = 0) const;
	
public:
	explicit ResourceProcess (laddr_t entry);
	virtual ~ResourceProcess ();

	virtual ResourceProcess *asProcess ();

	virtual ResourceThread *Call ();
	virtual int Attach(Resource *resource, uint32_t access, uint32_t spec);
	virtual int Detach(Resource *resource);

	const PageInstance *PageFault (laddr_t addr, uint32_t *access);

	Instance *FindInstance (id_t id) const;

	int ModifyResource(id_t id, int param_id, const void *param, size_t param_size);

	bool copyIn(offset_t offset, const void *src, size_t size);
};

} // namespace Core;
