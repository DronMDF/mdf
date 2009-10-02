#
# Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

PREFIX = $(MDF_IMAGE)/Develop

all: 	$(PREFIX)/Library/libNamer.a \
	$(PREFIX)/Include/MDF/Namer.h

$(PREFIX)/Library/libNamer.a: NamerCall.c
	$(CC) -std=c99 $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/NamerCall.o -c $<
	$(MKDIR) -p $(dir $@)
	$(AR) r $@ $(MDF_TEMP)/NamerCall.o
	$(RANLIB) $@

$(PREFIX)/Include/MDF/Namer.h: Namer.h
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
