#
# Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 07/08/07 00:41:30
#

PREFIX = $(MDF_IMAGE)/System/Core

SOURCES = $(basename $(wildcard *.cpp))
EXCLUDESOURCES =

ISOURCES = $(filter-out $(basename $(EXCLUDESOURCES)),$(SOURCES))

OBJECTS = $(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(ISOURCES)))

CORE = $(MDF_ROOT)/Develop/System/Core.o

all:	$(PREFIX)/Kernel-Linux

$(PREFIX)/Kernel-Linux : $(OBJECTS) $(MDF_TEMP)/libdiehard.a
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) -o $@ $(OBJECTS) $(MDF_TEMP)/libdiehard.a $(CORE) -lstdc++

$(MDF_TEMP)/%.o : %.cpp
	$(ECHO) Compile $<...
	$(MKDIR) -p $(dir $@)
	$(CXX) $(CXXFLAGS) -DVERSION=\"$(VERSION)\" -c $< -o $@

# DIEHARD stuff
$(MDF_TEMP)/libdiehard.a : $(PREFIX)/operm5.cov
	$(MAKE) -C diehard

$(PREFIX)/operm5.cov : diehard/operm5.cov
	$(ECHO) Copy $< to $@
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
