#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#

PREFIX = $(MDF_IMAGE)/System/Device

all: 	$(PREFIX)/Keyboard \
	$(PREFIX)/Keyboard.sym

$(PREFIX)/Keyboard: $(MDF_TEMP)/Keyboard.o $(MDF_TEMP)/i8042.o
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(LD) $(LDFLAGS) -o $@ \
		$(MDF_TEMP)/Keyboard.o $(MDF_TEMP)/i8042.o \
		-lNamer -lc

$(MDF_TEMP)/%.o: %.c
	$(ECHO) Compile $@...
	$(CC) -std=c99 $(CFLAGS) -DVERSION=\"$(VERSION)\" -o $@ -c $<

$(PREFIX)/Keyboard.sym: $(PREFIX)/Keyboard
	$(ECHO) Create $@...
	$(NM) --defined-only $< | sort > $@
	$(STRIP) -s $<
