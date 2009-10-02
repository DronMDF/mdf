#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

UT_SOURCES=$(basename $(wildcard *.cpp))
LIB_SOURCES=$(basename $(notdir $(wildcard ../*.c)))

OBJECTS=$(addprefix $(MDF_TEMP)/test_, $(addsuffix .o, $(UT_SOURCES))) \
	$(addprefix $(MDF_TEMP)/test_lib_, $(addsuffix .o, $(LIB_SOURCES)))

.PHONY: test
test: $(MDF_TEMP)/test_runner
	$(ECHO) Testing Core
	$<

$(MDF_TEMP)/test_runner: $(OBJECTS)
	$(ECHO) Linking for localhost $(notdir $@)...
	# TODO: Может бытть здесь следует написать HOST_LD?
	$(HOST_CXX) -o $@ $(OBJECTS) -lboost_unit_test_framework -lstdc++

$(MDF_TEMP)/test_%.o : %.cpp
	$(ECHO) Compile for localhost test/$<...
	$(HOST_CXX) $(HOST_CXXFLAGS) -fno-builtin -c -o $@ $<

$(MDF_TEMP)/test_lib_%.o : ../%.c
	$(ECHO) Compile for localhost $(notdir $<)...
	$(HOST_CC) $(HOST_CFLAGS) -fno-builtin -DVERSION=\"$(VERSION)-test\" -c -o $@ $<
