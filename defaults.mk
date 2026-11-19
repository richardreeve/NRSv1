# $Id: defaults.mk,v 1.4 2004/05/31 17:17:11 richardr Exp $
ifdef SOFTHOME
  ifneq ($(SOFTHOME), $(wildcard $(SOFTHOME)))
    $(error Environment variable $$SOFTHOME=$(SOFTHOME) - this does not exist)
  endif
else
  ifneq ($(wildcard /home/$(USER)/software),)
    SOFTHOME=/home/$(USER)/software
  else
    ifneq ($(wildcard ~/software),)
      SOFTHOME=~/software
    else
      $(error If you do not wish to use ~/softhome as the software install \
directory, you must set the environment variable $$SOFTHOME)
    endif
  endif
endif

# Directory to store all executables (set to . to store locally)
BINDIR = $(SOFTHOME)/bin/$(TARGET)

# Directory to store all libraries (must be a single directory for all
# or they will not be found during linking)
LIBDIR = $(SOFTHOME)/lib/$(TARGET)

# Directory to store kernel modules
MODDIR = $(SOFTHOME)/lib/modules/$(TARGET)

# Directory to store all documentation
DOCDIR = $(SOFTHOME)/doc

# All possible library directories needed by software not on standard path
LIBDIRS = 

# Project name
PROJNAME = nrs
# Root directory for software
PROJDIR = $(SOFTHOME)/$(PROJNAME)

# Root directory for cross compilers
ifeq ($(shell rpm -q kteam | grep not),)
  X_COMPILER_PRESENT = true
  KTEAM_CONFIG := $(shell rpm -ql kteam | grep kteam-config)
endif

# All possible include directories not on standard path
INCDIRS = -I $(PROJDIR)/include #-I /usr/local/include/ #\
#	  -I /usr/local/include/g++-v3 \
#	  -I /usr/local/include/g++-v3/i686-pc-linux-gnu \
#	  -I /usr/src/linux/include

# who and what is the main host for the software?
BASEHOST = taxis.stir.ac.uk
BASEHOME = /home/richardr/software
BASEUSER = richardr


ifeq ($(shell rpm -q allegro),)
  ALLEGRO_PRESENT = true
  ALLEGRO_CONFIG := $(shell rpm -ql allegro | grep allegro-config)
  ALLEGRO_LIBS := $(shell $(ALLEGRO_CONFIG) --shared)
else
  ifneq ($(wildcard /usr/bin/allegro-config /usr/local/bin/allegro-config),)
    ALLEGRO_PRESENT = true
    ALLEGRO_CONFIG = $(firstword $(wildcard /usr/bin/allegro-config \
                                            /usr/local/bin/allegro-config))
    ALLEGRO_LIBS := $(shell $(ALLEGRO_CONFIG) --shared)
  endif
endif


ifdef RTLDIR
  ifeq ($(RTLDIR), $(wildcard $(RTLDIR)))
    RTL_PRESENT=true
  endif
endif

# Default executables - change as required
# ----------------------------------------
SHELL=/bin/sh

LINUX_CC  = /usr/bin/gcc #/usr/bin/gcc-2.96 #gcc #-Wfloat-equal
MODULE_CC  = /usr/bin/gcc-2.96 #gcc #-Wfloat-equal
LINUX_CXX = /usr/bin/g++ #/usr/bin/g++-2.96 #g++ #-Wfloat-equal
LINUX_AR  = ar
#LINUX_AR_FLAGS = -rucv
LINUX_AR_FLAGS = -rc

YACC = /usr/bin/bison
LEX = /usr/bin/flex
FIG2DEV = fig2dev
EPSTOPDF = /usr/bin/epstopdf
LATEX = /usr/bin/latex
BIBTEX = /usr/bin/bibtex
PDFLATEX = /usr/bin/pdflatex
DVIPS = /usr/bin/dvips
MV = /bin/mv
RM = /bin/rm
BASENAME=/bin/basename
PWD=/bin/pwd
GREP=/bin/grep
XARGS=/usr/bin/xargs

##
## Commented out by DJS because it is already defined above.
##
#KTEAM_CONFIG:=$(shell rpm -ql kteahm | grep kteam-config)

##
## Only set these if KTEAM_CONFIG is set. Otherwise we get 
## errors "Command not found" 
##
ifneq ($(KTEAM_CONFIG),)
  ROBOT_CC := $(shell $(KTEAM_CONFIG) --cc)
  ROBOT_AR := $(shell $(KTEAM_CONFIG) --ar)
  #ROBOT_AR_FLAGS = -rsuc
  ROBOT_AR_FLAGS := $(shell $(KTEAM_CONFIG) --arflags)
endif


PURIFY = #purify
MAKE = make
MIRRORDIR = mirrordir

# depends on targets
CC = $(firstword $(patsubst %,$(ROBOT_CC),$(KHEPERA)$(KOALA)) $(LINUX_CC))
AR = $(firstword $(patsubst %,$(ROBOT_AR),$(KHEPERA)$(KOALA)) $(LINUX_AR))
CXX = $(LINUX_CXX)
AR_FLAGS = $(firstword $(patsubst %, $(ROBOT_AR_FLAGS),$(KHEPHERA)$(KOALA)) \
                                              $(LINUX_AR_FLAGS))
# my programs
WAITKEY=$(SOFTHOME)/bin/linux/waitkey

# Miscellaneous
# -------------
CFLAGS = -Wall -O2 \
	 $(patsubst khepera, $(KHE_CFLAGS), $(KHEPERA)) \
	 $(patsubst koala, $(KOA_CFLAGS), $(KOALA))

ifdef DEBUG
 CFLAGS += -g
endif

ifdef PROFILE
 CFLAGS += -pg
endif


CFLAGS += -DTRACE

# only needed for depends...
#CXXFLAGS = -I /usr/local/include/g++-v3/ -I /usr/include/g++-2/ 
CXXFLAGS = #-I /usr/include/g++-2/ 

#Builds non standard library paths into executable and stuff for koala/khepera
LDOPTIONS = $(patsubst khepera, $(KHE_LDFLAGS_SREC), $(KHEPERA)) \
	    $(patsubst koala, $(KOA_LDFLAGS_SREC), $(KOALA)) \
	    # $(LIBDIRS:-L%=-R%)


# Libraries NOT compiled here needed by all executables
LIBS = $(patsubst khepera,$(KHE_LIBS),$(KHEPERA)) \
       $(patsubst koala,$(KOA_LIBS),$(KOALA))
       #-lm

# are we using purify
PURIFY = #purify

# Set to @ to avoid seeing compilation commands
SILENT = @

# -----------------------------------------------
# NO CHANGES SHOULD BE NECESSARY BELOW THIS POINT
# -----------------------------------------------

# Dependent subdirectories (adjust in local makefiles)
DIRS =
# Executable/library files created by Makefile (adjust in local makefiles)
CLOBBERFILES =
# defines changed in local makefile
LDEFINES = $(DEFINES) -DPROJDIR=\"$(PROJDIR)/\" 
ifdef RTL_PRESENT
  LDEFINES += -DRTL_PRESENT
endif
ifdef ALLEGRO_PRESENT
  LDEFINES += -DALLEGRO_PRESENT
endif
ifdef DEBUG
  LDEFINES += -DDEBUG
endif
# Libraries compiled here needed in specific executables
SOFTLIBS =
# Files associated with required libraries
SOFTDEPS = $(SOFTLIBS:-l%=$(LIBDIR)/lib%.a)

# all possible target
TARGETS = koala khepera linux

# KOALA = 'koala' if -DKOALA (otherwise blank)
KOALA = $(firstword $(patsubst %,koala,$(filter -DKOALA, $(DEFINES))))

# KHEPERA = 'khepera' if -DKHEPERA (otherwise blank)
KHEPERA = $(firstword $(patsubst %,khepera,$(filter -DKHEPERA, $(DEFINES))))

# TARGET = 'koala', 'khepera' or 'linux'
#MID_TARGET = $(patsubst %koala,koala,linux$(KHEPERA)$(KOALA))
#TARGET = $(patsubst %khepera,khepera,$(MID_TARGET))
TARGET = $(firstword $(KHEPERA) $(KOALA) linux)
# OBJDIR is the subdirectory to place the object files in
OBJDIR = $(TARGET)

# Robot stuff
# -----------
# Assembler Options :
#          --register-prefix-optional : avoid adding a % in front of
#                       register name (%a0-%a7,%d0-%d7)
#          -pic       : position independent code (work only for 68030/40)
ROBOT_ASFLAGS = -Wa,--register-prefix-optional 

ifneq ($(KTEAM_CONFIG),)
  ROBOT_INC := $(shell $(KTEAM_CONFIG) --include)
endif
#ROBOT_INC = -nostdinc -I$(PROJDIR)/stdinc

# Archiver Options :
#          -s         : write object files into archive
#          -r         : replace files in archive if already present

# Koala stuff
# -----------

# Linker Options :
#          -nostdlib  : Do not use standard gcc libraries
#          -s         : Omits all symbol info from output file
#          -Tfilename : Use filename as a ld script file
#          -bieee     : Output format IEEE
#          -bbinary   : Output format binary
#          -bsrec     : Output format S-record

ifneq ($(KTEAM_CONFIG),)
  KOA_LDFLAGS_SREC := $(shell $(KTEAM_CONFIG) --koala --ldflags)
endif
#KOA_LDFLAGS_SREC = -nostdlib -L$(PROJDIR)/lib \
#	-Wl,-s,-Tukoacode,-bsrec,$(PROJDIR)/lib/ukoacrt0.o

# Compiler Options
#          -m68332    : use the CPU32 extension (do not work properly)
#          -mno-68851 : do not use any mmu functions
#          -msoft-float : do not use the floating point functions

#KOA_CFLAGS = $(ROBOT_INC) $(ROBOT_ASFLAGS) \
#	     -m68000 -mno-68851 -msoft-float -fno-builtin

ifneq ($(KTEAM_CONFIG),)
  KOA_CFLAGS := $(shell $(KTEAM_CONFIG) --koala --cflags)
  KOA_LIBS := $(shell $(KTEAM_CONFIG) --koala --libs)
endif
#KOA_LIBS = -lkoa

# Khepera stuff
# --------------

#KHE_LDFLAGS_SREC = -nostdlib -L$(PROJDIR)/lib \
#	-Wl,-s,-Tukhecode,-bsrec,$(PROJDIR)/lib/ukhecrt0.o

#KHE_CFLAGS = $(ROBOT_INC) $(ROBOT_ASFLAGS) \
#	     -m68000 -mno-68851 -msoft-float -fno-builtin

#KHE_LIBS = -lkhe

ifneq ($(KTEAM_CONFIG),)
  KHE_LDFLAGS_SREC := $(shell $(KTEAM_CONFIG) --khepera --ldflags)
  KHE_CFLAGS := $(shell $(KTEAM_CONFIG) --khepera --cflags)
  KHE_LIBS := $(shell $(KTEAM_CONFIG) --khepera --libs)
endif

# Defaults for Compiling
# ----------------------
SOURCES = $(SOURCE) $(SOURCE2) $(SOURCE3) $(SOURCE4)

TMPOBJECT = $(addprefix obj/$(OBJDIR)/, $(patsubst %.c,%.o,\
              $(patsubst %.cc,%.o,$(patsubst %.fig,%.eps,\
	        $(patsubst %.tex,%.tex,\
                  $(patsubst %.eps,%.eps,$(patsubst %.y,%.tab.c,\
                    $(patsubst %.lex,%.tab.c,$(SOURCE)))))))) \
		      $(patsubst %.y, %.tab.c, $(filter %.y, $(SOURCE))) \
		      $(patsubst %.lex, %.tab.c, $(filter %.lex, $(SOURCE))) \
		        )

OBJECT = $(TMPOBJECT) $(patsubst %.eps, %.pdf, $(filter %.eps, $(TMPOBJECT)))

DEPEND = $(patsubst %.o, %.d, $(filter %.o, $(OBJECT)))

TMPOBJECT2 = $(addprefix obj/$(OBJDIR)/, $(patsubst %.c,%.o,\
               $(patsubst %.cc,%.o,$(patsubst %.fig,%.eps,\
	         $(patsubst %.tex,%.tex,\
                   $(patsubst %.eps,%.eps,$(patsubst %.y,%.tab.c,\
                     $(patsubst %.lex,%.tab.c,$(SOURCE2)))))))) \
		      $(patsubst %.y, %.tab.c, $(filter %.y, $(SOURCE2))) \
		      $(patsubst %.lex, %.tab.c, $(filter %.lex, $(SOURCE2))) \
		        )

OBJECT2 = $(TMPOBJECT2) $(patsubst %.eps, %.pdf, \
                          $(filter %.eps, $(TMPOBJECT2)))

DEPEND2 = $(patsubst %.o, %.d, $(filter %.o, $(OBJECT2)))

TMPOBJECT3 = $(addprefix obj/$(OBJDIR)/, $(patsubst %.c,%.o,\
               $(patsubst %.cc,%.o,$(patsubst %.fig,%.eps,\
	         $(patsubst %.tex,%.tex,\
                   $(patsubst %.eps,%.eps,$(patsubst %.y,%.tab.c,\
                     $(patsubst %.lex,%.tab.c,$(SOURCE3)))))))) \
		      $(patsubst %.y, %.tab.c, $(filter %.y, $(SOURCE3))) \
		      $(patsubst %.lex, %.tab.c, $(filter %.lex, $(SOURCE3))) \
		        )

OBJECT3 = $(TMPOBJECT3) $(patsubst %.eps, %.pdf, \
                          $(filter %.eps, $(TMPOBJECT3)))

DEPEND3 = $(patsubst %.o, %.d, $(filter %.o, $(OBJECT3)))


TMPOBJECT4 = $(addprefix obj/$(OBJDIR)/, $(patsubst %.c,%.o,\
               $(patsubst %.cc,%.o,$(patsubst %.fig,%.eps,\
	         $(patsubst %.tex,%.tex,\
                   $(patsubst %.eps,%.eps,$(patsubst %.y,%.tab.c,\
                     $(patsubst %.lex,%.tab.c,$(SOURCE4)))))))) \
		      $(patsubst %.y, %.tab.c, $(filter %.y, $(SOURCE4))) \
		      $(patsubst %.lex, %.tab.c, $(filter %.lex, $(SOURCE4))) \
		        )

OBJECT4 = $(TMPOBJECT4) $(patsubst %.eps, %.pdf, \
                          $(filter %.eps, $(TMPOBJECT4)))

DEPEND4 = $(patsubst %.o, %.d, $(filter %.o, $(OBJECT4)))

OBJECTS = $(OBJECT) $(OBJECT2) $(OBJECT3) $(OBJECT4)

DEPENDS = $(DEPEND) $(DEPEND2) $(DEPEND3) $(DEPEND4)

#The 'default' target should be defined in each Makefile

standard: default

.PHONY: standard default new realclean news all install reinstall
.PHONY: install_modules remove_modules domod unmod dotether untether serial
.PHONY: docamera uncamera download upload download_and_update bzip2_and_upload
.PHONY: debug zip unzip gunzip gzip bunzip2 bzip2
.PHONY: clean tidy dist distclean

new:
	$(SILENT) $(MAKE) realclean
	$(SILENT) $(MAKE) default
news:
	$(SILENT) $(MAKE) realclean
	$(SILENT) $(MAKE) all

install: dirs all
reinstall: all
install_modules: domod dotether docamera
remove_modules: unmod untether uncamera
normal_serial: serial

download_and_update: download bunzip2
		     $(SILENT) $(MAKE) reinstall

bzip2_and_upload: bzip2 upload

download:
	@echo "    ---- Getting installation from $(BASEUSER)@$(BASEHOST)"
	$(SILENT) (if [ "$(HOSTNAME)" = "$(BASEHOST)" ]; then \
		   if [ "$(USER)" = "$(BASEUSER)" ]; then \
		    echo "    ++++ You are $(BASEUSER) on $(BASEHOST)"; \
		   else \
		    cp $(BASEHOME)/$(PROJNAME).tbz2 $(SOFTHOME); \
		   fi; \
		  else \
		   scp $(BASEUSER)@$(BASEHOST):$(BASEHOME)/$(PROJNAME).tbz2 \
		       $(SOFTHOME); \
		  fi)

upload:
	@echo "    ---- Giving update to $(BASEUSER)@$(BASEHOST)"
	$(SILENT) if [ "$(HOSTNAME)" = "$(BASEHOST)" ]; then \
		   if [ "$(USER)" = "$(BASEUSER)" ]; then \
		    echo "    ++++ You are $(BASEUSER) on $(BASEHOST)"; \
		   else \
		    cp $(SOFTHOME)/$(PROJNAME).tbz2 $(BASEHOME); \
		   fi; \
		  else \
		   scp $(SOFTHOME)/$(PROJNAME).tbz2 \
		   $(BASEUSER)@$(BASEHOST):$(BASEHOME)/$(PROJNAME).tbz2; \
		  fi

domod:
	@echo "    ---- Reinstalling realtime modules"
	$(SILENT) su -c "$(PROJDIR)/scripts/domod $(USER)"

unmod:
	@echo "    ---- Removing realtime modules"
	$(SILENT) su -c $(PROJDIR)/scripts/unmod

dotether:
	@echo "    ---- Installing I2C modules"
	$(SILENT) su -c $(PROJDIR)/scripts/dotether

untether:
	@echo "    ---- Removing I2C modules"
	$(SILENT) su -c $(PROJDIR)/scripts/untether

docamera:
	@echo "    ---- Installing camera modules"
	$(SILENT) su -c $(PROJDIR)/scripts/docamera

uncamera:
	@echo "    ---- Removing camera modules"
	$(SILENT) su -c $(PROJDIR)/scripts/uncamera

serial:
	@echo "    ---- Setting serial to normal koala usage"
	$(SILENT) su -c "$(PROJDIR)/scripts/normal_serial"

debug:
	$(SILENT) $(MAKE) new DEBUG=true

profile:
	$(SILENT) $(MAKE) new PROFILE=true

zip: realclean tidy
	@echo "    ---- Zipping up $(PROJNAME) directory"
	$(SILENT) cd $(SOFTHOME); \
		  if [ -e $(PROJNAME).zip ]; then \
		     echo "    ---- removing old zip file"; \
		     $(RM) -f $(PROJNAME).zip; \
		  fi; \
		  zip -r $(PROJNAME) $(PROJNAME)

unzip: 
	@echo "    ---- Unzipping $(PROJNAME) directory"
	$(SILENT) cd $(SOFTHOME); \
		  if [ -e $(PROJNAME).zip ]; then \
		     if [ -e $(PROJNAME).old ]; then \
			echo "    ---- removing $(PROJNAME).old directory"; \
			$(RM) -rf $(PROJNAME).old; \
		     fi; \
		     mv $(PROJNAME) $(PROJNAME).old; \
		     unzip $(PROJNAME).zip; \
		  else \
		     echo "    ****  no zip file"; \
		  fi

bzip2:
	@echo "    ---- Bzipping $(PROJNAME) directory"
	$(SILENT) cd $(SOFTHOME); \
		  mkdir -p $(PROJNAME)-$(VERSION); \
		  $(MIRRORDIR) $(PROJNAME) $(PROJNAME)-$(VERSION); \
		  cd $(PROJNAME)-$(VERSION); make clean tidy; cd ..; \
		  if [ -e $(PROJNAME).tbz2 ]; then \
		     echo "    ---- removing old tbz2 file"; \
		     $(RM) -f $(PROJNAME).tbz2; \
		  fi; \
		  tar jcf $(PROJNAME).tbz2 $(PROJNAME)-$(VERSION); \
		  $(RM) -rf $(PROJNAME)-$(VERSION)
	@echo "    ---- Finished bzipping $(PROJNAME) directory"

dist: bzip2

bunzip2: 
	@echo "    ---- Bunzipping $(PROJNAME) directory"
	$(SILENT) cd $(SOFTHOME); \
		  if [ -e $(PROJNAME).tbz2 ]; then \
		     if [ ! -d $(PROJNAME).old ]; then \
			echo "    ---- making $(PROJNAME).old directory"; \
			mkdir -p $(PROJNAME).old; \
		     fi; \
		     $(MIRRORDIR) $(PROJNAME) $(PROJNAME).old; \
		     cd $(PROJNAME).old; make clean; cd ..; \
		     VER:=$(shell tar jtf $(PROJNAME).tbz2 | sed -ne 's/$(PROJNAME)-\([^\/]*\)\/defaults.mk/\1/p'); \
		     tar jxf $(PROJNAME).tbz2; \
		     mirrordir $(PROJNAME)-$$VER $(PROJNAME); \
		     $(RM) -rf $(PROJNAME)-$$VER; \
		     echo "    ---- Finished bunzipping $(PROJNAME) directory";\
		  else \
		     echo "    ****  no bzip2 file"; \
		  fi

# documentation is made from tex files

#$(DOCDIR)/%.ps: $(patsubst %.tex,,$(OBJECTS)) obj/linux/%.tex
$(DOCDIR)/%.ps:
	@echo "    ---- Compiling $< (tex) to postscript"
	$(SILENT) cd obj/linux; \
        rm -f *.blg *.bbl *.brf *.tpt *.toc *.out *.lot *.lof *.aux *.log; \
	if !($(LATEX) $* > /dev/null < /dev/null); then $(LATEX) $*; fi; \
	if $(GREP) -q '\\citation' $*.aux; then \
	   if !($(BIBTEX) $* > /dev/null < /dev/null); then \
	      $(BIBTEX) $*; fi; fi; \
	if !($(LATEX) $* > /dev/null < /dev/null); then $(LATEX) $*; fi; \
	if !($(LATEX) $* > /dev/null < /dev/null); then $(LATEX) $*; fi; \
	if !($(DVIPS) -o $@ $*.dvi 2> /dev/null < /dev/null); then \
	  $(DVIPS) -o $@ $*.dvi; \
        rm -f *.blg *.bbl *.brf *.tpt *.toc *.out *.lot *.lof *.aux *.log; \
	fi

#$(DOCDIR)/%.pdf: $(patsubst %.tex,,$(OBJECTS)) obj/linux/%.tex
$(DOCDIR)/%.pdf:
	@echo "    ---- Compiling $< (tex) to pdf"
	$(SILENT) cd obj/linux; \
        rm -f *.blg *.bbl *.brf *.tpt *.toc *.out *.lot *.lof *.aux *.log; \
	if !($(PDFLATEX) $* > /dev/null < /dev/null); then \
	   $(PDFLATEX) $*; fi; \
	if $(GREP) -q '\\citation' $*.aux; then \
	   if !($(BIBTEX) $* > /dev/null < /dev/null); then \
	      $(BIBTEX) $*; fi; fi; \
	if !($(PDFLATEX) $* > /dev/null < /dev/null); then \
	   $(PDFLATEX) $*; fi; \
	if !($(PDFLATEX) $* > /dev/null < /dev/null); then \
	   $(PDFLATEX) $*; fi; \
        if !(thumbpdf $(@F) > /dev/null < /dev/null); then \
           thumbpdf $(@F); fi; \
        if !($(PDFLATEX) $* > /dev/null < /dev/null); then \
           $(PDFLATEX) $*; fi; \
        rm -f *.blg *.bbl *.brf *.tpt *.toc *.out *.lot *.lof *.aux *.log; \
	$(MV) $*.pdf $(DOCDIR)

obj/linux/%.eps: %.fig
	@echo "    ---- Compiling $< (xfig) to eps"
	$(SILENT) $(FIG2DEV) -L eps $< $@

obj/linux/%.eps: %.eps
	@echo "    ---- Moving $< (eps)"
	$(SILENT) cp $< $@

obj/linux/%.pdf: %.fig
	@echo "    ---- Compiling $< (xfig) to pdf"
	$(SILENT) $(FIG2DEV) -L pdf $< $@

obj/linux/%.pdf: %.eps
	@echo "    ---- Compiling $< (eps) to pdf"
	$(SILENT) $(EPSTOPDF) $< --outfile=$@

obj/linux/%.tex: %.tex
	@echo "    ---- Moving $< (tex)"
	$(SILENT) cp $< $@

# All linux object files stored in obj/linux/ subdirectories

obj/linux/%.o: %.cc
	@echo "    ---- Generating dependencies for $< (C++)"
	$(SILENT) $(CXX) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CXXFLAGS) $(CFLAGS) \
			 -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $< (C++)"
	$(SILENT) $(CXX) $(INCDIRS) $(LDEFINES) $(CXXFLAGS) $(CFLAGS) \
			 -c -o $@ $<
#obj/linux/%.d: %.cc obj/linux/%.o
#	@echo "    ---- Generating dependencies for $< (C++)"
#	$(SILENT) $(CXX) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CXXFLAGS) $(CFLAGS) \
#			 -c -o $(@:.d=.o) $< > /dev/null
obj/linux/%.o: %.c
	@echo "    ---- Generating dependencies for $< (C)"
	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $< (C)"
	$(SILENT) $(CC) $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $@ $<

#obj/linux/%.d: %.c obj/linux/%.o
#	@echo "    ---- Generating dependencies for $< (C)"
#	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $(@:.d=.o) $< > /dev/null

obj/linux/%.tab.c: %.y
	@echo "    ---- Generating $@ (C from bison)"
	$(SILENT) $(YACC) -d -o$@ $<

obj/linux/%.tab.o: obj/linux/%.tab.c
	@echo "    ---- Generating dependencies for $< (bison/flex)"
	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $@ (C from bison/flex)"
	$(SILENT) $(CC) $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $@ $(@:.o=.c)

#obj/linux/%.tab.d: %.y obj/linux/%.tab.o
#	@echo "    ---- Generating dependencies for $< (bison)"
#	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $(@:.d=.o) $(patsubst %.y, obj/linux/%.tab.c, $<) > /dev/null

obj/linux/%.tab.c: %.lex
	@echo "    ---- Generating $@ (C from flex)"
	$(SILENT) $(LEX) -o$@ $<

#obj/linux/%.tab.d: %.lex obj/linux/%.tab.o
#	@echo "    ---- Generating dependencies for $< (flex)"
#	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) -c -o $(@:.d=.o) $(patsubst %.lex, obj/linux/%.tab.c, $<) > /dev/null

# except real-time kernel modules

$(MODDIR)/%.o: %.c
	@echo "    ---- Generating dependencies for $< (RTL C kernel module)"
	$(SILENT) $(CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) $(RTFLAGS) -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $< (RTL C kernel module)"
	$(SILENT) $(MODULE_CC) $(INCDIRS) $(LDEFINES) $(CFLAGS) $(RTFLAGS) -c -o $@ $<

#$(MODDIR)/%.d: %.c $(MODDIR)/%.o
#	@echo "    ---- Generating dependencies for $< (RTL C kernel module)"
#	$(SILENT) $(LINUX_CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(CFLAGS) $(RTFLAGS) -c -o $(@:.d=.o) $< > /dev/null
#> $@


# Koala in obj/koala/

ifdef X_COMPILER_PRESENT
obj/koala/%.o: %.c
	@echo "    ---- Generating dependencies for $< (C) for koala"
	$(SILENT) $(LINUX_CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(ROBOT_INC) \
	 -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $< (C) for koala"
	$(SILENT) $(CC) $(LDEFINES) $(INCDIRS) $(CFLAGS) -c -o $@ $<
else
obj/koala/%.o: %.c
	$(error You are compiling for the koala without the cross-compiler \
present!)
endif

#obj/koala/%.d: %.c obj/koala/%.o
#	@echo "    ---- Generating dependencies for $< (C) for koala"
#	$(SILENT) $(LINUX_CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) -nostdinc \
#-I$(KOA_PACK)/include -c -o $(@:.d=.o) $< > /dev/null

# and Khepera in obj/khepera/

ifdef X_COMPILER_PRESENT
obj/khepera/%.o: %.c
	@echo "    ---- Generating dependencies for $< (C) for khepera"
	$(SILENT) $(LINUX_CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) $(ROBOT_INC) \
	 -c -o $@ $< > /dev/null
	@echo "    ---- Compiling $< (C) for khepera"
	$(SILENT) $(CC) $(LDEFINES) $(INCDIRS) $(CFLAGS) -c -o $@ $<
else
obj/khepera/%.o: %.c
	$(error You are compiling for the khepera without the cross-compiler \
present!)
endif

#obj/khepera/%.d: %.c obj/khepera/%.o
#	@echo "    ---- Generating dependencies for $< (C) for khepera"
#	$(SILENT) $(LINUX_CC) -MD -E -MT $@ $(INCDIRS) $(LDEFINES) -nostdinc \
#-I$(KHE_PACK)/include -c -o $(@:.d=.o) $< > /dev/null

#library missing

$(LIBDIR)/lib%.a:
	@echo "    **** Trying to make $* library from wrong directory"
	@echo "         (currently in $(shell $(PWD)))"
	$(SILENT) exit 1

# make object file directories

dirs:
	@echo "    ---- making object directories in $(shell $(PWD) | $(XARGS) $(BASENAME))/"
	$(SILENT) if [ "$(DIRS)" != "" ]; then      \
			for Dir in $(DIRS)""; do    \
				HERE=$(shell $(PWD));      \
				if [ -d $$Dir ]; then \
				cd $$Dir;	    \
				$(MAKE) dirs;	    \
				cd $$HERE;          \
				else            \
				 echo Directory $$Dir/ not found!; \
				fi; \
			done;			    \
			echo "    ---- making library and executable\
				directories";       \
			for Dir in $(TARGETS); do   \
				mkdir -p $(SOFTHOME)/bin/$$Dir; \
				mkdir -p $(SOFTHOME)/lib/$$Dir; \
				mkdir -p $(SOFTHOME)/lib/modules/$$Dir; \
			done;                       \
			mkdir -p $(DOCDIR);         \
		  else				    \
			for Dir in $(TARGETS); do   \
				mkdir -p obj/$$Dir; \
			done;			    \
		  fi

# Get rid of the .o files.
clean:
	@echo "    ---- Cleaning $(shell $(PWD) | $(XARGS) $(BASENAME))/ of object files"
	$(SILENT) $(RM) -f obj/linux/*.* obj/koala/*.* obj/khepera/*.*
	$(SILENT) if [ "$(DIRS)" != "" ]; then   \
			for Dir in $(DIRS)""; do \
				HERE=$(shell $(PWD));   \
				if [ -d $$Dir ]; then \
				 cd $$Dir;       \
				 $(MAKE) clean;  \
				 cd $$HERE;      \
				else            \
				 echo Directory $$Dir/ not found!; \
				fi; \
			done;                    \
		  fi

# Get rid of the .o files and the libraries and executables.
realclean:
	@echo "    ---- Cleaning $(shell $(PWD) | $(XARGS) $(BASENAME))/ of object files\
			and libs/executables"
	$(SILENT) $(RM) -f obj/linux/*.* obj/koala/*.* obj/khepera/*.* \
			 *.bak $(CLOBBERFILES)
	$(SILENT) if [ "$(DIRS)" != "" ]; then     \
			for Dir in $(DIRS)""; do   \
				HERE=$(shell $(PWD));     \
				if [ -d $$Dir ]; then \
				cd $$Dir;          \
				$(MAKE) realclean; \
				cd $$HERE;         \
				else            \
				 echo Directory $$Dir/ not found!; \
				fi; \
			done;                      \
		  fi

distclean: realclean tidy

RUBBISH = $(wildcard  *~ .*~ \#*\# *.bak core)

NEAT = "    ---- Delete following files?\n$(RUBBISH:%=         %\n)\
		 ('q' to abort)"

tidy:
	@echo "    .... Cleaning $(shell $(PWD) | $(XARGS) $(BASENAME))/ of backup files"
	$(SILENT) if [ -f $(WAITKEY) ]; then \
	  if [ $(words $(RUBBISH)) != 0 ]; then \
            echo -e $(NEAT); \
	    $(WAITKEY) $(words $(RUBBISH)); \
	  fi; \
	else \
	  if [ $(words $(RUBBISH)) != 0 ]; then \
	    echo -e "    ---- Deleting $(RUBBISH)"; \
	  fi; \
	fi
	$(SILENT) $(RM) -f $(RUBBISH)
	$(SILENT) if [ "$(DIRS)" != "" ]; then     \
			for Dir in $(DIRS) include initialise scripts scilab""; do \
				HERE=$(shell $(PWD));     \
				if [ -d $$Dir ]; then \
				cd $$Dir;          \
				$(MAKE) tidy;      \
				cd $$HERE;         \
				else            \
				 echo Directory $$Dir/ not found!; \
				fi; \
			done;                      \
		  fi


#-------------------------------------------------------------------------

# Hack to get around makefile include errors...
ifneq ($(wildcard $(DEPENDS)),$(DEPENDS))
%.d:
	$(SILENT) touch $@
endif
