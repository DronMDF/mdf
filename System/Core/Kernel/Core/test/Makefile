#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

CORE_SOURCES=$(notdir $(wildcard ../*.cpp)))
CORE_OBJECTS=$(addprefix test_core_, $(basename $(CORE_SOURCES)))

UT_SOURCES=$(wildcard *.cpp)
UT_OBJECTS=$(addprefix test_, $(basename $(UT_SOURCES)))

OBJECTS=$(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(CORE_OBJECTS) $(UT_OBJECTS)))

.PHONY: test
test: $(MDF_TEMP)/Runner
	$(ECHO) Testing Core
	$< --log_level=warning --random=1

$(MDF_TEMP)/Runner: $(OBJECTS)
	$(ECHO) Linking for localhost $@...
	$(HOST_CXX) $(HOST_CXXFLAGS) -o $@ $(OBJECTS) \
		-lboost_unit_test_framework -lstdc++

$(MDF_TEMP)/test_%.o : %.cpp $(wildcard *.h) $(wildcard ../*.h)
	$(ECHO) Compile for localhost test/$<...
	$(HOST_CXX) $(HOST_CXXFLAGS) -Wno-effc++ -c -o $@ $<

$(MDF_TEMP)/test_core_%.o : ../%.cpp  $(wildcard ../*.h)
	$(ECHO) Compile for localhost $(notdir $<)...
	$(HOST_CXX) $(HOST_CXXFLAGS) --include Types.h \
		-DVERSION=\"$(VERSION)-test\" -c -o $@ $<
