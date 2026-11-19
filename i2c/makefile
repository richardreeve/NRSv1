# Tracker Makefile
# $Id: makefile,v 1.1.1.1 2003/07/21 14:46:57 darrens Exp $

# Available Targets are:
# $(BINDIR)/tracker.linux make executable for appropriate platform
# default: depends on whether -DKOALA is set (currently linux)
# all: everything
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

LDEFINES +=
# possible defines = -DDEBUG

LIBDIRS += -L$(LIBDIR) -L/usr/X11R6/lib

SOFTLIBS = 

LIBS += $(ALLEGRO_LIBS)

SOURCE = i2c.cc

#-------------------------------------------------------------------------

ifeq ($(TARGET), linux)
  ifneq ($(wildcard /lib/modules/*/build/include/linux/i2c-dev.h),)
    default: $(BINDIR)/tether.$(TARGET)
  else
    default:
	@echo "    .... No i2c in kernel, cannot compile for $(TARGET)"
  endif
else
  default:
	@echo "    .... No target in i2c/ for $(TARGET)"
endif

$(BINDIR)/tether.$(TARGET): $(OBJECT) $(SOFTDEPS)
	@echo "    ---- Building executable $(notdir $@)"
	$(SILENT) $(PURIFY) $(CXX) -o $@ $(LDOPTIONS) $(CXXFLAGS) \
		$(CFLAGS) $(OBJECT) $(LIBDIRS) $(SOFTLIBS) $(LIBS)

CLOBBERFILES = $(BINDIR)/../linux/tether.linux

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
