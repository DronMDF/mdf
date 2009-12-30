//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>
#include <MDF/KernelImp.h>

#include <stdio.h>

char id[] = "MDFVER: System-Test/TPCTest-" VERSION;

static volatile void *vbuf = NULL;

static
volatile void *getVideoMemory ()
{
	id_t rid = INVALID_ID;

	const struct KernelCreateRegionParam cpar = {
		.offset = 0,
		.size = 4000,
		.access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE,
	};

	if (KernelCreate(RESOURCE_TYPE_REGION, &cpar, sizeof(cpar), &rid) != SUCCESS)
		return NULL;

	if (rid == INVALID_ID)
		return NULL;

	// Биндим на физическую видеопамять
	const struct KernelModifyRegionBindParam mpar = {
		.id = 0,
		.offset = 0xb8000,
		.size = 4000,
		.shift = 0,
	};

	if (KernelModify(rid, RESOURCE_MODIFY_REGION_PHYSICALBIND, &mpar, sizeof(mpar)) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	laddr_t addr = 0;
	if (KernelModify(rid, RESOURCE_MODIFY_REGION_MAP, &addr, sizeof(laddr_t)) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	size_t addr_size = sizeof(laddr_t);
	if (KernelInfo(rid, RESOURCE_INFO_REGION_INSTANCE_ADDR, &addr, &addr_size) != SUCCESS) {
		KernelDetach(rid, 0);
		return NULL;
	}

	return (volatile void *)addr;
}

void Message(int row, const char *msg)
{
	for (int i = 0; i < 80 && *msg != '\0'; i++, msg++) {
		*((uint16_t *)vbuf + row * 80 + i) = *msg | 0xf400;
	}
}

void
ServiceThread (id_t process __attribute__((unused)),
	void *param, size_t size, uint32_t flags)
{
	if (flags & RESOURCE_CALL_READONLY)
		return;		// Нечего сказать!

	if (size < sizeof(size_t))
		return;		// Минимум однако!

	size_t width = *(size_t *)param;

	for (size_t i = 0; i < size / 2; i++)
		((uint16_t *)param)[i] = (i < width) ? ('*' + 0xf00) : 0;
}

static id_t service = INVALID_ID;

void
BlinkThread (id_t process __attribute__((unused)),
	void *param, size_t size, uint32_t flags __attribute__((unused)))
{
	if (size < sizeof(int))
		return;

	int row = *(int *)param;
	size_t c = 0;
	int cstep = 1;
	void *ptr = (char *)vbuf + row * 160;

	while (1) {
		*((int *)ptr) = c;
		if (KernelCall(service, ptr, 160, RESOURCE_CALL_COPY) != SUCCESS) {
			Message(row, "Unable to call service");
			continue;
		}

		c += cstep;

		if (cstep == 1 && c == 80)
			cstep = -1;

		if (cstep == -1 && c == 0)
			cstep = 1;

		if (KernelWait(0, 0, 0) != SUCCESS) {
			Message(row, "Wait problem?");
			continue;
 		}
	}
}

int main (int argc __unused__, char **argv __unused__)
{
	vbuf = getVideoMemory();
	if (vbuf == NULL) return -1;

	struct KernelCreateCallParam cp = {
		.entry = (laddr_t)ServiceThread,
	};

	if (KernelCreate(RESOURCE_TYPE_CALL, &cp, sizeof(cp), &service) != SUCCESS) {
		Message(1, "Unable to create CALL");
		return -1;
	}

	for (int i = 1; i < 25; i++) {
		id_t nt = INVALID_ID;

		struct KernelCreateThreadParam tp = {
			.entry = (laddr_t)BlinkThread,
		};

		if (KernelCreate(RESOURCE_TYPE_THREAD, &tp, sizeof(tp), &nt) != SUCCESS) {
			Message(i, "Unable to create THREAD");
			continue;
		}

		if (nt == INVALID_ID) {
			Message(i, "Unable to create THREAD");
			continue;
		}

		if (KernelCall(nt, &i, sizeof(int), RESOURCE_CALL_ASYNC |
				RESOURCE_CALL_COPY | RESOURCE_CALL_READONLY) != SUCCESS)
		{
			Message(i, "Unable to call THREAD");
			continue;
		}
	}

	return 0;
}

