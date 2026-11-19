# New Neural Makefile
# $Id: makefile,v 1.4 2003/07/29 20:43:49 darrens Exp $

# Available Targets are:
# $(LIBDIR)/libNode.a compiles the library for linux
# depend: realclean: clean: new: dirs: debug: profile:

include ../defaults.mk

#-------------------------------------------------------------------------

# All directories to be compiled in a standard make should be here

LDEFINES +=
# possible defines = -DDEBUG

LIBDIRS += -L$(LIBDIR)

SOFTLIBS = -lNode

SOURCE = Exception.cc Message.cc Params.cc Node.cc Router.cc Neuron.cc \
         SpikingNeuron.cc NonSpikingNeuron.cc MotorNeuron.cc PoissonNeuron.cc \
         ConductanceSynapse.cc CurrentSynapse.cc MotorSynapse.cc \
         NeutralSynapse.cc GeneratedInput.cc SpikeSource.cc \
	 PlainSynapse.cc ConditionableSynapse.cc ModulatorySynapse.cc
SOURCE2 = FileInput.cc Tracker.cc RobotIO.cc
SOURCE3 = helper.cc

#-------------------------------------------------------------------------

ifeq ($(TARGET), linux)
  default: $(LIBDIR)/libNode.a $(LIBDIR)/libDerivedNode.a $(BINDIR)/nodeHelper.$(TARGET)
#  default: $(LIBDIR)/libNode.a $(BINDIR)/nodeTest.$(TARGET) \
#           $(BINDIR)/nodeHelper.$(TARGET)
else
  default:
	@echo "    .... No target in Node/ for $(TARGET)"
endif

$(LIBDIR)/libNode.a: $(DEPEND) $(OBJECT)
		     @echo "    ---- Building library\
			     $(notdir $@) $(patsubst %, with %,\
			     $(patsubst -D$(TARGET),,\
			       $(patsubst -DPROJDIR%,,$(LDEFINES))))"
		       $(SILENT) $(AR) $(AR_FLAGS) $@ $(OBJECT)

$(LIBDIR)/libDerivedNode.a: $(DEPEND2) $(OBJECT2)
		     @echo "    ---- Building library\
			     $(notdir $@) $(patsubst %, with %,\
			     $(patsubst -D$(TARGET),,\
			       $(patsubst -DPROJDIR%,,$(LDEFINES))))"
		       $(SILENT) $(AR) $(AR_FLAGS) $@ $(OBJECT2)


#$(BINDIR)/nodeTest.$(TARGET): $(DEPEND2) $(OBJECT2) $(SOFTDEPS)
#	@echo "    ---- Building executable $(notdir $@)"
#	$(SILENT) $(PURIFY) $(CXX) -o $@ $(LDOPTIONS) \
#		$(CFLAGS) $(OBJECT2) $(LIBDIRS) $(SOFTLIBS) $(LIBS)

$(BINDIR)/nodeHelper.$(TARGET): $(DEPEND3) $(OBJECT3) $(SOFTDEPS)
	@echo "    ---- Building executable $(notdir $@)"
	$(SILENT) $(PURIFY) $(CXX) -static -o $@ $(LDOPTIONS) \
		$(CFLAGS) $(OBJECT3) $(LIBDIRS) $(SOFTLIBS) $(LIBS)

CLOBBERFILES = $(LIBDIR)/../linux/libNode.a \
               $(LIBDIR)/../linux/libDerivedNode.a \
               $(BINDIR)/../linux/nodeHelper.linux
#               $(BINDIR)/../linux/nodeTest.linux \

#-------------------------------------------------------------------------
# Dependencies (only include if present!)

ifneq ($(wildcard $(DEPENDS)),)
     include $(wildcard $(DEPENDS))
endif
