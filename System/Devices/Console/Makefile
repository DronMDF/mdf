#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

PREFIX = $(MDF_IMAGE)/System/Device

all: 	$(PREFIX)/Console \
	$(PREFIX)/Console.sym

$(PREFIX)/Console: Console.c
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) -std=c99 $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/Console.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/Console.o -lNamer -lc

$(PREFIX)/Console.sym: $(PREFIX)/Console
	$(ECHO) Create $@...
	$(NM) --defined-only $< | sort > $@
	$(STRIP) -s $<
