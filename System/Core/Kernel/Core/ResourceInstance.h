//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

class Resource;
class ResourceThread;

class ResourceInstance
{
private:
	Resource *m_resource;
	uint32_t m_access;
	laddr_t m_addr;

public:
	Link<ResourceInstance> ProcessLink;

private:
	ResourceInstance (const ResourceInstance &);
	ResourceInstance & operator = (const ResourceInstance &);

public:
	ResourceInstance(Resource *resource, uint32_t access, uint32_t param);
	~ResourceInstance();

	Resource *getResource() const;
	id_t getId() const;
	uint32_t getAccess() const;

	laddr_t getAddr() const;
	void setAddr(laddr_t addr);

	int Modify (int paramid, const void *param, size_t param_size);
	int Info (int infoid, void *info, size_t *info_size) const;
	ResourceThread *Call();

	// Ресурсные функции (они здесь для того, чтобы паралельно проверить доступ и границы)
	bool inBounds(laddr_t addr) const;
	const PageInstance *PageFault(laddr_t addr, uint32_t *access);
};

} // namespace Core
