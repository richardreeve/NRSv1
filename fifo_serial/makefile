# Main Makefile
# $Id: makefile,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

# Available Targets are:
# $(MODDIR)/fifo_serial.o makes a serial module for linux
# default: depends on whether -DKOALA is set (currently linux)
# all: everything
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk
# add in lots of real time things here
ifdef RTL_PRESENT
  include $(RTLDIR)/rtl.mk
endif
#-------------------------------------------------------------------------

# All directories to be compiled in a standard make should be here

LDEFINES +=
# possible defines = -DDEBUG -DKOALA -DKHEPERA

INCDIRS += $(INCLUDE)

SOFTLIBS =

SOURCE = fifo_serial.c
#-------------------------------------------------------------------------
ifeq ($(TARGET), linux)
  ifdef RTL_PRESENT
    default: $(MODDIR)/fifo_serial.d $(MODDIR)/fifo_serial.o
  else
  default:
	@echo "    ++++ RTL not present, skipping fifo_serial/ directory"
  endif
else
  default:
	@echo "    .... No target in fifo_serial/ for $(TARGET)"
endif

CLOBBERFILES = $(MODDIR)/fifo_serial.o $(MODDIR)/fifo_serial.d

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(MODDIR)/fifo_serial.d),)
  include $(wildcard $(MODDIR)/fifo_serial.d)
endif
#ifneq ($(wildcard $(RTL_DIR)/.hdepend),)
#  include $(RTL_DIR)/.hdepend
#endif
