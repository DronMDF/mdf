#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 19/09/06 20:57:05
#

.SUFFIXES:

export IPREFIX = $(MDF_IMAGE)/Develop/Include
export LPREFIX = $(MDF_IMAGE)/Develop/Library
INCLUDEGEN = $(MDF_ROOT)/Develop/Utility/lt_includegen.pl
UNITGEN = $(MDF_ROOT)/Develop/Utility/lt_unitgen.pl

export TARGET = $(LPREFIX)/libUnicode.a

IPATT = $(shell ls *.h.in)
INCLUDES = $(addprefix $(IPREFIX)/MDF/, $(IPATT:.h.in=.h))

SOURCES = $(shell find ./Units/ -name *.xml)
export UNITS = $(addprefix $(MDF_TEMP)/, $(addsuffix .c, $(shell $(UNITGEN) --list $(SOURCES))))

all: $(TARGET)

$(IPREFIX)/MDF/%.h : %.h.in
	$(ECHO) Generate $@...
	$(MKDIR) -p $(dir $@)
	$(INCLUDEGEN) -o $@ $<

$(MDF_TEMP)/Makefile : Makefile.stage2
	$(CP) $< $@

$(UNITS) : $(MDF_TEMP)/Makefile $(SOURCES) $(INCLUDES)
	$(ECHO) Generate unit sources...
	$(UNITGEN) --targetdir "$(MDF_TEMP)" --makefile --makepub "$<" $(SOURCES)

$(TARGET) : $(MDF_TEMP)/Makefile $(UNITS)
	$(MAKE) -C $(MDF_TEMP)
