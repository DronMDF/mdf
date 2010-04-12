#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

PREFIX = $(MDF_IMAGE)/System/Core

SOURCES = $(basename $(wildcard *.S *.c *.cpp))
EXCLUDESOURCES =

ISOURCES = $(filter-out $(basename $(EXCLUDESOURCES)),$(SOURCES))

OBJECTS = $(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(ISOURCES)))

CORE = $(MDF_ROOT)/Develop/Library/IA32/MDF/Core.o

FLAGS=-DSTUB

# Типы преинклюдятся...
FLAGS+= --include "$(MDF_DEV)/Include/IA32/MDF/Types.h"

# Стандартные инклюды нафиг не нужны, мои нужны.
FLAGS+= -I $(MDF_DEV)/Include/MDF

# Очень много ругани - пока поосторожничаем.
FLAGS += -Wno-conversion -Wno-sign-conversion

CFLAGS += ${FLAGS}
CXXFLAGS += ${FLAGS}

all:	$(PREFIX)/Kernel-IA32		\
	$(PREFIX)/Kernel-IA32.sym	\

$(PREFIX)/Kernel-IA32 : test $(OBJECTS)
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(LD) -T Stub.ld -o $@ $(OBJECTS) $(CORE)

$(PREFIX)/Kernel-IA32.sym : $(PREFIX)/Kernel-IA32
	$(ECHO) Create $@...
	$(MKDIR) -p $(dir $@)
	$(NM) -C -n $< > $@

$(MDF_TEMP)/%.o : %.c
	$(ECHO) Compile $<...
	$(CC) $(CFLAGS) -DVERSION=\"$(VERSION)\" -c $< -o $@

$(MDF_TEMP)/%.o : %.cpp
	$(ECHO) Compile $<...
	$(CXX) $(CXXFLAGS) -DVERSION=\"$(VERSION)\" -c -o $@ $<

$(MDF_TEMP)/%.o : %.S
	$(ECHO) Compile $<...
	$(AS) $(ASFLAGS) $< -o $@

.PHONY: test
test:
	$(MAKE) -C Test
