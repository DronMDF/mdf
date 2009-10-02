#!/usr/bin/sed -nf
#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 11/12/06 15:16:12
#

# Enable Futures
s/^.*#undef\W\+\(HAVE_STRERROR\).*$/#define \1 1/
s/^.*#undef\W\+\(HAVE_VPRINTF\).*$/#define \1 1/
s/^.*#undef\W\+\(HAVE_STRCHR\).*$/#define \1 1/
s/^.*#undef\W\+\(TM_IN_SYS_TIME\).*$/#define \1 1/

# disable Futures
s/^.*#define\W\+\(HAVE_SYS_ERRLIST\)\W.*$/#undef \1/
s/^.*#define\W\+\(HAVE_SYS_RESOURCE_H\)\W.*$/#undef \1/




