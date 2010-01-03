//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct _Task Task;

namespace Core {

class Resource;
class ResourceThread;
class ResourceInstance;
	
class CallHelper {
private:
	CallHelper(const CallHelper &);
	CallHelper &operator=(const CallHelper &);

protected:
	ResourceThread *m_caller;
	ResourceThread *m_called;
	
	ResourceThread *getCallerThread(const Task *task) const;
	virtual ResourceInstance *getCalledInstance(ResourceThread *thread, id_t id) const;

public:
	CallHelper(const Task *task);
	virtual ~CallHelper();
	
	bool checkCalledAccess(id_t id);
	bool createCalled(id_t id);

	bool copyOutRequest(const void *request, size_t size, uint32_t access) const;
	void setCopyBack(const void *request, size_t size) const;

	void runAsinchronized() const;
	void runSinchronized() const;
};

} // namespace Core
