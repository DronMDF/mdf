#
# Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 28/08/07 22:08:12
#

# TODO: Это надо в конфиг
CD = cd

PREFIX = $(MDF_IMAGE)/System/Docs/Kernel
SOURCES = $(wildcard *.tex)
TARGET = $(PREFIX)/MDFKernelBook.pdf

all: $(TARGET)

$(MDF_TEMP)/MDFKernelBook.pdf: $(SOURCES) $(MDF_TEMP)/version.tex
	$(CP) $(SOURCES) $(MDF_TEMP)
	$(CD) $(MDF_TEMP) && pdflatex $(shell pwd)/Book.tex
	$(CD) $(MDF_TEMP) && pdflatex $(shell pwd)/Book.tex
	$(CP) $(MDF_TEMP)/Book.pdf $@

$(MDF_TEMP)/version.tex :
	$(ECHO) $(VERSION) > $@

$(PREFIX)/%.pdf: $(MDF_TEMP)/%.pdf
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
