#
# Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

IFLAGS=-I $(MDF_ROOT)/Develop/Include/MDF --include Types.h

STUB_SOURCES = Allocator.c Descriptor.c IO.c Multiboot.c Utils.c
STUB_OBJECTS=$(addprefix stub_, $(addsuffix .o, $(basename $(STUB_SOURCES))))

UT_SOURCES=$(filter-out Runner.cpp, $(wildcard *.cpp))
UT_OBJECTS=$(addprefix test_, $(addsuffix .o, $(basename $(UT_SOURCES))))

OBJECTS=$(addprefix $(MDF_TEMP)/, $(STUB_OBJECTS) $(UT_OBJECTS))

.PHONY: test
test: $(MDF_TEMP)/Runner
	$(ECHO) Testing Stub-IA32
	$< --log_level=warning

$(MDF_TEMP)/Runner: $(OBJECTS)
	$(ECHO) Linking for localhost $@...
	$(HOST_CXX) $(HOST_CXXFLAGS) -o $@ Runner.cpp $(OBJECTS) \
		-lboost_unit_test_framework -lstdc++

$(MDF_TEMP)/test_%.o : %.cpp $(wildcard *.h ../*.h ../*.c)
	$(ECHO) Compile for localhost test/$<...
	$(HOST_CXX) $(HOST_CXXFLAGS) $(IFLAGS) -Wno-effc++ -c -o $@ $<

$(MDF_TEMP)/stub_%.o : ../%.c $(wildcard ../*.h)
	$(ECHO) Compile for localhost $(notdir $<)...
	$(HOST_CC) $(HOST_CFLAGS) $(IFLAGS) -c -o $@ $<

