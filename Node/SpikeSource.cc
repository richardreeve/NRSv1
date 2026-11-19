// $Id: SpikeSource.cc,v 1.5 2003/10/16 17:45:45 darrens Exp $

#include <iostream>
#include <string>

#include <cmath>

#pragma implementation

#include "Neuron.hh"
#include "Exception.hh"
#include "SpikeSource.hh"
#include "Params.hh"

#include "SpikeSource.hh"

#define PARAM_TIME "spikeTime"
#define PARAM_PERIOD "spikePeriod"
#define PARAM_END "spikeDuration"
#define SPIKE_SOURCE "SpikeSource"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void spike_source_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    Neuron::initParams();
    
    try
      {
	aParams.addParam(SPIKE_SOURCE, "enum" );
      }
    catch ( bad_param& bp )
      {
	already=true;
        _WARN_("Exception: " << bp.what() << "; Hint: " << bp.hint() 
                    << "; Thrower: " << bp.origin());
      }

    if (!already)
      {
	try
	  {
	    aParams.addParam(PARAM_TIME, "ms");
	    aParams.addParam(PARAM_PERIOD, "ms");
	    aParams.addParam(PARAM_END, "ms");
	  }
	catch ( bad_param& bp )
	  {
            _WARN_("Exception: " << bp.what() << "; Hint: " << bp.hint() 
                    << "; Thrower: " << bp.origin());
	    throw;
	  }
      }
  }
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void SpikeSource::initParams() throw ( bad_param, bad_file )
{
  spike_source_init( gRoot );
}
//-----------------------------------------------------------------------------
/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void SpikeSource::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a neuron called " << iName << std::endl;
#endif

  // Set name for spike message
  iMessage.newSource( iName );

  // Initialise internal variables
  iSpikeTime = (int) 
    (iParams.getParam(PARAM_TIME)/iParams.getParam("stepT"));
  iSpikePeriod = (int) 
    (iParams.getParam(PARAM_PERIOD)/iParams.getParam("stepT"));
  iSpikeDuration = (int) 
    (iParams.getParam(PARAM_END)/iParams.getParam("stepT"));

  // Data correction.  If the user sets PARAM_PERIOD to zero, it means
  // there is no spike repitition.  Inside the code however, a value of
  // -1 represents no repitition.  
  if (iSpikePeriod == 0) iSpikePeriod = -1;

  // Data validation. If there is a negative spike time, give a warning
  // that a spike will never be produced.
  if (iParams.getParam(PARAM_TIME) <= 0)
    {
      _WARN_("Parameter " << PARAM_TIME << " for neuron (" 
             << SPIKE_SOURCE << "/" << iName
             << ") has negative or zero value. "
             << "A spike will never be produced.");

      // By setting this to negative, we effectively disable this neuron.
      iTimeTilNextSpike = -1;
    }
  else
    {
      iTimeTilNextSpike = iSpikeTime;
    }

  // No spikes at start
  iSpikes = 0;
  iNowFiring = false;

  // No timesteps since last output
  iCount = 0.0;

  // Set the initial state of the timer
  if (iSpikeDuration == 0)
    {
      // No timer will ever be used if SpikeDuration is 0
      iState = TimerNotUsed;
   }
  else
    {
      // The duration timer is has not yet begun
      iState = NotStarted;
    }

  // Also, if the iSpikePeriod is -1, it means there will never be
  // repeated spikes, so again, the spike timer will not be used.
  if (iSpikePeriod == -1) iState = TimerNotUsed;
}

//-----------------------------------------------------------------------------
/// Do one timestep for self only
bool SpikeSource::nodeTimestep()
{
  // ???? What does this do?
  iCount++;

  // Reset the 'currently firing flag'.
  iNowFiring = false;

  // If there is no time til another spike is due, then immediately exit
  // since there is nothing for us to do.
  if (iTimeTilNextSpike < 0) return true;

  // Decrement spike wait time
  iTimeTilNextSpike--;

  
  // If spike wait time has elapsed, send out a spike. 
  if (iTimeTilNextSpike == 0)
    {
      iNowFiring = true;
      
      // Send out a spike
      iParent->doReceive( iMessage );
      iSpikes++;

      // Now set the next wait period
      iTimeTilNextSpike = iSpikePeriod;

      // Start the duration timer, if it has not already been started
      if (iState == NotStarted)
        {
          iState = Started;
        }
    }

  // Decrement the spike-duration life-time counter
  if (iState == Started)
    {
      // check for expiration of the timer
      if (iSpikeDuration == 0)
        {
          iState = Finished;

          // Set this variable which should effectively prevent any more
          // spikes from being generated.
          iTimeTilNextSpike = -1;
        }
      iSpikeDuration--;      
    }
  
  return true;
}
//-----------------------------------------------------------------------------
/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& SpikeSource::externalise( std::ostream& anOStream, 
					  std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();

#ifdef DEBUG
  anOStream << "// Configuring a spiking neuron called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Neuron " << iName << SPIKE_SOURCE;

  if (!iTarget.empty())
    anOStream << " targets " << iTarget;
  
  if (iParams.getParam(SPIKE_SOURCE) != pParams.getParam(SPIKE_SOURCE))
    {
      do_params(extras, anOStream, indent);
      anOStream << indent;
      iParams.put(anOStream, SPIKE_SOURCE);
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
std::ostream& SpikeSource::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputSpike ))
	anOStream << myName << ".iSpikes\t";
      break;
    case outTypeMatlab:
      // TODO - sort out this setion once I understand what it is doing
      if (gOutputLevel.test( outputSpike ))
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
      std::cerr << "SpikeSource::write: Cannot write this output type"
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
double* SpikeSource::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = iSpikes/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = (iNowFiring? 1.0 : 0.0);
	    }
	}
      iCount = 0;
      iSpikes = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSpike ))
	{
	  *(thePtrPos++) = (iNowFiring? 1.0 : 0.0);
	}
    }      
  return thePtrPos;
}
