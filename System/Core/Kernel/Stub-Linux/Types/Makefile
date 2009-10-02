#
# Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 07/08/07 00:40:52
#

PREFIX = $(MDF_IMAGE)/Develop/Include/MDF
HEADER = Stub-Linux.h
TARGET = $(PREFIX)/Stub.h

all: $(TARGET)

$(TARGET) : $(PREFIX)/$(HEADER)
	$(ECHO) "Symlink $(HEADER) -> Stub.h..."
	$(MKDIR) -p $(dir $@)
	$(SYMLINK) $(HEADER) $@

$(PREFIX)/$(HEADER) : $(HEADER)
	$(ECHO) Copy $<...
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
