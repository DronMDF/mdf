//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

namespace Core {

class ResourceThread;
class ResourceProcess;

class ResourceCall : public Resource
{
private:
	ResourceProcess *m_process;
	laddr_t m_entry;

	ResourceCall(ResourceProcess *process, laddr_t entry);

	ResourceCall();
	ResourceCall(const ResourceCall &);
	ResourceCall & operator =(const ResourceCall &);

public:
	static Resource *Create(ResourceProcess *process, const void *param, size_t size);

	virtual ResourceCall *asCall();

	laddr_t getEntry() const;

	virtual ResourceThread *Call();
};

} // namespace Core;
