//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#ifndef __MDF_NAMER_H_
#define __MDF_NAMER_H_

#include <MDF/Kernel.h>

#define NAMER_OK		0
#define NAMER_NO_SERVICE	1
#define NAMER_INVALID_PARAM	2
#define NAMER_BUSY		3
#define NAMER_FAIL		4

union namer_message
{
	struct {
		int Status;
		char Reply[];
	} Reply;

	struct {
		size Offset;
		size Size;
		char Request[];
	} Request;
};

// NamerSelfServiceName
#define NSSN "Namer://"

result NamerCall (const void * const Buffer, const size BufferSize, const uint32 Flags);
handle NamerProcess (void);

#endif // __MDF_NAMER_H_
