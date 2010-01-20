//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#include <MDF/Kernel2.h>
#include <MDF/Locks.h>

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <MDF/Namer.h>

char id[] = "MDFVER: System-Core/Namer-" VERSION;

#define STATUS_EMPTY	0
#define STATUS_USED	1

#define RECORDS		32

// Статическая декларация - мера временная
struct NamerReq {
	bool used;
	handle tpc;
	char prefix[128];
} NamerTable[RECORDS];

lock_t NamerSelfLock;

void  NamerService (handle Caller, void *Buffer, size BufferSize, uint32 Flags);
uint32 NamerSelfService (union namer_message *msg);
uint32 NamerForwardService (union namer_message *msg);

int main (int argc, char **argv)
{
	// Инициализация
	for (int i = 0; i < RECORDS; i++)
		NamerTable[i].used = false;

	Unlock(&NamerSelfLock);

	handle nh;
	KernelTPCCreate (0, NamerService, KERNEL_ACCESS_GLOBALCALL | KERNEL_ACCESS_GLOBALINFO, &nh);
	KernelResourceModify (nh, KERNEL_MODIFY_RESOURCE_NAME, "Namer", 6);
}

void  NamerService (handle Caller, void *Buffer, size BufferSize, uint32 Flags)
{
	if (Flags & KERNEL_RESOURCE_CALL_READONLY ||
	    Buffer == NULL || BufferSize < sizeof (int))
	{
		return;
	}

	union namer_message *msg = (union namer_message *)Buffer;

	if (BufferSize < sizeof (union namer_message)) {
		msg->Reply.Status = NAMER_INVALID_PARAM;
		return;
	}

	if (msg->Request.Offset + msg->Request.Size > BufferSize) {
		msg->Reply.Status = NAMER_INVALID_PARAM;
		return;
	}

	if (msg->Request.Size >= strlen(NSSN) &&
	    memcmp (msg->Request.Request, NSSN, strlen (NSSN)) == 0)
	{
		msg->Request.Offset += strlen (NSSN);
		msg->Request.Size -= strlen (NSSN);

		msg->Reply.Status = NamerSelfService (msg);
		return;
	}

	msg->Reply.Status = NamerForwardService (msg);
	return;
}

uint32 NamerSelfService (union namer_message *msg)
{
	// Строка должна оканчиваться на 0!
	if (memchr ((char *)msg + msg->Request.Offset, 0, msg->Request.Size)
		== NULL)
	{
		return NAMER_INVALID_PARAM;
	}

	Lock(&NamerSelfLock);

	int idx = -1;

	for (int i = 0; i < RECORDS; i++) {
		if (NamerTable[i].used == false) {
			idx = i;
			break;
	}	}

	if (idx == -1) {
		// Нет свободного места
		Unlock(&NamerSelfLock);
		return NAMER_BUSY;
	}

	if (sscanf ((char *)msg + msg->Request.Offset,
	    "Register?prefix='%[^']'&tpc=%u",
	    NamerTable[idx].prefix,
	    (unsigned int *)((void *)&(NamerTable[idx].tpc))) < 2)
	{
		Unlock(&NamerSelfLock);
		return NAMER_INVALID_PARAM;
	}

	NamerTable[idx].used = true;
	Unlock(&NamerSelfLock);

	return NAMER_OK;
}

uint32 NamerForwardService (union namer_message *msg)
{
	int idx = -1;
	size eqc;

	for (int tm = 0; tm < 500; tm++) {
		// Ищем подходящий сервис
		Lock(&NamerSelfLock);

		eqc = 0;
		for (int i = 0; i < RECORDS; i++)
		{
			if (NamerTable[i].used)
			{
				size e = strlen (NamerTable[i].prefix);
				if (msg->Request.Size >= e &&
				    strncmp ((char *)msg + msg->Request.Offset,
					NamerTable[i].prefix, e) == 0 &&
				    e > eqc)
				{
					idx = i; eqc = e;
				}
			}
		}

		Unlock(&NamerSelfLock);

		if (idx != -1)
			break;

		KernelSheduleNext (0);
	}

	if (idx == -1)
		return NAMER_NO_SERVICE;	// Истек таймаут

	// Здесь пока не идеально... надо форвардить на основе данных из ядра.
	msg->Request.Offset += eqc;
	msg->Request.Size -= eqc;

	if (KernelResourceCall (NamerTable[idx].tpc, msg,
		msg->Request.Offset + msg->Request.Size,
		KERNEL_RESOURCE_CALL_COPY) != KERNEL_OK)
	{
		return NAMER_FAIL;
	}

	return NAMER_OK;
}
