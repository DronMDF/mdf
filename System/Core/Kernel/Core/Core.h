//
// Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include "Stub.h"

#if defined(__cplusplus)
extern "C" {
#endif

// Служебные...
const char *CoreVersion(void);

void CoreInit(void);

unsigned long CoreRandom (void);
void CorePrint (const char *format, ...);

// Внутреннее отражение API
int CoreWait (const Task *task, id_t id, int evt, timeout_t timeout);
int CoreFind (const char *name, size_t name_size, id_t * const id);
int CoreCreate (const Task *task, int type, const void *param,
	size_t param_size, id_t *id);
int CoreCall (const Task *task, id_t id, const void *buffer, size_t buffer_size,
	int flags);
int CoreAttach (const Task *task, id_t rid, id_t pid,
		int access, uint32_t spec);
int CoreRelease (id_t id, int flags);
int CoreModify (const Task *task, id_t id, int modify_id,
		const void *param, size_t param_size);
int CoreInfo (const Task *task, id_t id, int infoId, void *info,
		size_t *info_size);

const PageInstance *CoreThreadPageFault (const Task *task,
	laddr_t addr, uint32_t *access);

#if defined(__cplusplus)
} // extern "C"
#endif
