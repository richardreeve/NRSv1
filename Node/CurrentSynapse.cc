// $Id: CurrentSynapse.cc,v 1.3 2003/09/25 17:40:15 darrens Exp $

#include <iostream>
#include <map>
#include <string>

#include <cmath>
#pragma implementation
#include "CurrentSynapse.hh"
#include "Params.hh"
#include "base.h"
#include "Exception.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void cus_init( Node* aRoot ) throw ( bad_param, bad_file )
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
    
    already=false;
    
    try
      {
	aParams.addParam( "CurrentInjection", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
	std::cerr << bp.what() << " already defined!" << std::endl;
      }
    
    if (!already)
      {
	try
	  {
	    // spike input parameters
	    aParams.addParam( "ecSynI", "pA" );
	    aParams.addParam( "icSynI", "pA" );
	    aParams.addParam( "cSynFacI", "pA" );
	    aParams.addParam( "cSynDepFrac","/ms" );
	    
	    // analogue input parameters
	    aParams.addParam( "aecSynIT", "pA/ms/unit" );
	    aParams.addParam( "aicSynIT", "pA/ms/unit" );
	    aParams.addParam( "acSynFacIT", "pA/ms/unit" );
	    aParams.addParam( "acSynDepFracT", "/ms/unit" );
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
void CurrentSynapse::initParams() throw ( bad_param, bad_file )
{
  cus_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void CurrentSynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a current synapse called " << iName 
       << " which belongs to node " << iParent->getName() << std::endl;
#endif

  Params& pParams = getRoot()->getParams();
  double stepT = iParams.getParam( "stepT" );
  double tc = - stepT / pParams.getParam( "membC" );

  double i=0;
  double ic=0;

  switch (iSign)
    {
    case SYNAPSES_SIGNLESS:
      _WARN_("Synapse" <<
             " (" << "CurrentInjection" << "/" << iName << ") " <<
             "defined as signless (i.e., neither excitatory or inhibitory)." <<
             " It defaults to excitatory.");
      /* fall through to case SYNAPSES_EXCITATORY */
    case SYNAPSES_EXCITATORY:
      i = iParams.getParam( "ecSynI" );
      ic = iParams.getParam( "aecSynIT" ) * stepT;
      break;
    case SYNAPSES_INHIBITORY:
      i = iParams.getParam( "icSynI" );
      ic = iParams.getParam( "aicSynIT" ) * stepT;
      break;
    case SYNAPSES_SHUNTING:
      // Illegal type for current injection synapses
      throw bad_node( iName );
      break;
    }

  // scaled base current
  iIBase_tc = tc * i;
  iICBase_tc = tc * ic;
  // Decay rate of current in synapse
  iDecay = exp( log( 0.5 ) * stepT / iParams.getParam( "expSynT" ) );
  // scaled current current defaults to 0
  iIActual_tc = 0.0;
  // scaled facilitatory current increase
  iIFac_tc = tc * iParams.getParam("cSynFacI");
  iICFac_tc = tc * iParams.getParam("acSynFacIT") * stepT;
  // and associated decay
  iTFac = exp( log( 0.5 ) * stepT / iParams.getParam( "synFacT" ) );
  // start at 0
  iIFacReal_tc = 0;
  // scaled depressive current decrease
  iDepFrac = iParams.getParam( "cSynDepFrac" );
  iCDepFrac_ln = log( iParams.getParam( "acSynDepFracT" ) ) * stepT;
  // and associated decay
  iTDep = exp( log( 0.5 ) * stepT / iParams.getParam( "synDepT" ) );
  // start at 1 (no depression)
  iDepRealFrac = 1.0;

  // No timesteps since last output
  iCount = 0;
  iSumIActual_tc = 0;
  iSumDepRealFrac = 0;
  iSumIFacReal_tc = 0;
}

/// Do one timestep for self only
bool CurrentSynapse::nodeTimestep()
{
  // decay current
  iIActual_tc *= iDecay;
  iSumIActual_tc += iIActual_tc;
  
  // decay facilitation/depression
  iIFacReal_tc *= iTFac;
  iSumIFacReal_tc += iIFacReal_tc;
  
  // iDepRealFrac = 1.0 - (1.0 - iDepRealFrac) * iTDep;
  iDepRealFrac -= 1.0;
  iDepRealFrac *= iTDep;
  iDepRealFrac += 1.0;
  iSumDepRealFrac += iDepRealFrac;

  // Update output neuron
  iParent->doReceive( iMessage.newCurrent( iIActual_tc ) );

  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& CurrentSynapse::externalise( std::ostream& anOStream, 
					   std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a current synapse called " << iName
	    << " which belongs to node " << iParent->getName()
	    << std::endl << indent
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " CurrentInjection "; 
  switch (iSign)
    {
    case SYNAPSES_SIGNLESS:
      _WARN_("Synapse" <<
             " (" << "CurrentInjection" << "/" << iName << ") " <<
             "defined as signless (i.e., neither excitatory or inhibitory)." <<
             " It defaults to excitatory.");
      /* fall through to case SYNAPSES_EXCITATORY */
    case SYNAPSES_EXCITATORY:
      anOStream << "excitatory ";
      break;
    case SYNAPSES_INHIBITORY:
      anOStream << "inhibitory ";
      break;
    case SYNAPSES_SHUNTING:
      anOStream << "shunting ";
      break;
    }
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

  if (iParams.getParam( "ecSynI" ) != pParams.getParam( "ecSynI" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "ecSynI" );
    }

  if (iParams.getParam( "icSynI" ) != pParams.getParam( "icSynI" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "icSynI" );
    }

  if (iParams.getParam( "cSynFacI" ) != pParams.getParam( "cSynFacI" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "cSynFacI" );
    }

  if (iParams.getParam( "cSynDepFrac" ) !=
      pParams.getParam( "cSynDepFrac" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "cSynDepFrac" );
    }

  if (iParams.getParam( "aecSynIT" ) != pParams.getParam( "aecSynIT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "aecSynIT" );
    }

  if (iParams.getParam( "aicSynIT" ) != pParams.getParam( "aicSynIT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "aicSynIT" );
    }
  
  if (iParams.getParam( "acSynFacIT" ) != pParams.getParam( "acSynFacIT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "acSynFacIT" );
    }
  
  if (iParams.getParam( "acSynDepFracT" ) != 
      pParams.getParam( "acSynDepFracT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "acSynDepFracT" );
    }

  if (extras)
    {
      indent.erase( indent.size() - 1 );
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
std::ostream& CurrentSynapse::write( std::ostream& anOStream,
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
	anOStream << myName << ".iIActual_tc\t";
      if (gOutputLevel.test( outputFacDep ))
	anOStream << myName << ".iIFacReal_tc\t" 
		  << myName << ".iDepRealFrac\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputSynapse )) ||
	  (gOutputLevel.test( outputFacDep )))
	{
	  anOStream << aName << ".currentsynapse = 1;" << std::endl;
	  double tcMemb = - iParams.getParam( "stepT" ) /
	    iParent->getParams().getParam( "membC" );
	  anOStream << aName << ".tcMemb = " << tcMemb << ";" << std::endl;
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
double* CurrentSynapse::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iSumIActual_tc/iCount;
	    }
	  if (gOutputLevel.test( outputFacDep ))
	    {
	      *(thePtrPos++) = iSumIFacReal_tc/iCount;
	      *(thePtrPos++) = iSumDepRealFrac/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSynapse ))
	    {
	      *(thePtrPos++) = iIActual_tc;
	    }
	  if (gOutputLevel.test( outputFacDep ))
	    {
	      *(thePtrPos++) = iIFacReal_tc;
	      *(thePtrPos++) = iDepRealFrac;
	    }
	}
      
      iCount = 0;
      iSumIFacReal_tc = 0;
      iSumIActual_tc = 0;
      iSumDepRealFrac = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSynapse ))
	{
	  *(thePtrPos++) = iIActual_tc;
	}
      if (gOutputLevel.test( outputFacDep ))
	{
	  *(thePtrPos++) = iIFacReal_tc;
	  *(thePtrPos++) = iDepRealFrac;
	}
    }      
  return thePtrPos;
}
