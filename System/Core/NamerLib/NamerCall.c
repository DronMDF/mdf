//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#include <MDF/Kernel.h>

// TODO: Надо будет разнести по объектникам... пока мне лень...

// TODO: Вообще здесь должен стоять BAD_HANDLE, Но у нас пока проблемы с bss.
static handle NamerHandle = 0xffffffff;

static handle NamerFind (void)
{
	if (NamerHandle != 0xffffffff)
		return NamerHandle;

	// Ждем намера
	// TODO: По идее надо таймаут предусмотреть...
	while (1) {
		if (KernelResourceFind ("Namer", 6, &NamerHandle) == KERNEL_OK)
			return NamerHandle;

		KernelSheduleNext (100);
	}

	return BAD_HANDLE;
}

result NamerCall (const void * const Buffer, const size BufferSize, const uint32 Flags)
{
	handle nh = NamerFind ();

	if (nh == BAD_HANDLE)
		return KERNEL_INVALIDHANDLE;

	return KernelResourceCall (nh, Buffer, BufferSize, Flags);
}

handle NamerProcess (void)
{
	handle nh = NamerFind ();

	if (nh == BAD_HANDLE)
		return BAD_HANDLE;

	handle namer_proc;
	size sz = (size)sizeof (namer_proc);
	if (KernelResourceInfo (KERNEL_INFO_RESOURCE_OWNER, nh, &namer_proc, &sz) != KERNEL_OK)
		return BAD_HANDLE;

	return namer_proc;
}
