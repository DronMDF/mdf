#
# Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Создано: Пятница 28 июля 2006
#

PREFIX = $(MDF_IMAGE)/Develop/Utility
UTILITY = lt_includegen.pl lt_unitgen.pl

all: $(addprefix $(PREFIX)/,$(UTILITY))

$(PREFIX)/%.pl : %.pl $(MDF_TEMP)/%.pl.tested
	mkdir -p $(dir $@)
	cp $< $@

$(MDF_TEMP)/lt_includegen.pl.tested : lt_includegen.pl
	./$< -o $(MDF_TEMP)/test.h Test/test.h.in
	diff -bB $(MDF_TEMP)/test.h Test/test.h.out
	touch $@

$(MDF_TEMP)/lt_unitgen.pl.tested : lt_unitgen.pl $(MDF_TEMP)/lt_unitgen.pl.list.tested
	./$< --targetdir=$(MDF_TEMP) \
		--makefile --makepub=$(MDF_TEMP)/Makefile \
		./Test/unit.xml ./Test/unit2.xml

	diff -bB $(MDF_TEMP)/testunit.c Test/testunit.c.out
	diff -bB $(MDF_TEMP)/testunit_test_main.c Test/testunit_test_main.c.out
	diff -bB $(MDF_TEMP)/testunit_test_extra.c Test/testunit_test_extra.c.out
	diff -bB $(MDF_TEMP)/Makefile.testunit Test/Makefile.testunit.out

	diff -bB $(MDF_TEMP)/test2.c Test/test2.c.out
	if [ -e $(MDF_TEMP)/test2_test_main.c ]; then false; fi
	if [ -e $(MDF_TEMP)/test2_test_extra.c ]; then false; fi
	diff -bB $(MDF_TEMP)/Makefile.test2 Test/Makefile.test2.out

	diff -bB $(MDF_TEMP)/test3.c Test/test3.c.out
	if [ -e $(MDF_TEMP)/test3_test_main.c ]; then false; fi
	if [ -e $(MDF_TEMP)/test3_test_extra.c ]; then false; fi
	diff -bB $(MDF_TEMP)/Makefile.test3 Test/Makefile.test3.out

	diff -bB $(MDF_TEMP)/Makefile Test/Makefile.out
	diff -bB $(MDF_TEMP)/test.h Test/test.h.pp.out

	# Еще надо попробовать промейкать, чтобы убедиться что все работает.
	make -C $(MDF_TEMP) testunit.o test2.o

	touch $@

$(MDF_TEMP)/lt_unitgen.pl.list.tested : lt_unitgen.pl
	./$< --list ./Test/unit.xml ./Test/unit2.xml > $(MDF_TEMP)/unit.list
	diff -bB $(MDF_TEMP)/unit.list Test/unit.list.out
	touch $@
