# Gui Makefile
# $Id: makefile,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

# Available Targets are:
# $(LIBDIR)/libgui.a make library for appropriate platform
# default: depends on whether -DKOALA is set (currently linux)
# all: everything
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

# All directories to be compiled in a standard make should be here

LDEFINES +=
# possible defines = -DDEBUG -DKOALA -DKHEPERA

LIBDIRS += -L$(LIBDIR)

SOFTLIBS =

SOURCE = dialog.c
#-------------------------------------------------------------------------
ifeq ($(TARGET), linux)
  ifdef ALLEGRO_PRESENT
    default: $(LIBDIR)/libgui.a
  else
  default:
	@echo "    ++++ Allegro not present, skipping gui/ directory"
  endif
else
  default:
	@echo "    .... No target in gui/ for $(TARGET)"
endif

$(LIBDIR)/libgui.a: $(DEPEND) $(OBJECT)
		       	@echo "    ---- Building library\
				$(notdir $@) $(patsubst %, with %,\
				$(patsubst -D$(TARGET),,\
				  $(patsubst -DPROJDIR%,,$(LDEFINES))))"
		       	$(SILENT) $(AR) $(AR_FLAGS) $@ $(OBJECT)

CLOBBERFILES = $(LIBDIR)/../linux/libgui.a

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
