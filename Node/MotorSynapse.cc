// $Id: MotorSynapse.cc,v 1.3 2003/09/25 17:40:15 darrens Exp $

#include <iostream>
#include <map>
#include <string>

#include <cmath>
#include "base.h"
#include "Params.hh"
#include "ConductanceSynapse.hh"
#pragma implementation
#include "MotorSynapse.hh"
#include "Exception.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void ms_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    ConductanceSynapse::initParams();
    
    try
      {
	aParams.addParam( "MotorSynapse", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam( "emSpeed", "m/s" );
	    aParams.addParam( "imSpeed", "m/s" );
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
void MotorSynapse::initParams() throw ( bad_param, bad_file )
{
  ms_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void MotorSynapse::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a motor synapse called " << iName 
	    << " which belongs to node " << iParent->getName() << std::endl;
#endif
  Params& pParams = iParent->getParams();
  iParams.setParam( "motorSpeedScale",
		    pParams.getParam( "motorSpeedScale" ) );

  iParams.setParam( "evSynV", iParams.getParam( "emSpeed" ) 
		    * iParams.getParam( "motorSpeedScale" ) );
  iParams.setParam( "ivSynV", iParams.getParam( "imSpeed" ) 
		    * iParams.getParam( "motorSpeedScale" ) );

  ConductanceSynapse::initialise();
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& MotorSynapse::externalise( std::ostream& anOStream, 
					 std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();
  
#ifdef DEBUG
  anOStream << "// Configuring a motor synapse called " << iName
	    << " which belongs to node " << iParent->getName()
	    << std::endl << indent
	    << std::endl << indent;
#endif

  // Note may not be connected, so should not mention it...
  if (iRemote.begin() == iRemote.end())
    return anOStream;

  anOStream << "add Synapse " << iName << " MotorSynapse "; 
  switch (iSign)
    {
    case SYNAPSES_SIGNLESS:
      _WARN_("Synapse" <<
             " (" << "MotorSynapse" << "/" << iName << ") " <<
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

  if (iParams.getParam( "emSpeed" ) != pParams.getParam( "emSpeed" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "emSpeed" );
    }

  if (iParams.getParam( "imSpeed" ) != pParams.getParam( "imSpeed" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "imSpeed" );
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

  if (iParams.getParam( "vSynG" ) != pParams.getParam( "vSynG" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynG" );
    }

//    if (iParams.getParam( "evSynG" ) != pParams.getParam( "evSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "evSynG" );
//      }

//    if (iParams.getParam( "ivSynG" ) != pParams.getParam( "ivSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "ivSynG" );
//      }

//    if (iParams.getParam( "svSynG" ) != pParams.getParam( "svSynG" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "svSynG" );
//      }

  if (iParams.getParam( "vSynFacG" ) != pParams.getParam( "vSynFacG" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynFacG" );
    }

  if (iParams.getParam( "vSynDepFrac" ) !=
      pParams.getParam( "vSynDepFrac" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "vSynDepFrac" );
    }

  if (iParams.getParam( "avSynGT" ) != pParams.getParam( "avSynGT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynGT" );
    }

//    if (iParams.getParam( "aevSynGT" ) != pParams.getParam( "aevSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "aevSynGT" );
//      }

//    if (iParams.getParam( "aivSynGT" ) != pParams.getParam( "aivSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "aivSynGT" );
//      }

//    if (iParams.getParam( "asvSynGT" ) != pParams.getParam( "asvSynGT" ))
//      {
//        do_params( extras, anOStream, indent );
//        anOStream << indent;
//        iParams.put( anOStream, "asvSynGT" );
//      }

  if (iParams.getParam( "avSynFacGT" ) != pParams.getParam( "avSynFacGT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynFacGT" );
    }

  if (iParams.getParam( "avSynDepFracT" ) != 
      pParams.getParam( "avSynDepFracT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "avSynDepFracT" );
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
