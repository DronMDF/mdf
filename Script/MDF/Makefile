#
# Copyright (c) 2000-2006 Andrey Valyaev (dron@infosec.ru)
# All rights reserved.
#
# Создано: Пятница 28 июля 2006
#

MODULES = $(wildcard *.pm)
PREFIX = $(MDF_IMAGE)/Develop/Library/Perl/MDF

all: $(addprefix $(PREFIX)/, $(MODULES))

$(PREFIX)/%.pm : %.pm
	mkdir -p $(dir $@)
	cp $< $@

