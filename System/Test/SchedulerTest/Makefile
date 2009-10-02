#
# Copyright (c) 2000-2005 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Created 05 August 2001 15:30:33
#

all: $(MDF_IMAGE)/System/Test/SchedulerTest \
	$(MDF_IMAGE)/System/Test/SchedulerTest.sym

$(MDF_IMAGE)/System/Test/SchedulerTest: SchedulerTest.c $(MDF_TEMP)/unittest
	# Прогоняем тесты
	$(MDF_TEMP)/unittest
	# а потом уже линкуем...
	$(ECHO) Link $@...
	$(MKDIR) -p $(dir $@)
	$(CC) -std=c99 $(CFLAGS) -DVERSION=\"$(VERSION)\" \
		-o $(MDF_TEMP)/SchedulerTest.o -c $<
	$(LD) $(LDFLAGS) -o $@ $(MDF_TEMP)/SchedulerTest.o

$(MDF_IMAGE)/System/Test/SchedulerTest.sym: $(MDF_IMAGE)/System/Test/SchedulerTest
	$(ECHO) Create $@...
	$(NM) --defined-only $< >$@
	$(STRIP) -s $<

$(MDF_TEMP)/unittest: SchedulerTest.c
	$(HOST_CXX) $(HOST_CXXFLAGS) -std=c++98 -DTEST -o $@ $< \
		-lboost_unit_test_framework -lstdc++
