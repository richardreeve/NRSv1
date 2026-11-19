// $Id: Neuron.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>

#pragma implementation
#include "Neuron.hh"
#include "Params.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void neuron_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "Neuron", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam( "membV", "mV" );
	    aParams.addParam( "membC", "pF" );
	    aParams.addParam( "membG", "nS" );
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
void Neuron::initParams() throw ( bad_param, bad_file )
{
  neuron_init( gRoot );
}
