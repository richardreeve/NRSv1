// $Id: SpikingNeuron.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <string>

#include <cmath>

#pragma implementation

#include "Neuron.hh"
#include "SpikingNeuron.hh"
#include "Params.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void sneuron_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    Neuron::initParams();
    
    try
      {
	aParams.addParam( "Spiking", "enum" );
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
	    aParams.addParam( "threshV", "mV" );
	    aParams.addParam( "recovV", "mV" );
	    aParams.addParam( "spikeV", "mV" );
	    aParams.addParam( "refractT", "ms" );
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
void SpikingNeuron::initParams() throw ( bad_param, bad_file )
{
  sneuron_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void SpikingNeuron::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a neuron called " << iName << std::endl;
#endif

  // Set name for spike message
  iMessage.newSource( iName );

  // Initialise internal variables
  iVBase = iParams.getParam( "membV" );
  iVThreshold = iParams.getParam( "threshV" ) - iParams.getParam( "membV" );
  iVRecovery = iParams.getParam( "recovV" ) - iParams.getParam( "membV" );

  // SpikingNeuron stable at membrane potential (shifted to 0)
  iVActual = 0;
  iVTarget = 0;

  // Refractory period is number of timesteps since last spike
  iTRefract = static_cast< int >( rint( iParams.getParam( "refractT" )
					/ iParams.getParam( "stepT" ) ) );

  // SpikingNeuron is initially not refractory
  iAxon = iTRefract >? 1;

  // No spikes at start
  iSpikes = 0;

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
  iCount = 0.0;
  iSumAxon = 0.0;
  iSumVActual = 0.0;
  iSumSumG_tc = 0.0;
}

/// Do one timestep for self only
bool SpikingNeuron::nodeTimestep()
{
  // initialise iSumG_tc
  iSumG_tc = iInputG_tc + iGMemb_tc;

  // Check whether still in refractory period after spike (haven't incremented)
  if (iTRefract > iAxon)
    {
      // Only increment if refractory
      iAxon++;
      // If so, no inputs, just decay
      // Don't even need to do this:
      //      iDecay = iBaseDecay;
      //      iVTarget = 0;
      //      iSumI_tc = 0;
      iVActual *= iBaseDecay;
    }
  else
    { // Else calculate decay from inputs
      iDecay = exp( iSumG_tc );
      iVTarget = iSumGV_tc / iSumG_tc;
      iVActual -= ((iVActual - iVTarget) * 
		   (1.0 - iDecay)) + iSumI_tc;
    }

  // Are we above threshold?
  if (iVActual >= iVThreshold)
    { // Spike into axon
      iAxon = 0;
      iSpikes++;
      iVActual = iVRecovery;
      iParent->doReceive( iMessage );
    }
  else if (iAxon == 0)
    { // iTRefract = 0, but we don't want people to think we're firing!
      iAxon++;
    }

  // Now reset all inputs
  iInputG_tc = 0;
  iSumGV_tc = 0;
  iSumI_tc = 0;
  
  // output update
  iSumAxon += iAxon;
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
std::ostream& SpikingNeuron::externalise( std::ostream& anOStream, 
					  std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();

#ifdef DEBUG
  anOStream << "// Configuring a spiking neuron called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Neuron " << iName << " Spiking";

  if (!iTarget.empty())
    anOStream << " targets " << iTarget;
  
  if (iParams.getParam( "membV" ) != pParams.getParam( "membV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "membV" );
    }

  if (iParams.getParam( "threshV" ) != pParams.getParam( "threshV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "threshV" );
    }

  if (iParams.getParam( "recovV" ) != pParams.getParam( "recovV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "recovV" );
    }

  if (iParams.getParam( "spikeV" ) != pParams.getParam( "spikeV" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "spikeV" );
    }

  if (iParams.getParam( "refractT" ) != pParams.getParam( "refractT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "refractT" );
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
std::ostream& SpikingNeuron::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputSpike ))
	anOStream << myName << ".iSpikes\t";
      if (gOutputLevel.test( outputAxon ))
	anOStream << myName << ".iAxon\t";
      if (gOutputLevel.test( outputMembG ))
	anOStream << myName << ".iSumG_tc\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputMembV )) ||
	  (gOutputLevel.test( outputSpike )) ||
	  (gOutputLevel.test( outputAxon )) ||
	  (gOutputLevel.test( outputMembG )))
	{
	  anOStream << aName << ".spikingneuron = 1;" << std::endl;
	  if (gOutputLevel.test( outputMembG ))
	    {
	      double tcMemb = - iParams.getParam( "stepT" ) /
		iParams.getParam( "membC" );
	      anOStream << aName << ".tcMemb = " << tcMemb << ";" 
			<< std::endl;
	      anOStream << aName << ".baseG = "
			<< iParams.getParam( "membG" ) << ";" << std::endl;
	    }

	  if (gOutputLevel.test( outputMembV ))
	    {
	      anOStream << aName << ".thresholdV = "
			<< iParams.getParam( "threshV" ) << ";" << std::endl;
	      anOStream << aName << ".spikeV = "
			<< iParams.getParam( "spikeV" ) << ";" << std::endl;
	      anOStream << aName << ".recoveryV = " 
			<< iParams.getParam( "recovV" ) << ";" << std::endl;
	      anOStream << aName << ".baseV = " 
			<< iParams.getParam( "membV" ) << ";" << std::endl;
	    }
	  anOStream << aName << ".refractT = "
		    << iParams.getParam( "refractT" ) << ";" << std::endl;
	  anOStream << totalName << " = node_convert( " 
		    << aName << " );" << std::endl;
	  anOStream << "clear " << aName << std::endl;
	}
      break;
    default:
      std::cerr << "SpikingNeuron::write: Cannot write this output type"
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
double* SpikingNeuron::put( double* anArray, bool average )
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
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = iSpikes/iCount;
	    }
	  if (gOutputLevel.test( outputAxon ))
	    {
	      *(thePtrPos++) = iSumAxon/iCount;
	    }
	  if (gOutputLevel.test( outputMembG ))
	    {
	      *(thePtrPos++) = iSumSumG_tc/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputMembV ))
	    {
	      *(thePtrPos++) = iVActual + iVBase;
	    }
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = (iAxon == 0 ? 1.0 : 0.0);
	    }
	  if (gOutputLevel.test( outputAxon ))
	    {
	      *(thePtrPos++) = static_cast< double >( iAxon );
	    }
	  if (gOutputLevel.test( outputMembG ))
	    {
	      *(thePtrPos++) = iSumG_tc;
	    }
	}
      
      iCount = 0;
      iSpikes = 0;
      iSumAxon = 0;
      iSumVActual = 0;
      iSumSumG_tc = 0;
    }
  else
    {
      if (gOutputLevel.test( outputMembV ))
	{
	  *(thePtrPos++) = iVActual + iVBase;
	}
      if (gOutputLevel.test( outputSpike ))
	{
	  *(thePtrPos++) = (iAxon == 0 ? 1.0 : 0.0);
	}
      if (gOutputLevel.test( outputAxon ))
	{
	  *(thePtrPos++) = static_cast< double >( iAxon );
	}
      if (gOutputLevel.test( outputMembG ))
	{
	  *(thePtrPos++) = iSumG_tc;
	}
    }      
  return thePtrPos;
}
