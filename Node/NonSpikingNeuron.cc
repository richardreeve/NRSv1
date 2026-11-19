// $Id: NonSpikingNeuron.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <string>

#include <cmath>
#include "Neuron.hh"
#pragma implementation
#include "NonSpikingNeuron.hh"
#include "Params.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void nsneuron_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    Neuron::initParams();
    
    try
      {
	aParams.addParam( "NonSpiking", "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
      }

    if (!already)
      {
	try
	  {
	    // no unique parameters
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
void NonSpikingNeuron::initParams() throw ( bad_param, bad_file )
{
  nsneuron_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void NonSpikingNeuron::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a non-spiking neuron called " << iName << std::endl;
#endif

  // Set base voltage
  iVBase = iParams.getParam( "membV" );

  // Set name for spike message
  iMessage.newSource( iName );

  // NonSpikingNeuron stable at membrane potential (shifted to 0)
  iVActual = 0;
  iVTarget = 0;

  // scaling factor for other calculations (timestep/capacitance)
  double tcMemb = - iParams.getParam( "stepT" ) / iParams.getParam( "membC" );

  // scaled membrane conductance
  iGMemb_tc = iParams.getParam( "membG" ) * tcMemb;

  // base decay rate without synapses
  iBaseDecay = exp( iGMemb_tc );

  // set as current
  iDecay = iBaseDecay;

  // scaled sum of input conductances
  iSumG_tc = iGMemb_tc;
  iInputG_tc = 0;

  // scaled current flow across membrane from voltage dependent synaptic inputs
  iSumGV_tc = 0;

  // scaled current flow across membrane from current injection synaptic inputs
  iSumI_tc = 0;

  // Start at base
  iVActual = 0;

  // No timesteps since last output
  iCount = 0;
  iSumVActual = 0;
  iSumSumG_tc = 0.0;
}

/// Do one timestep for self only
bool NonSpikingNeuron::nodeTimestep()
{
  // initialise iSumG_tc
  iSumG_tc = iInputG_tc + iGMemb_tc;

  // Calculate decay from inputs
  iDecay = exp( iSumG_tc );
  iVTarget = iSumGV_tc / iSumG_tc;
  iVActual -= ((iVActual - iVTarget) * 
	       (1.0 - iDecay)) + iSumI_tc;

  // Now reset all inputs
  iSumGV_tc = 0;
  iSumI_tc = 0;
  iInputG_tc = 0;
  iParent->doReceive( iMessage.newAnalogue( iVActual + iVBase ) );

  // output update
  iSumVActual += iVActual;
  iSumSumG_tc += iSumG_tc;
  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& NonSpikingNeuron::externalise( std::ostream& anOStream, 
					     std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();

#ifdef DEBUG
  anOStream << "// Configuring a non-spiking neuron called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Neuron " << iName << " NonSpiking";
  
  if (!iTarget.empty())
    anOStream << " targets " << iTarget;

  if (iParams.getParam( "membV" ) != pParams.getParam( "membV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "membV" );
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

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& NonSpikingNeuron::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputMembV ))
	anOStream << myName << ".iVActual\t";
      if (gOutputLevel.test( outputMembG ))
	anOStream << myName << ".iSumG_tc\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputMembV )) ||
	  (gOutputLevel.test( outputMembG )))
	{
	  anOStream << aName << ".nonspikingneuron = 1;" << std::endl;
	  if (gOutputLevel.test( outputMembG ))
	    {
	      double tcMemb = - iParams.getParam( "stepT" ) /
		iParams.getParam( "membC" );
	      anOStream << aName << ".tcMemb = " << tcMemb << ";" << std::endl;
	      anOStream << aName << ".baseG = "
			<< iParams.getParam( "membG" ) << ";" << std::endl;
	    }
	  
	  if (gOutputLevel.test( outputMembV ))
	    {
	      anOStream << aName << ".baseV = " 
			<< iParams.getParam( "membV" ) << ";" << std::endl;
	    }
	  anOStream << totalName << " = node_convert( " 
		    << aName << " );" << std::endl;
	  anOStream << "clear " << aName << std::endl;
	}
      break;
    default:
      std::cerr << "NonSpikingNeuron::write: Cannot write this output type"
		<< std::endl;
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
double* NonSpikingNeuron::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputMembV ))
	    {
	      *(thePtrPos++) = iSumVActual/iCount + iVBase;
	    }
	  if (gOutputLevel.test( outputMembG ))
	    {
	      *(thePtrPos++) = iSumSumG_tc / iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputMembV ))
	    {
	      *(thePtrPos++) = iVActual + iVBase;
	    }
	  if (gOutputLevel.test( outputMembG ))
	    {
	      *(thePtrPos++) = iSumG_tc;
	    }
	}
      
      iCount = 0;
      iSumVActual = 0;
      iSumSumG_tc = 0;
    }
  else
    {
      if (gOutputLevel.test( outputMembV ))
	{
	  *(thePtrPos++) = iVActual + iVBase;
	}
      if (gOutputLevel.test( outputMembG ))
	{
	  *(thePtrPos++) = iSumG_tc;
	}
    }      
  return thePtrPos;
}

