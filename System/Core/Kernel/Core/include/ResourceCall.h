//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Resource.h"

namespace Core {

class ResourceCall : public Resource
{
private:
	Process *m_process;
	laddr_t m_entry;

	ResourceCall(Process *process, laddr_t entry);

	ResourceCall();
	ResourceCall(const ResourceCall &);
	ResourceCall & operator =(const ResourceCall &);

public:
	static Resource *Create(Process *process, const void *param, size_t size);

	virtual ResourceCall *asCall();

	laddr_t getEntry() const;

	virtual ResourceThread *Call();
};

} // namespace Core;
