#-------------------------------------------------------------------------
# Makefile for Key/Value pair library and test programs
#
# Author : G.J.Crone
# Stolen by : R. Nichol
#
#
#-------------------------------------------------------------------------

include ${ANITA_FLIGHT_SOFT_DIR}/standard_definitions.mk

LIB_OBJS         =  keyValuePair.o

Name = libkvp
DynLib  = $(ANITA_LIB_DIR)/$(Name).${DllSuf}
Library = $(ANITA_LIB_DIR)/$(Name).a

all: $(Library) $(DynLib)

$(Library): $(LIB_OBJS)
	@/bin/rm -f $(Library)	
	@echo "Linking $(Library) ..."
	@ar -r $@ $^

$(DynLib): $(LIB_OBJS)
	@/bin/rm -f $(DynLib)	
	@echo "Creating $(DynLib) ..."
	@$(LD) $(LDFLAGS) $(LIBS) $(SOFLAGS) $(LIB_OBJS) -o $(DynLib)
	@chmod 555 $(DynLib)

clean: objclean
	@/bin/rm -f $(Library) $(DynLib)
