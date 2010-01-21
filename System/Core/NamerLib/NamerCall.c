//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>
//#include <MDF/KernelImp.h>

#include <string.h>

#include "Namer.h"

// TODO: Надо будет разнести по объектникам... пока мне лень...

static id_t NamerId = INVALID_ID;

static id_t NamerFind()
{
	if (NamerId != INVALID_ID) {
		return NamerId;
	}

	// Ждем намера
	// TODO: По идее надо таймаут предусмотреть...
	while (1) {
		if (KernelFind (NamerName, strlen(NamerName), &NamerId) == SUCCESS) {
			return NamerId;
		}

		KernelWait(0, 0, 100);
	}

	return INVALID_ID;
}

int NamerCall(void *buffer, size_t size, uint32_t flags)
{
	const id_t nid = NamerFind();

	if (nid == INVALID_ID) {
		return ERROR_INVALIDID;
	}

	return KernelCall(nid, buffer, size, flags);
}

id_t NamerProcess(void)
{
	const id_t nid = NamerFind();

	if (nid == INVALID_ID) {
		return INVALID_ID;
	}

	id_t pid = INVALID_ID;
	size_t pid_size = sizeof(pid);
	if (KernelInfo(nid, RESOURCE_INFO_OWNER, &pid, &pid_size) != SUCCESS) {
		return INVALID_ID;
	}

	return pid;
}
