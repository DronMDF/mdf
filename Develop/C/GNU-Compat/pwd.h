//
// Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 11/12/06 14:48:22
//

#pragma once

typedef unsigned long uid_t;
typedef unsigned long gid_t;

struct passwd
{
	char	*pw_name;
//	char	*pw_passwd;
//	uid_t	pw_uid;
//	gid_t	pw_gid;
//	char	*pw_gecos;
	char	*pw_dir;
	char	*pw_shell;
};
