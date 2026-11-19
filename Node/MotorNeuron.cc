// $Id: MotorNeuron.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <string>

#include <cmath>
#include "Params.hh"
#include "NonSpikingNeuron.hh"
#pragma implementation
#include "MotorNeuron.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void motor_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();

    NonSpikingNeuron::initParams();

    try
      {
	aParams.addParam( "MotorNeuron", "enum" );
      }
    catch ( bad_param& bp )
      {
	std::cerr << bp.what() << " already defined!" << std::endl;
	already=true;
      }

    if (!already)
      {
	try
	  {
	    aParams.addParam( "baseSpeed", "m/s" );
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
void MotorNeuron::initParams() throw ( bad_param, bad_file )
{
  motor_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void MotorNeuron::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a MotorNeuron called " << iName << std::endl;
#endif

  if (iTarget.empty())
    {
#ifdef DEBUG
      std::cerr << "MotorNeuron without target... strange" << std::endl;
#endif
    }
  else
    {
      Params& tParams = getNode( iTarget )->getParams();
      iParams.setParam( "motorSpeedScale",
			tParams.getParam( "motorSpeedScale" ) );
    }

  iVBase = iParams.getParam( "baseSpeed" ) * 
    iParams.getParam( "motorSpeedScale" );
  iParams.setParam( "membV", iVBase );
  NonSpikingNeuron::initialise();
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream&  MotorNeuron::externalise( std::ostream& anOStream, 
					 std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();


#ifdef DEBUG
  anOStream << "// Configuring a MotorNeuron called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Neuron " << iName << " MotorNeuron";
  
  if (!iTarget.empty())
    anOStream << " targets " << iTarget;
  

  if (iParams.getParam( "baseSpeed" ) != pParams.getParam( "baseSpeed" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "baseSpeed" );
    }

  if (iParams.getParam( "membC" ) != pParams.getParam( "membC" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "membC" );
    }

  if (iParams.getParam( "membG" ) != pParams.getParam( "membG" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "membG" );
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
