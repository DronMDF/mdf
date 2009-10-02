#
# Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

PREFIX = $(MDF_IMAGE)/System/Core

all: 	$(PREFIX)/Shell \
	$(PREFIX)/Shell.sym

$(PREFIX)/Shell: Shell.c
	$(MKDIR) -p $(dir $@)
	$(CC) -std=c99 $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/Shell.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/Shell.o -lNamer -lc

$(PREFIX)/Shell.sym: $(PREFIX)/Shell
	$(NM) --defined-only $< | sort > $@
	$(STRIP) -s $<
