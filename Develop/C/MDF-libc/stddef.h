//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#pragma once

typedef int prtdiff_t;

#ifndef __size_t_are_declared
typedef unsigned int size_t;
#define __size_t_are_declared 1
#endif

#ifndef __cplusplus	// Для С++ это встроенный тип!
typedef unsigned short wchar_t;
#endif

#undef NULL	// Вдруг старый нуль не по стандарту :)
#define NULL ((void *)0)

#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
