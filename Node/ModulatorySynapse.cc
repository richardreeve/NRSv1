// $Id: ModulatorySynapse.cc,v 1.3 2003/09/01 16:21:42 darrens Exp $
#include <iostream>
#include <map>
#include <string>

#include <cmath>
#pragma implementation
#include "ModulatorySynapse.hh"
#include "Params.hh"
#include "base.h"

#define PARAM_MOD_STRENTGH "mModStr"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void modulatorysynapse_init( Node* aRoot ) throw ( bad_param, bad_file )
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
            // though ModulatorySynapse doesn't make use of most of these
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
            std::cerr << bp.what() << " already defined!" << std::endl;
            throw;
          }
      }

    
    try
      {
        already=false;
	aParams.addParam("ModulatorySynapse", "enum");
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam(PARAM_MOD_STRENTGH , "nS" );
          }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
            _TRACE_("Exception: " << bp.what() << " Hint: " << bp.hint() 
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
void ModulatorySynapse::initParams() throw ( bad_param, bad_file )
{
  modulatorysynapse_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void ModulatorySynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a ModulatorySynapse called " << iName 
       << " which belongs to node " << iParent->getName() << std::endl;
#endif

  iModStrength = iParams.getParam(PARAM_MOD_STRENTGH);

  // No spike has occurred at the start
  iSpikeArrived = false;
  iJustFired = false;

  // No timesteps since last output
  iCount = 0;
  iSumModOutput = 0;
}

/// Do one timestep for self only
bool ModulatorySynapse::nodeTimestep()
{
  // Update output neuron
  if (iSpikeArrived)
    {
      iParent->doReceive(iMessage.newAnalogue(iModStrength));

      iSpikeArrived = false;
      iJustFired = true;
    }
  else
    {
      iJustFired = false;
    }

  // Update variables concerned with keeping track of averages
  iCount++;

  if (iJustFired) iSumModOutput += iModStrength;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& ModulatorySynapse::externalise( std::ostream& anOStream, 
					       std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a ModulatorySynapse called " << iName
	    << " which belongs to node " << iParent->getName() 
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " ModulatorySynapse "; 
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

  if (iParams.getParam(PARAM_MOD_STRENTGH ) != pParams.getParam(PARAM_MOD_STRENTGH ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, PARAM_MOD_STRENTGH );
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
std::ostream& ModulatorySynapse::write( std::ostream& anOStream,
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
	anOStream << myName << ".iModOutput\t";
      break;
    case outTypeMatlab:
      if (gOutputLevel.test( outputSynapse ))
	{
	  anOStream << aName << ".modulatorysynapse = 1;" << std::endl;
          //	  anOStream << aName << ".synV = " << synV << ";" << std::endl;
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
double* ModulatorySynapse::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test(outputSynapse))
	    {
              *(thePtrPos++) = iSumModOutput/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test(outputSynapse))
	    {
              *(thePtrPos++) = iSumModOutput;
	    }
	}
      
      iCount = 0;
      iSumModOutput = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSynapse ))
	{
	  *(thePtrPos++) = (iJustFired? iModStrength : 0.0);
	}
    }      
  return thePtrPos;
}

