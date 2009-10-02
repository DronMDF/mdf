#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 29/11/06 21:54:42
#

WDIR = $(MDF_TEMP)/bash-3.2

# Мое стандартное значение для configure не подходит.
LDFLAGS = -nostdlib -Xlinker -Tt -Xlinker 1000 -Xlinker -eProcessEntry \
        $(MDF_ROOT)/Develop/Library/Startup.o \
        -L$(MDF_ROOT)/Develop/Library

all:
	cd $(WDIR) && ./configure --host=i686-pc-linux-gnu \
		--prefix=$(MDF_IMAGE)/GNU --enable-static-link \
		--enable-minimal-config --without-libintl-prefix

	# Уродский конфигур вообще нифига не чекает.
	# Правим настройки сами.
	sed -f config.h.sed -i $(WDIR)/config.h

	# mkbuiltins собирается не для той платформы!
	sed -e 's/\(LDFLAGS_FOR_BUILD\) =.*/\1 = /' \
		-i $(WDIR)/builtins/Makefile

	# Инклюды они забывают...
	patch $(WDIR)/shell.h shell.h.patch

	# С типами у них тоже беда.
	patch $(WDIR)/lib/sh/oslib.c lib-sh-oslib.c.patch

	# Почему-то забыли мой libc!
	sed -e 's/\(LOCAL_LIBS\) =\(.*\)/\1 = -lc \2/' \
		-i $(WDIR)/Makefile

	cd $(WDIR) && make
	cd $(WDIR) && make install
