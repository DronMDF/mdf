//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct _Task Task;

namespace Core {

class ResourceThread;
	
class CallHelper {
private:
	int m_status;
	
	const Task *task;
	id_t id;
	const void *buffer;
	size_t buffer_size;
	int flags;

	void setStatus(int status);

protected:
	int getStatus() const;
	
	ResourceThread *createCalledThread(const Task *task, id_t id);
	void copyOutRequest(ResourceThread *thread, laddr_t base,
			    const void *request, size_t request_size) const;

public:
	CallHelper(const Task *task, id_t id, 
		   const void *buffer, size_t buffer_size, int flags);
		   
	int execute();
};

} // namespace Core
