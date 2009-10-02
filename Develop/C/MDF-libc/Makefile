#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 27/11/06 20:49:38
#

IPREFIX = $(MDF_IMAGE)/Develop/Include
LPREFIX = $(MDF_IMAGE)/Develop/Library

TARGET = $(LPREFIX)/libc.a
INCLUDES = $(addprefix $(IPREFIX)/, $(wildcard *.h))

SOURCES = $(basename $(wildcard *.c))
OBJECTS = $(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(SOURCES)))

all: test $(TARGET) $(INCLUDES)

$(IPREFIX)/%.h : %.h
	$(ECHO) Install $(notdir $@)...
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@

$(TARGET) : $(OBJECTS)
	$(ECHO) Create library $(notdir $@)...
	$(MKDIR) -p $(dir $@)
	$(AR) -r $@ $(OBJECTS)
	$(RANLIB) $@

$(MDF_TEMP)/%.o : %.c
	$(ECHO) Compile $<...
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

.PHONY: test
test:
	$(MAKE) -C test
