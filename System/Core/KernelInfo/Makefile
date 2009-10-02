#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Created 05 August 2001 15:30:33
#

all: $(MDF_IMAGE)/System/Core/KernelInfo \
	$(MDF_IMAGE)/System/Core/KernelInfo.sym

$(MDF_IMAGE)/System/Core/KernelInfo: KernelInfo.c
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/KernelInfo.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/KernelInfo.o -lc

$(MDF_IMAGE)/System/Core/KernelInfo.sym: $(MDF_IMAGE)/System/Core/KernelInfo
	$(ECHO) Create $@...
	$(NM) --defined-only $< >$@
	$(STRIP) -s $<
