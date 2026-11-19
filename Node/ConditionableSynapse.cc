// $Id: ConditionableSynapse.cc,v 1.5 2003/12/19 13:46:04 darrens Exp $
#include <iostream>
#include <map>
#include <string>

#include <cmath>
#pragma implementation
#include "ConditionableSynapse.hh"
#include "Params.hh"
#include "base.h"

#define PARAM_FAC_HALFLIFE   "facDecayT"
#define PARAM_TRACE_HALFLIFE "traceDecayT"
#define PARAM_PRIMER_FACTOR  "traceFactor"

using namespace Runtime;

namespace 
{
  // This function mdoes the initialisation
  void conditionablesynapse_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already = false;
    Params& aParams=aRoot->getParams();
   

    try
      {
	aParams.addParam( "Synapse", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    // Shared with all synapses
	    aParams.addParam( "excitatory", "enum" );
	    aParams.addParam( "inhibitory", "enum" );
	    aParams.addParam( "expSynT", "ms" );
	    aParams.addParam( "synDelayT", "ms" );
	    aParams.addParam( "synFacT", "ms" );
	    aParams.addParam( "synDepT", "ms" );
	  }
	catch ( bad_param& bp )
	  {
            _TRACE_("Parameter exception: " << bp.what() 
                    << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }
    
    try
      {
        already=false;
        aParams.addParam("ConductanceBased", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam( "shunting", "enum" );

	    aParams.addParam( "evSynV", "mV" );
	    aParams.addParam( "ivSynV", "mV" );
	    aParams.addParam( "gvSynV", "mV" );
	    
	    // spike input parameters
	    aParams.addParam( "vSynG", "nS" );
	    aParams.addParam( "vSynFacG", "nS" );
	    aParams.addParam( "vSynDepFrac", "/ms" );
	    
	    // analogue input parameters
	    aParams.addParam( "avSynGT", "nS/ms/unit" );
	    aParams.addParam( "avSynFacGT", "nS/ms/ms/unit/unit" );
	    aParams.addParam( "avSynDepFracT", "/ms/unit" );
	  }
	catch ( bad_param& bp )
	  {
            _TRACE_("Parameter exception: " << bp.what() 
                    << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }

    try
      {
        already=false;
	aParams.addParam( "ConditionableSynapse", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam(PARAM_FAC_HALFLIFE, "ms");
	    aParams.addParam(PARAM_TRACE_HALFLIFE, "ms");
	    aParams.addParam(PARAM_PRIMER_FACTOR, "none");
          }
	catch ( bad_param& bp )
	  {
            _TRACE_("Exception:" << bp.what() << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }
  }
}

//-----------------------------------------------------------------------------
/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void ConditionableSynapse::initParams() throw ( bad_param, bad_file )
{
  // Call base class do initialise its parameters...
  PlainSynapse::initParams();

  // ... and now initialise parameters for this class
  conditionablesynapse_init(gRoot);
}

//-----------------------------------------------------------------------------
/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void ConditionableSynapse::initialise()
{
  // Initialise the base class
  PlainSynapse::initialise();

#ifdef DEBUG
  std::cout << "Initialising a ConditionableSynapse called " << iName 
       << " which belongs to node " << iParent->getName() << std::endl;
#endif

  double stepT = iParams.getParam( "stepT" );

  // Access the parent neuron's conductance so that the
  // time-membConductance constant can be stored.  This will be used to
  // convert the conductance values received in conductance change
  // messages.
  Params& pParams = iParent->getParams();
  iTC = - stepT / pParams.getParam( "membC" );

  // Decay rate of conductance level due to facilitation
  iFacDecay = exp(log(0.5) * stepT / iParams.getParam(PARAM_FAC_HALFLIFE));

  // Decay rate of CS-activity-trace
  iTraceDecay = exp(log(0.5) * stepT / iParams.getParam(PARAM_TRACE_HALFLIFE));

  // Get the trace scaling factor
  iTraceFactor = iParams.getParam(PARAM_PRIMER_FACTOR);
  
  // No timesteps since last output
  iSumFacG_tc = 0;
  iFacG_tc = 0;  
  iCSTrace = 0;

  PlainSynapse::initialise();
}

//-----------------------------------------------------------------------------
/// Do one timestep for self only
bool ConditionableSynapse::nodeTimestep()
{
  // Decay the conductance due to facilitation effects
  iFacG_tc *= iFacDecay;

  // Decay the CS activity trace
  iCSTrace *= iTraceDecay;
  
  // Update variables concerned with averages.  Note, we do no update
  // iCount here - that is done in the base class
  iSumFacG_tc += iFacG_tc;

  // Call the base class to complete nodeTimestep processing
  return PlainSynapse::nodeTimestep();
}

//-----------------------------------------------------------------------------
/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& ConditionableSynapse::externalise( std::ostream& anOStream, 
					       std::string indent )
{

  // Unfortunately, this member does not inherit well.  That is, I can't
  // inherit from the base class member because at some point it would
  // output the name, "PlainSynapse", which would spoil the output for a
  // ConditionableSynapse.  TODO - at some point, modularise better the
  // base class and this class.

  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a ConditionableSynapse called " << iName
	    << " which belongs to node " << iParent->getName() 
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " ConditionableSynapse "; 
  anOStream << "from ";
  // Note illegal if more than one, but we'll print it out anyway...
  for (std::map< std::string, bool >::iterator anIter = iRemote.begin();
       anIter != iRemote.end(); anIter++)
    {
      if (anIter != iRemote.begin())
	anOStream << "/* and illegal ";
      anOStream << anIter->first << " ";
      if (anIter != iRemote.begin())
	anOStream << "*/ ";
    }

  anOStream << "to " << iParent->getName();
  
  if (iParams.getParam( "synDelayT" ) != pParams.getParam( "synDelayT" ))
    {
      anOStream << " delay " << iParams.getParam( "synDelayT" );
    }

  if (iParams.getParam( "expSynT" ) != pParams.getParam( "expSynT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "expSynT" );
    }

  if (iParams.getParam( "synFacT" ) != pParams.getParam( "synFacT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "synFacT" );
    }

  if (iParams.getParam( "synDepT" ) != pParams.getParam( "synDepT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "synDepT" );
    }

  if (iParams.getParam(PARAM_FAC_HALFLIFE) != pParams.getParam(PARAM_FAC_HALFLIFE))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, PARAM_FAC_HALFLIFE);
    }

  if (iParams.getParam(PARAM_TRACE_HALFLIFE) != pParams.getParam(PARAM_TRACE_HALFLIFE))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, PARAM_TRACE_HALFLIFE);
    }

  if (extras)
    {
      indent.erase( indent.size()-1 );
      anOStream << indent << "}";
    }
  else
    anOStream << ";";
  
  return anOStream;
}

//-----------------------------------------------------------------------------
/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& ConditionableSynapse::write( std::ostream& anOStream,
					 OutputType aType )
{
  std::string aName = getName( true );
  std::string myName = aName;
  std::string totalName = getRoot()->getName() + '.' + aName;
  std::string::size_type anST = aName.find( '.' );
  while (anST != aName.npos)
    {
      aName.replace( anST, 1, "_" );
      anST = aName.find( '.' );
    }

  switch ( aType )
    {
    case outTypeText:
      // Unfortunately, here I have to output details of the parameters
      // in the base class.  This is because this member does not
      // inherit well - the type of the name appears hard codeed (see
      // the 'case outTypeMatlab' statement below.
      if (gOutputLevel.test( outputSynapse ))
	anOStream << myName << ".iFacG_tc\t"
                  << myName << ".iGActual_tc\t";
      break;

      // TODO - this MatLab handlers has not yet been updated to output
      // details of the facG_tc variable
    case outTypeMatlab:
      if (gOutputLevel.test( outputSynapse ))
	{
	  anOStream << aName << ".conditionalbesynapse = 1;" << std::endl;
	  double tcMemb = - iParams.getParam( "stepT" ) /
	    iParent->getParams().getParam( "membC" );
	  anOStream << aName << ".tcMemb = " << tcMemb << ";" << std::endl;
	  double synV = iVBattery + iParent->getParams().getParam( "membV" );
	  anOStream << aName << ".synV = " << synV << ";" << std::endl;
	  anOStream << totalName << " = node_convert( "
		    << aName << " );" << std::endl;
	  anOStream << "clear " << aName << std::endl;
	}
      break;
    default:
      std::cerr << "Unknown output type" << std::endl;
      break;
    }
  return anOStream;
}

//-----------------------------------------------------------------------------
/// Output current state of node to array
/**
 * This is used by all the other put methods to gather data
 * @param anArray array of doubles to store data in
 * @param average true to get average since last reading, else current
 * @return pointer to end of used part of array
 */
double* ConditionableSynapse::put( double* anArray, bool average )
{
  // Call the super class to put out the values of its parameters
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test(outputSynapse))
	    {
	      *(thePtrPos++) = iSumFacG_tc / iCount;
              /* the actual-conductance is 'put' by the base class */
	    }
	}
      else
	{
	  if (gOutputLevel.test(outputSynapse))
	    {
	      *(thePtrPos++) = iSumFacG_tc;
              /* the actual-conductance is 'put' by the base class */
	    }
	}
      
      /* Note, usually the iCount variable is reset after calculating
         averages.  However if we do that here, in a derived class, the
         super class - which has yet to be called - won't be able to
         calculate averages.  So don't reset iCount here. Let the super
         class.method do it. */
      iSumFacG_tc = 0;
    }
  else
    {
      if (gOutputLevel.test(outputSynapse))
	{
          *(thePtrPos++) = iFacG_tc;
          /* the actual-conductance is 'put' by the base class */
	}
    }    

  // Call the equivalent base class method.  This has to be done last
  // because the base class will reset the iCount member - which would
  // mean that averages could not be calculated here.
  return PlainSynapse::put(thePtrPos, average);
}

