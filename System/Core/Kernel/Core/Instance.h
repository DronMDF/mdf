//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

class Resource;
class ResourceThread;

class Instance {
	// Этото код остается в Instance
private:
	Resource *m_resource;	// TODO: Следящий ptr?
	uint32_t m_access;

private:
	Instance (const Instance &);
	Instance & operator = (const Instance &);

public:
	Instance(Resource *resource, uint32_t access, uint32_t param);
	~Instance();

	Resource *getResource() const;
	
	id_t id() const;
	uint32_t getAccess() const;

	// Ресурсные функции (они здесь для того, чтобы паралельно проверить доступ и границы)
	bool inBounds(laddr_t addr) const;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);
	
	// Этото код переходит в InstanceProcess
private:
	laddr_t m_addr;

public:
	Link<Instance> ProcessLink;

	laddr_t getAddr() const;
	void setAddr(laddr_t addr);

	int Modify (int paramid, const void *param, size_t param_size);
	int Info (int infoid, void *info, size_t *info_size) const;
	ResourceThread *Call();
};

} // namespace Core
