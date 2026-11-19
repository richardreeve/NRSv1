# Downloader main Makefile
# $Id: makefile,v 1.2 2003/09/19 11:19:14 darrens Exp $

# Available Targets are:
# $(BINDIR)/download.$(TARGET) make executable for appropriate platform
# default: as above
# all: everything
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

# All directories to be compiled in a standard make should be here

LDEFINES +=

LIBDIRS += -L$(LIBDIR)

LIBS += -lxml2 -lz

 SOFTLIBS = -lInterface -lUserInterface -lNode  -lDerivedNode -lSensor -lMotor -lPeripheral -lXML

SOURCE = main_$(TARGET).c

#-------------------------------------------------------------------------

ifeq ($(TARGET), linux)
  default: $(BINDIR)/download.$(TARGET)
else
  default:
	@echo "    .... No target in download/ for $(TARGET)"
endif

$(BINDIR)/download.$(TARGET): $(OBJECT) $(SOFTDEPS)
	@echo "    ---- Building executable $(notdir $@)"
	$(SILENT) $(PURIFY) $(CXX) -o $@ $(LDOPTIONS) \
		$(CFLAGS) $(OBJECT) $(LIBDIRS) $(SOFTLIBS) $(LIBS)

CLOBBERFILES = $(BINDIR)/../linux/download.linux

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
