//
// Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
// All rights reserved.
//
// Created: 11/12/06 14:41:21
//

#pragma once(__SYS_TIME_H__)

struct timeval {
	int tv_sec, tv_usec;
};

struct timezone {
	int tz_minuteswest;
};
