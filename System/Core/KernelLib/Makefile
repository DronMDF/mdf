#
# Copyright (c) 2000-2009 Андрей Валяев <dron@infosec.ru>
# This code is licenced under the GPL3 (http://www.gnu.org/licenses/#GPL)
#

PREFIX=$(MDF_IMAGE)/Develop

all: $(PREFIX)/Library/Startup.o

$(PREFIX)/Library/Startup.o : Startup.c
	$(ECHO) Create $@...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ -c $<
