//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>

int main(int argc, char **argv);

void ProcessEntry (id_t handle __unused__, void *buffer, size_t size, uint32_t flags)
{
	int rv = main (0, 0);

	if (size < sizeof (int)) return;
	if ((flags & RESOURCE_CALL_READONLY) != 0) return;

	*((int *)buffer) = rv;
}
