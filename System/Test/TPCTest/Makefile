#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

PREFIX=$(MDF_IMAGE)/System/Test

all: 	$(PREFIX)/TPCTest $(PREFIX)/TPCTest.sym

$(PREFIX)/TPCTest: TPCTest.c
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/TPCTest.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/TPCTest.o -lc

$(PREFIX)/TPCTest.sym: $(PREFIX)/TPCTest
	$(ECHO) Create $@...
	$(NM) --defined-only $< > $@
	$(STRIP) -s $<
