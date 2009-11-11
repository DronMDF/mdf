//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

typedef struct _Task Task;

namespace Core {

class InterfaceCallHelper {
private:
	const Task *task;
	id_t id;
	const void *buffer;
	size_t buffer_size;
	int flags;
	
public:
	InterfaceCallHelper(const Task *task, id_t id, const void *buffer, size_t buffer_size, int flags);
	int execute();
};

} // namespace Core
