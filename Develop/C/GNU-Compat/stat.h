//
// Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 30/11/06 16:10:02
//

#pragma once

struct stat {
	unsigned int st_size;
	unsigned int st_mode;
	unsigned int st_dev;
	unsigned int st_ino;
	unsigned long st_atime;
	unsigned long st_mtime;
	unsigned int st_uid;
	unsigned int st_gid;
	unsigned long st_nlink;
};

#define	S_ISGID		02000
#define	S_ISUID		04000
#define S_IFREG       0100000

typedef unsigned long dev_t;
typedef unsigned long ino_t;
