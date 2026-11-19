// $Id: PlainSynapse.cc,v 1.2 2003/09/25 17:40:15 darrens Exp $
#include <iostream>
#include <map>
#include <string>

#include <cmath>
#pragma implementation
#include "Exception.hh"
#include "PlainSynapse.hh"
#include "Params.hh"
#include "base.h"

#define PARAM_REV_POT  "pSynE"
#define PARAM_CONDUCTANCE "pSynG"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void plainsynapse_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already;
    Params& aParams=aRoot->getParams();

    try
      {
        already=false;
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
            // Shared with all synapses, so must be included here even
            // though PlainSynapse doesn't make use of most of these
            // parameters.
            aParams.addParam( "excitatory", "enum" );
            aParams.addParam( "inhibitory", "enum" );
            aParams.addParam( "expSynT", "ms" );
            aParams.addParam( "synDelayT", "ms" );
            aParams.addParam( "synFacT", "ms" );
            aParams.addParam( "synDepT", "ms" );
          }
        catch ( bad_param& bp )
          {
            _WARN_("Exception: " << bp.what() << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
            throw;
          }
      }
   
    try
      {
        already=false;
	aParams.addParam( "PlainSynapse", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam(PARAM_REV_POT, "mV" );
	    aParams.addParam(PARAM_CONDUCTANCE, "nS" );
          }
	catch ( bad_param& bp )
	  {
            _WARN_("Exception: " << bp.what() << " Hint: " << bp.hint() 
                    << " Thrower: " << bp.origin());
	    throw;
	  }
      }
  }
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void PlainSynapse::initParams() throw ( bad_param, bad_file )
{
  plainsynapse_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void PlainSynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a PlainSynapse called " << iName 
       << " which belongs to node " << iParent->getName() << std::endl;
#endif

  Params& pParams = iParent->getParams();

  double stepT = iParams.getParam( "stepT" );

  // Access the parent's membranceConductance
  double tc = - stepT / pParams.getParam( "membC" );

  // Relative reversal potential of the synapse.  Throughout NRS,
  // calculations involving membrane potential are scaled so that the
  // membrane potential is mathematically treated as if it resides at
  // zero.  So to conform with that coordinate syste, we here transform
  // the user-specified reveral potential to the zero-based coordinates.
  // Not that the scaling is based upon the membrane potential of the
  // synapse's target, hence the use of pParams, which is the paramater
  // object for the parent of this synapse.
  iVBattery = iParams.getParam(PARAM_REV_POT)-pParams.getParam("membV");

  // Scale the conductance.  It is combined with the 'tc' parameter,
  // defined above.  This done because this scaled value can be used
  // directly within an exponential within the received neuron to
  // determine the evolution of the membrane potential.  If we didn't do
  // this scaling here, and instead just passed the conductance along,
  // the scaling would need to be performed within the neuron, for each
  // timestep. Hence this is more efficient.
  iGBase_tc = tc * iParams.getParam(PARAM_CONDUCTANCE);

  // Decay rate of conductance in synapse
  iDecay = exp( log( 0.5 ) * stepT / iParams.getParam( "expSynT" ) );

  // scaled current conductance defaults to 0
  iGActual_tc = 0.0;

  // No timesteps since last output
  iCount = 0;
  iSumGActual_tc = 0;
}

/// Do one timestep for self only
bool PlainSynapse::nodeTimestep()
{
  // decay conductance
  iGActual_tc *= iDecay;
  
  // Update output neuron
  double theGV = iGActual_tc * iVBattery;
  iParent->doReceive( iMessage.newConductance( iGActual_tc, theGV ) );

  // Update variables concerned with keeping track of averages
  iCount++;
  iSumGActual_tc += iGActual_tc;

  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& PlainSynapse::externalise( std::ostream& anOStream, 
					       std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a PlainSynapse called " << iName
	    << " which belongs to node " << iParent->getName() 
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " PlainSynapse "; 
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

  if (iParams.getParam(PARAM_REV_POT) != pParams.getParam(PARAM_REV_POT))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, PARAM_REV_POT);
    }

  if (iParams.getParam(PARAM_CONDUCTANCE) != pParams.getParam(PARAM_CONDUCTANCE))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, PARAM_CONDUCTANCE);
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

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& PlainSynapse::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputSynapse ))
	anOStream << myName << ".iGActual_tc\t";
      break;
    case outTypeMatlab:
      // TODO - don't know how to update this section
      if (gOutputLevel.test( outputSynapse ))
	{
	  anOStream << aName << ".plainsynapse = 1;" << std::endl;
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

/// Output current state of node to array
/**
 * This is used by all the other put methods to gather data
 * @param anArray array of doubles to store data in
 * @param average true to get average since last reading, else current
 * @return pointer to end of used part of array
 */
double* PlainSynapse::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iSumGActual_tc/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iGActual_tc;
	    }
	}
      
      iCount = 0;
      iSumGActual_tc = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSynapse ))
	{
	  *(thePtrPos++) = iGActual_tc;
	}
    }      
  return thePtrPos;
}

