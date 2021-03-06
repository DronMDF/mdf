//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "List.h"

namespace Core {

class Resource;
class Thread;

class Instance {
private:
	Resource *m_resource;
	uint32_t m_access;
	laddr_t m_addr;

private:
	Instance (const Instance &);
	Instance & operator = (const Instance &);

protected:
	laddr_t addr() const;
	void setAddr(laddr_t addr);

	// TODO: access - это тоже получается процессная фича? или регионная но не тредовая.
	explicit Instance(Resource *resource, uint32_t access = 0);
	
public:
	Link<Instance> ResourceLink;

	virtual ~Instance();
	
	Resource *resource() const;
	id_t id() const;
	
	bool allow(uint32_t access) const;
	uint32_t getAccess() const { return m_access; } 
	
	virtual void event(uint32_t eid);
	virtual bool active() const;
};

} // namespace Core
