//
// Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 13/12/06 00:13:53
//

#pragma once

struct dirent {
	char d_name[256];
	unsigned int d_fileno;
	unsigned int d_ino;
};

typedef struct dirent DIR;

