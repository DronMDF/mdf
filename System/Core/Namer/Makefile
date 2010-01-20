#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
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
