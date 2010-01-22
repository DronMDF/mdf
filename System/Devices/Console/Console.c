//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Types.h>
#include <MDF/Kernel.h>
#include <MDF/KernelImp.h>

#include <MDF/Namer.h>

#include <stdio.h>
#include <string.h>
#include <stddef.h>
#include <stdbool.h>

char id[] = "MDFVER: System-Device/Console-" VERSION;

void ConsoleService(id_t tid, void *buffer, size_t size, uint32_t flags);
void PrintCharacter(int ch);
void PrintString(const char *str);

static volatile uint16_t *vbuf = NULL;
static lock_t console_lock;

static bool port_enable = false;
static int vpemu = 80 * 24;

static
volatile uint16_t *getVideoMemory()
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

	return (volatile uint16_t *)addr;
}

static int GetPortAccess(uint16_t first_port, uint16_t last_port)
{
	const struct KernelCreateRegionParam cpar = {
		.offset = 0,
		.size = last_port - first_port + 1,
		.access = RESOURCE_ACCESS_READ | RESOURCE_ACCESS_WRITE,
	};

	id_t rid = INVALID_ID;
	
	int rv = KernelCreate(RESOURCE_TYPE_REGION, &cpar, sizeof(cpar), &rid);
	if (rv != SUCCESS) return rv;

	rv = KernelModify(rid, RESOURCE_MODIFY_REGION_PORTBIND, &first_port, sizeof(uint16_t));
	if (rv != SUCCESS) {
		KernelDetach(rid, 0);
		return rv;
	}

	port_enable = true;
	return SUCCESS;
}

bool RegisterService()
{
	id_t namer_pid = NamerProcess();

	char tmpmsg[256];
	sprintf(tmpmsg, "Console: Namer process id - %u.\n", namer_pid);
	PrintString(tmpmsg);

// 	if (KernelTPCCreate(0, ConsoleService, 0, &id) != KERNEL_OK)
// 		return false;
// 
// 	PrintString ("Console: TPC entry created.\n");
// 
// 	// Надо разрешить намеру вызывать TPC
// 	if (KernelResourceAttach (id, namer_proc, KERNEL_ACCESS_CALL, 0) != KERNEL_OK) {
// 		PrintString ("Console: KernelResourceAttach failed!\n");
// 		return false;
// 	}
// 
// 	char req[256];
// 	union namer_message *msg = (union namer_message *)req;
// 
// 	sprintf (msg->Request.Request,
// 		NSSN "Register?prefix='Console://'&tpc=%u",
// 		(unsigned int)id);
// 	msg->Request.Offset = offsetof (union namer_message, Request.Request);
// 	msg->Request.Size = strlen (msg->Request.Request) + 1;
// 
// 	sprintf (tmpmsg, "Console: registration request - '%s'\n",
// 		msg->Request.Request);
// 	PrintString (tmpmsg);
// 
// 	if (NamerCall (msg, msg->Request.Offset + msg->Request.Size, 0)
// 		!= KERNEL_OK ||
// 	    msg->Reply.Status != NAMER_OK)
// 	{
// 		PrintString ("Console: KernelResourceCall failed!\n");
// 		return false;
// 	}

	return true;
}

int main(int argc, char **argv)
{
	unlock(&console_lock);
	// TODO: ошибка, .bss так и не очищается для модулей.
	port_enable = false;

	vbuf = getVideoMemory();
	vpemu = 80 * 24;
	PrintString("Console: Video memory mapped.\n");

	if (GetPortAccess(0x3d4, 0x3d5) != SUCCESS) {
		PrintString("Console: Port access failed.\n");
		return -1;
	}
	PrintString("Console: Port access granted.\n");

	if (RegisterService() == false) {
		return -1;
	}

	PrintString("Console: Initialization done.\n");
	return 0;
}

void ConsoleService(id_t tid, void *buffer, size_t size, uint32_t flags)
{
// 	if (BufferSize < sizeof (union namer_message))
// 		return;
// 
// 	union namer_message *msg = (union namer_message *)Buffer;
// 
// 	if (msg->Request.Offset + msg->Request.Size > BufferSize)
// 		return;
// 
// 	// Консоль пока тупая до безобразия.
// 	Lock(&OutputLock);
// 
// 	if (memchr ((char *)msg + msg->Request.Offset, 0, msg->Request.Size)
// 		!= NULL)
// 	{
// 		PrintString ((char *)msg + msg->Request.Offset);
// 	}
// 
// 	Unlock(&OutputLock);

	return;
}

int GetCursorPosition ()
{
	if (!port_enable) {
		return vpemu;
	}
	
	write_io_byte(0x3d4, 14);
	int hi = read_io_byte(0x3d5);
	write_io_byte(0x3d4, 15);
	int lo = read_io_byte(0x3d5);

	hi &= 0xff;
	lo &= 0xff;

	return (hi << 8) | lo;
}

void SetCursorPosition (int p)
{
	if (!port_enable) {
		vpemu = p;
		return;
	};
	
	write_io_byte(0x3d4, 15);
	write_io_byte(0x3d5, p & 0xff);
	write_io_byte(0x3d4, 14);
	write_io_byte(0x3d5, (p >> 8) & 0xff);
}

void PrintCharacter(int ch)
{
	int vptr = GetCursorPosition();

	if (vptr >= 80 * 25) {
		// Scroll
		for (int i = 0; i < 80 * 24; i++)
			vbuf[i] = vbuf[i + 80];
		for (int i = 80 * 24; i < 80 * 25; i++)
			vbuf[i] = 0;
		vptr -= 80;
	}

	ch &= 0x7f;

	switch (ch) {
		default:
			vbuf[vptr++] = ch | 0x700;
			break;
		case '\n':
			vptr += 80;
			vptr -= vptr % 80;
			break;
	}

	SetCursorPosition(vptr);
}

void PrintString(const char *str)
{
	while (*str != 0) {
		PrintCharacter(*str);
		str++;
	}
}
