#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

SOURCES = $(basename $(wildcard *.cpp))
OBJECTS = $(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(SOURCES)))

PREFIX = $(MDF_IMAGE)/Develop

# Объектный файл коры складываем в зависимости от архитектуры.
OPREFIX = $(PREFIX)/Library/$(ARCH)

# Инклюд вполне себе платформонезависим.
IPREFIX = $(PREFIX)/Include

CXXFLAGS += -fexceptions -frtti -DCORE -include "$(MDF_DEV)/Include/$(ARCH)/MDF/Types.h"

all: $(OPREFIX)/MDF/Core.o $(IPREFIX)/MDF/Kernel.h \
	$(IPREFIX)/MDF/Core.h $(IPREFIX)/MDF/Stub.h

$(OPREFIX)/MDF/Core.o : test $(OBJECTS)
	$(ECHO) Link $(notdir $@)...
	$(MKDIR) -p $(dir $@)
	$(LD) -Ur -o $@ $(OBJECTS)

$(IPREFIX)/MDF/%.h : include/%.h
	$(ECHO) Install $(notdir $@)...
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@

$(MDF_TEMP)/%.o : %.cpp $(wildcard *.h)
	$(ECHO) Compile $<...
	$(CXX) $(CXXFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

.PHONY: test
test:
	$(MAKE) -C test
