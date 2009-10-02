#
# Copyright (c) 2000-2006 Андрей Валяев (dron@infosec.ru)
# All rights reserved.
#
# Created: 30/11/06 16:08:17
#

.SUFFIXES:

INCLUDEGEN = $(MDF_ROOT)/Develop/Utility/lt_includegen.pl
INCLUDEPATH = $(MDF_IMAGE)/Develop/Include

TARGETS = sys/types.h sys/stat.h fcntl.h argz.h sys/param.h sys/file.h \
	sys/time.h pwd.h sys/wait.h termio.h sys/times.h dirent.h sys/ioctl.h

all: $(addprefix $(INCLUDEPATH)/, $(TARGETS) )

$(INCLUDEPATH)/%.h : %.h
	$(ECHO) Generate $<...
	$(MKDIR) -p $(dir $@)
	$(INCLUDEGEN) -o $@ $<

$(INCLUDEPATH)/sys/%.h : %.h
	$(ECHO) Generate sys/$<...
	$(MKDIR) -p $(dir $@)
	$(INCLUDEGEN) -o $@ $<
