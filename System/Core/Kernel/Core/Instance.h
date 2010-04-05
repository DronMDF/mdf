//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

class Resource;
class ResourceThread;
class InstanceProcess;	// Временно, пока ProcessLink не перенесу.

class Instance {
	// Этото код остается в Instance
private:
	Resource *m_resource;	// TODO: Следящий ptr?
	uint32_t m_access;

private:
	Instance (const Instance &);
	Instance & operator = (const Instance &);

public:
	Link<Instance> ResourceLink;
	
	Instance(Resource *resource, uint32_t access, uint32_t param);
	virtual ~Instance();

	Resource *resource() const;
	
	id_t id() const;
	uint32_t getAccess() const;

	// Ресурсные функции (они здесь для того, чтобы паралельно проверить доступ и границы)
	bool inBounds(laddr_t addr) const;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);

	virtual void event(uint32_t eid);

	bool allow(uint32_t want) const;
	
	// Этото код переходит в InstanceProcess
protected:
	laddr_t m_addr;

	laddr_t getAddr() const;
	void setAddr(laddr_t addr);

	int Modify (int paramid, const void *param, size_t param_size);
};

} // namespace Core
