#
# Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 26/06/07 22:27:27
#

PREFIX = $(MDF_IMAGE)/System/Test/Kernel

.PHONY: all TestPCall

all: TestPCall

TestPCall: $(PREFIX)/TestPCall $(PREFIX)/TestPCall.sym

$(PREFIX)/TestPCall: TestPCall.c
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/TestPCall.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/TestPCall.o -lc

$(PREFIX)/TestPCall.sym: $(PREFIX)/TestPCall
	$(ECHO) Create $@...
	$(NM) --defined-only $< | $(SORT) > $@
	$(STRIP) -s $<
