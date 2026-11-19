# Calibrate Makefile
# $Id: makefile,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

# Available Targets are:
# $(BINDIR)/calibrate.linux make executable for appropriate platform
# default: depends on whether -DKOALA is set (currently linux)
# all: everything
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

LDEFINES +=
# possible defines = -DDEBUG

LIBDIRS += -L$(LIBDIR) -L/usr/X11R6/lib

SOFTLIBS = -lCamera

LIBS += $(ALLEGRO_LIBS)

SOURCE = calibrate.cc track.cc

#-------------------------------------------------------------------------

ifeq ($(TARGET), linux)
  ifdef ALLEGRO_PRESENT
    default: $(BINDIR)/calibrate.$(TARGET)
  else
  default:
	@echo "    ++++ Allegro not present, skipping calibrate.$(TARGET)"
  endif
else
  default:
	@echo "    .... No target in calibrate/ for $(TARGET)"
endif

$(BINDIR)/calibrate.$(TARGET): $(DEPEND) $(OBJECT) $(SOFTDEPS)
	@echo "    ---- Building executable $(notdir $@)"
	$(SILENT) $(PURIFY) $(CXX) -o $@ $(LDOPTIONS) $(CXXFLAGS) \
		$(CFLAGS) $(OBJECT) $(LIBDIRS) $(SOFTLIBS) $(LIBS)

CLOBBERFILES = $(BINDIR)/../linux/calibrate.linux

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
