// $Id: SpikeSource.hh,v 1.3 2003/08/05 09:11:37 darrens Exp $

#ifndef __SPIKESOURCE_HH
#define __SPIKESOURCE_HH

#include <string>
#include <iostream>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Neuron.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{

  /// Spiking neuron class
  class SpikeSource : public Neuron
  {
  public:
    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    SpikeSource( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, aParent, spikeMessage )
    {
      initialise();
    }

    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParentName name of parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    SpikeSource( std::string aName, std::string aParentName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, aParentName, spikeMessage )
    {
      initialise();
    }

    /// Constructor for normal neuron connected to the current context
    /**
     * @param aName unique name for neuron
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    SpikeSource( std::string aName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, spikeMessage )
    {
      initialise();
    }

    /// Static constructor
    /**
     * @param aName unique name for neuron
     * @exception bad_node thrown if name already used,
     * or problem with parent
     */
    static void newNeuron( std::string aName )
      throw ( bad_node, bad_message, bad_param )
    {
      SpikeSource *aN;
      aN = new SpikeSource( aName );
    }

    /// Indicates whether node can receive different messages
    /**
     * @param aMessageType type of message to be sent
     * @return true if message can be received
     */
    virtual bool canReceive( MessageType aMessageType )
    {
      return Node::canReceive( aMessageType ); 
    }

    /// Receives and processes message
    /**
     * @param aMessage
     * @exception bad_message thrown if inappropriate message is sent
     */
    virtual void doReceive( Message& aMessage )
    {
      Node::doReceive( aMessage );
    }
    
    /// Do one timestep for self only
    virtual bool nodeTimestep();

    /// Initialises and configures the node
    /**
     * This is called by constructors, but can be called at any point
     * (if parameters have been changed, for instance).
     */
    virtual void initialise();

    /// Initialise parameter names for this node type
    /**
     * @exception bad_param thrown if the parameter already exists
     */
    static void initParams() throw ( bad_param, bad_file );

    /// Output initialisation script for neuron
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for neuron
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& write( std::ostream& anOStream,
				 OutputType aType = outTypeText );

    /// Output current state of node to array
    /**
     * This is used by all the other put methods to gather data
     * @param anArray array of doubles to store data in
     * @param average true to get average since last reading, else current
     * @return pointer to end of used part of array
     */
    virtual double* put( double* anArray, bool average = true );

  protected:
    
    // Number of spikes since last output
    double iSpikes;

    // Was a spike fired on the current timestep?
    bool iNowFiring;

    // Number of timesteps before spike is generated.
    int iSpikeTime; 
    
    // Number of timesteps, after the the start of spikes, before all spike
    // production is halted.
    int iSpikeDuration; 

    // This is a counter which is decremented every timesteps once
    // spikes have been initiated (assumine iSpikeDuration is
    // non-negative). Once this counter gets to zero, no more spikes are
    // generated.
    int iLifeTime;
    
    // State of the durationation timer.
    enum {TimerNotUsed, NotStarted, Started, Finished} iState;

    // The time period, in simulation timsteps, to repeat the generation
    // of an output spike after the first has been produced.  If value
    // is zero, then there is no repitition of spikes.
    int iSpikePeriod;  
    
    // The time, in simulation timesteps, until the next spike is
    // generated. If this ever becomes negative, no more spikes will be
    // produced.
    int iTimeTilNextSpike;
    
    SpikeMessage iMessage; // message which will be sent to parent.
    
  };
}

#endif
