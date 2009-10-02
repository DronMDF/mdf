#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Создано: Втр Июн 14 22:07:40 2005
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
