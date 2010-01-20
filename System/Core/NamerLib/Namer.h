//
// Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
// This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
//

#pragma once

#include <MDF/Kernel.h>

enum {
	NAMER_OK = 0,
	NAMER_NO_SERVICE = 1,
	NAMER_INVALID_PARAM = 2,
	NAMER_BUSY = 3,
	NAMER_FAIL = 4,
};

// TODO: Заменить на прообраз протокола
//	Или во! Сделать библиотеку NamerProto
// union namer_message
// {
// 	struct {
// 		int Status;
// 		char Reply[];
// 	} Reply;
// 
// 	struct {
// 		size Offset;
// 		size Size;
// 		char Request[];
// 	} Request;
// };

// NamerSelfServiceName
static const char NamerName[] = "NAMER";
static const char NamerPrefix[] = "namer://";

int NamerCall(void *buffer, size_t size, uint32_t flags);
id_t NamerProcess();
