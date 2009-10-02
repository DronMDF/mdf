//
// Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 26/06/07 22:30:50
//

#include <MDF/Kernel2.h>
#include <string.h>

void  Proc (handle Caller, void *Buffer, size BufferSize, uint32 Flags)
{
	// Меняем код!
	((int *)Buffer)[0] = 0x33333333;
}

int main (int argc, char **argv)
{
	handle pid = 0;
	int entry = 0x5000;

	KernelResourceCreate (RESOURCE_PROCESS, &entry, 4, &pid);

	handle code = 0;
	struct ResourceCreateRegionParam reg_param =
		{ 0x32, 0, 0, 0 };

	KernelResourceCreate (RESOURCE_REGION, &reg_param,
		sizeof(struct ResourceCreateRegionParam), &code);

	char *laddr = NULL;
	size psize = sizeof (char *);
        KernelResourceInfo (KERNEL_INFO_REGION_LADDR, code, &laddr, &psize);

	memcpy (laddr, Proc, 16);

	KernelResourceAttach (code, pid, 0, entry);

	static int data = 0xaaaaaaaa;
	KernelResourceCall (pid, &data, sizeof(int), KERNEL_RESOURCE_CALL_COPY);

 	KernelResourceWait (pid, RESOURCE_EVENT_DESTROY, 0xffffffff);

	while (1);

	return 0;
}

