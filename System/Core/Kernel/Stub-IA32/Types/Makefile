#
# Copyright (c) 2000-2010 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

PREFIX = $(MDF_IMAGE)/Develop/Include/$(ARCH)/MDF

all: $(PREFIX)/Types.h $(PREFIX)/KernelImp.h

$(PREFIX)/Types.h : Types.h
	$(ECHO) Copy $<...
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@

$(PREFIX)/KernelImp.h : Interface.h
	$(ECHO) Copy $<...
	$(MKDIR) -p $(dir $@)
	$(CP) $< $@
