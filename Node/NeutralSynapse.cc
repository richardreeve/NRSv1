// $Id: NeutralSynapse.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <cmath>
#include <map>
#pragma implementation
#include "NeutralSynapse.hh"
#include "Params.hh"
#include "base.h"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void ns_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
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
	    aParams.addParam( "synDelayT", "ms" );
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
    
    already=false;
    
    try
      {
	aParams.addParam( "PassThrough", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
  }
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void NeutralSynapse::initParams() throw ( bad_param, bad_file )
{
  ns_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void NeutralSynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a neutral synapse called " << iName 
	    << " which belongs to node " << iParent->getName() << std::endl;
#endif

  // No timesteps since last output
  iCount = 0;
  iAnalogue = 0;
  iSumAnalogue = 0;
  bAMessage = false;
  bSMessage = false;
}

/// Do one timestep for self only
bool NeutralSynapse::nodeTimestep()
{
  if (bAMessage)
    {
      bAMessage = false;
      iSumAnalogue += iAnalogue;
      iParent->doReceive( iMessage );
    }
  if (bSMessage)
    {
      bSMessage = false;
      iParent->doReceive( iSpikeMessage );
    }
  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& NeutralSynapse::externalise( std::ostream& anOStream, 
					   std::string indent )
{
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a neutral synapse called " << iName
	    << " which belongs to node " << iParent->getName() 
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " PassThrough "; 
  anOStream << "excitatory ";
  anOStream << "from ";

  // Note illegal if more than one, but we'll print it out anyway...
  for ( std::map< std::string, bool >::iterator anIter = iRemote.begin();
	anIter != iRemote.end(); anIter++ )
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

  anOStream << ";";
  
  return anOStream;
}

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& NeutralSynapse::write( std::ostream& anOStream,
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
	anOStream << myName << ".iAnalogue\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputSynapse )) ||
	  (gOutputLevel.test( outputFacDep )))
	{
	  anOStream << aName << ".neutralsynapse = 1;" << std::endl;
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
double* NeutralSynapse::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iSumAnalogue/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iAnalogue;
	    }
	}
      
      iCount = 0;
      iSumAnalogue = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSynapse ))
	{
	  *(thePtrPos++) = iAnalogue;
	}
    }      
  return thePtrPos;
}

