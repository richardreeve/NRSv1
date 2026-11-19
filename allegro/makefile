# Allegro Makefile
# $Id: makefile,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

# Available Targets are:
# $(PROJDIR)/include/font_times.h
# default: as above
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

# All directories to be compiled in a standard make should be here

LDEFINES +=

SOFTLIBS = 

SOURCE = font_times.dat

#-------------------------------------------------------------------------

ifeq ($(TARGET), linux)
  ifdef ALLEGRO_PRESENT
    default: $(PROJDIR)/include/font_times.h
  else
  default:
	@echo "    ++++ Allegro not present, skipping allegro/ directory"
  endif
else
  default:
	@echo "    .... No target in allegro/ for $(TARGET)"
endif

$(PROJDIR)/include/font_times.h: $(DEPEND) $(SOURCE)
		       @echo "    ---- Building allegro headers"
		       $(SILENT) dat -h $@ $(SOURCE)


CLOBBERFILES = # $(PROJDIR)/include/font_times.h

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
