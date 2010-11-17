//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"
#include "Resource.h"
#include "Memory.h"

namespace Core {

class InstanceProcess;

class Process : public Resource
{
private:
	laddr_t m_entry;
	List<InstanceProcess> m_instance_list;

	Memory m_pagetable;

	bool CheckRegionPlace (const Region *region, laddr_t base) const;
	laddr_t selectRegionBase (const Region *region, laddr_t ubase) const;

	/// Создаем инстанцию на ресурс
	InstanceProcess *createInstance(Resource *resource, uint32_t access, laddr_t base = 0) const;
	
public:
	explicit Process(laddr_t entry);
	virtual ~Process ();

	virtual Process *asProcess ();

	virtual ResourceThread *Call ();
	virtual int Attach(Resource *resource, uint32_t access, laddr_t base);
	virtual int Detach(Resource *resource);

	const PageInstance *PageFault (laddr_t addr, uint32_t *access);

	InstanceProcess *FindInstance(id_t id) const;

	int ModifyResource(id_t id, int param_id, const void *param, size_t param_size);

	bool copyIn(offset_t offset, const void *src, size_t size);
};

} // namespace Core;
