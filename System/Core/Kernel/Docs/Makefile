#
# Copyright (c) 2000-2007 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 28/08/07 22:08:12
#

PREFIX = $(MDF_IMAGE)/System/Docs/Kernel

# TODO: Это надо в конфиг
CD = cd

all:

$(MDF_TEMP)/version.tex :
	$(ECHO) $(VERSION) > $@

$(MDF_TEMP)/%.pdf: %.tex $(MDF_TEMP)/version.tex
	$(CD) $(MDF_TEMP) && pdflatex $(shell pwd)/$<
	$(CD) $(MDF_TEMP) && pdflatex $(shell pwd)/$<

$(PREFIX)/%.pdf: $(MDF_TEMP)/%.pdf
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
