//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>
#include <MDF/KernelImp.h>

#include <MDF/IOPorts.h>
#include <MDF/Locks.h>

#include <MDF/Namer.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

char id[] = "MDFVER: System-Device/Console-" VERSION;

void ConsoleService (handle tid, void *Buffer, size BufferSize, uint32 Flags);
void PrintCharacter(int ch);
void PrintString (const char *str);

static
volatile unsigned short *vbuf = NULL;

static char tmpmsg[256];

lock_t OutputLock;

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

static result GetPortAccess (int first_port, int last_port)
{
	result rv;
	handle ph;

	struct ResourceCreatePortParam p = {
		first_port,
		last_port,
		ACCESS_WRITE
	};

	if ((rv = KernelResourceCreate (RESOURCE_PORT, &p,
		sizeof (struct ResourceCreatePortParam), &ph)) != KERNEL_OK)
		return rv;

	return KERNEL_OK;
}

bool RegisterService ()
{
	handle namer_proc, id;

	namer_proc = NamerProcess ();

	sprintf (tmpmsg, "Console: Namer process id - %u.\n",
		(unsigned int)namer_proc);
	PrintString (tmpmsg);

	if (KernelTPCCreate (0, ConsoleService, 0, &id) != KERNEL_OK)
		return false;

	PrintString ("Console: TPC entry created.\n");

	// Надо разрешить намеру вызывать TPC
	if (KernelResourceAttach (id, namer_proc, KERNEL_ACCESS_CALL, 0) != KERNEL_OK) {
		PrintString ("Console: KernelResourceAttach failed!\n");
		return false;
	}

	char req[256];
	union namer_message *msg = (union namer_message *)req;

	sprintf (msg->Request.Request,
		NSSN "Register?prefix='Console://'&tpc=%u",
		(unsigned int)id);
	msg->Request.Offset = offsetof (union namer_message, Request.Request);
	msg->Request.Size = strlen (msg->Request.Request) + 1;

	sprintf (tmpmsg, "Console: registration request - '%s'\n",
		msg->Request.Request);
	PrintString (tmpmsg);

	if (NamerCall (msg, msg->Request.Offset + msg->Request.Size, 0)
		!= KERNEL_OK ||
	    msg->Reply.Status != NAMER_OK)
	{
		PrintString ("Console: KernelResourceCall failed!\n");
		return false;
	}

	return true;
}

int main (int argc, char **argv)
{
	Unlock (&OutputLock);

	// TODO: Функцию надо переделывать!
	if (KernelRegionPhysicalGet (0xb8000, 4000, &vbuf, (handle *)NULL) != KERNEL_OK)
	{
		return -1;
	}

	if (GetPortAccess (0x3d4, 0x3d5) != KERNEL_OK) {
		return -1;
	}

	PrintString ("\nConsole: Video memory mapped.\n");

	if (RegisterService () == false) {
		return -1;
	}

	return 0;
}

void ConsoleService (handle tid, void *Buffer, size BufferSize, uint32 Flags)
{
	if (BufferSize < sizeof (union namer_message))
		return;

	union namer_message *msg = (union namer_message *)Buffer;

	if (msg->Request.Offset + msg->Request.Size > BufferSize)
		return;

	// Консоль пока тупая до безобразия.
	Lock(&OutputLock);

	if (memchr ((char *)msg + msg->Request.Offset, 0, msg->Request.Size)
		!= NULL)
	{
		PrintString ((char *)msg + msg->Request.Offset);
	}

	Unlock(&OutputLock);

	return;
}

int GetCursorPosition ()
{
	out (0x3d4, 14);
	int hi = in (0x3d5);
	out (0x3d4, 15);
	int lo = in (0x3d5);

	hi &= 0xff;
	lo &= 0xff;

	return (hi << 8) | lo;
}

void SetCursorPosition (int p)
{
	out (0x3d4, 15);
	out (0x3d5, p & 0xff);
	out (0x3d4, 14);
	out (0x3d5, (p >> 8) & 0xff);
}

void PrintCharacter(int ch)
{
	int vptr = GetCursorPosition ();

	ch &= 0x7f;

	if (vptr >= 80 * 25) {
		// Scroll
		for (int i = 0; i < 80 * 24; i++)
			vbuf[i] = vbuf[i + 80];
		for (int i = 80 * 24; i < 80 * 25; i++)
			vbuf[i] = 0;
		vptr -= 80;
	}

	switch (ch) {
		default:
			vbuf[vptr++] = ch | 0x700;
			break;
		case '\n':
			vptr += 80;
			vptr -= vptr % 80;
			break;
	}

	SetCursorPosition (vptr);
}

void PrintString (const char *str)
{
	while (*str != 0) {
		PrintCharacter (*str);
		str++;
	}
}
