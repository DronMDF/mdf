#
# Makefile for diehard
#

CC=gcc

SOURCES = $(basename $(wildcard *.c))
EXCLUDESOURCES =

ISOURCES = $(filter-out $(basename $(EXCLUDESOURCES)),$(SOURCES))

OBJECTS = $(addprefix $(MDF_TEMP)/, $(addsuffix .o, $(ISOURCES)))

$(MDF_TEMP)/libdiehard.a : $(OBJECTS)
	$(ECHO) Create library $@...
	$(MKDIR) -p $(dir $@)
	$(AR) -r $@ $(OBJECTS)
	$(RANLIB) $@

$(MDF_TEMP)/%.o : %.c
	$(ECHO) Compile $<...
	$(MKDIR) -p $(dir $@)
	$(CC) $(CCFLAGS) -DVERSION=\"$(VERSION)\" -c $< -o $@
