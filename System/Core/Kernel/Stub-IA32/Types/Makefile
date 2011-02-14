#
# Copyright (c) 2000-2011 Андрей Валяев <dron@securitycode.ru>
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
