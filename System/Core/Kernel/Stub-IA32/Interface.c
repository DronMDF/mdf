//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <Core.h>
#include <Kernel.h>

#include "StubLocal.h"
#include "Arch.h"
#include "Memory.h"
#include "Page.h"

int StubInfoMemory(void *info, size_t *info_size);

int StubWait (id_t id, uint32_t event, timeout_t timeout)
{
	const Task *task = StubGetCurrentTask();
	return CoreWait(task, id, event, timeout);
}

int StubFind (const uaddr_t name_ptr __unused__, size_t name_size __unused__,
		const uaddr_t id_ptr __unused__)
{
	STUB_FATAL ("Under constructed");
	return -1;
}

int StubCreate (const int type, const uaddr_t param_ptr,
		const size_t param_size, const uaddr_t id_ptr)
{
	if (param_ptr + param_size > USER_MEMORY_SIZE ||
	    id_ptr + sizeof (id_t) > USER_MEMORY_SIZE)
	{
		return ERROR_INVALIDPARAM;
	}

	laddr_t param = u2laddr(param_ptr);
	if (!StubMemoryReadable (param, param_size))
		return ERROR_INVALIDPARAM;

	laddr_t id = u2laddr(id_ptr);
	const Task *task = StubGetCurrentTask();
	// return CoreCreate (task, type, l2vptr(param), param_size, l2vptr(id));
	int rv = CoreCreate (task, type, l2vptr(param), param_size, l2vptr(id));

	if (rv != SUCCESS) {
		CorePrint ("CoreCreate fail: %u\n", rv);
	}

	return rv;
}

int StubCall (id_t id, uaddr_t param, size_t size, uint32_t flags)
{
	if (param + size > USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	laddr_t paddr = u2laddr(param);
	if (!StubMemoryReadable(paddr, size))
		return ERROR_INVALIDPARAM;

	const Task *task = StubGetCurrentTask();
	int rv = CoreCall(task, id, l2vptr(paddr), size, flags);

	if (rv != SUCCESS) {
		CorePrint ("CoreCall fail: %u\n", rv);
	}

	return rv;
}

int StubAttach (const id_t rid, const id_t pid, const uint32_t access, const uint32_t specific)
{
	const Task *task = StubGetCurrentTask();

	//return CoreAttach (task, rid, pid, access, specific);
	int rv = CoreAttach (task, rid, pid, access, specific);

	if (rv != SUCCESS)
		CorePrint ("CoreAttach fail: %u\n", rv);

	return rv;
}

int StubDetach (const id_t id, const int flags)
{
	const Task *task = StubGetCurrentTask();
	
	const int rv = CoreDetach(task, id, flags);
	if (rv != SUCCESS) {
		CorePrint("CoreDetach fail: %u\n", rv);
	}
	
	return rv;
}

int StubModify (const id_t id, const int modify_id,
		const uaddr_t param_ptr, const size_t param_size)
{
	if (param_ptr + param_size > USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	laddr_t param = u2laddr(param_ptr);
	if (!StubMemoryReadable (param, param_size))
		return ERROR_INVALIDPARAM;

	const Task *task = StubGetCurrentTask();

	//return CoreModify (task, id, modify_id, l2vptr(param), param_size);

	// Вывод информации о сбое
	const int rv = CoreModify(task, id, modify_id, l2vptr(param), param_size);
	if (rv != SUCCESS) {
		CorePrint("CoreModify(0x%08x) fail: %u\n", modify_id, rv);
	}

	return rv;
}

int StubInfo (id_t id, int info_id, uaddr_t info_ptr, uaddr_t info_size_ptr)
{
	if (info_ptr >= USER_MEMORY_SIZE || info_size_ptr >= USER_MEMORY_SIZE)
		return ERROR_INVALIDPARAM;

	if (info_size_ptr == 0)
		return ERROR_INVALIDPARAM;

	void *info = nullptr;
	if (info_ptr != 0)
		info = l2vptr(u2laddr(info_ptr));

	size_t *info_size = nullptr;
	if (info_size_ptr != 0)
		info_size = l2vptr(u2laddr(info_size_ptr));

	switch (info_id) {
		case RESOURCE_INFO_STUB_VERSION: {
			const uint32_t version = 1;
			return StubInfoValue(info, info_size, &version, sizeof(uint32_t));
		}

		case RESOURCE_INFO_MEMORY:
			return StubInfoMemory(info, info_size);

		default:
			break;
	}

	const Task *task = StubGetCurrentTask();
	// return CoreInfo(task, id, info_id, info, info_size);

	int rv = CoreInfo(task, id, info_id, info, info_size);
	if (rv != SUCCESS) {
		CorePrint ("CoreInfo(0x%08x) fail: %u\n", info_id, rv);
	}

	return rv;
}

// API utility

int StubInfoMemory(void *info, size_t *info_size)
{
// 	struct KernelInfoMemory minfo = {
// 		.MemoryTotal = StubGetMemoryTotal(),
// 		.MemoryUsed = StubGetMemoryUsed(),
// 		.KernelMemoryUsed = StubKernelPagesCnt() * PAGE_SIZE,
// 		.KernelHeapTotal = StubMemoryReserve(),
// 		.KernelHeapUsed = 0,
// 	};

	// Это очень медленный способ опрделения памяти ядра.
	// введен потому, что вышеупомянутые функции врут.
	// Потом я здесь еще проверки вставлю.
	struct KernelInfoMemory minfo2;
	StubCalcMemoryUsage(&minfo2);
	StubCalcHeapUsage(&minfo2);

	return StubInfoValue(info, info_size, &minfo2, sizeof(struct KernelInfoMemory));
}

int StubInfoValue (void *info, size_t *info_size, const void *data, size_t data_size)
{
	if (info_size == nullptr)
		return ERROR_INVALIDPARAM;

	if (info != nullptr)
		StubMemoryCopy(info, data, min(*info_size, data_size));

	*info_size = data_size;
	return SUCCESS;
}
