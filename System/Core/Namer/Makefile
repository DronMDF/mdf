#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Created: Пнд Июн 13 22:09:54 2005
#

PREFIX = $(MDF_IMAGE)/System/Core

all: 	$(PREFIX)/Namer \
	$(PREFIX)/Namer.sym

$(PREFIX)/Namer: Namer.c
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/Namer.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/Namer.o -lc

$(PREFIX)/Namer.sym: $(PREFIX)/Namer
	$(ECHO) Create $@...
	$(NM) --defined-only $< | sort > $@
	$(STRIP) -s $<
