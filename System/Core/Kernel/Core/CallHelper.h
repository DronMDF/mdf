//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
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
	int m_status;
	
	const Task *task;
	id_t id;
	const void *buffer;
	size_t buffer_size;
	int flags;

	template<typename T>
	T returnStatus(int status);

	void setStatus(int status);

protected:
	int getStatus() const;

	ResourceThread *getCallerThread(const Task *task) const;
	Resource *findCalledResource(id_t id) const;
	ResourceInstance *getCalledInstance(ResourceThread *thread, id_t id) const;
	
	bool copyOutRequest(ResourceThread *called, const void *request,
			    size_t request_size, uint32_t access);

	bool setCopyBack(ResourceThread *called, ResourceThread *thread,
		const void *request, size_t request_size) const;

public:
	CallHelper(const Task *task, id_t id, 
		   const void *buffer, size_t buffer_size, int flags);
		   
	int execute();
};

} // namespace Core
