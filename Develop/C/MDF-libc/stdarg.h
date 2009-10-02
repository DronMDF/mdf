//
// Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
// All rights reserved.
//

#pragma once

// #include "stddef.h"
//
// typedef const char **va_list;
//
// #define va_arg(ap,type)		((type)(*(++ap)))
// #define va_copy(src,dst)	((src)=(va_list)(dst))
// #define va_end(ap)		((ap)=NULL)
// #define va_start(ap,p)		((ap)=((va_list)&(p)))

typedef __builtin_va_list va_list;

#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
#define va_copy(d,s)	__builtin_va_copy(d,s)
