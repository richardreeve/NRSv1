// Simple neuron
// $Id: PoissonNeuron.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __POISSONNEURON_HH
#define __POISSONNEURON_HH

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

  /// Minimal neurons convert analogue input to poisson spike train
  class PoissonNeuron : public Neuron
  {
  public:
    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    PoissonNeuron( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message, bad_param ) :
      Neuron( aName, aParent, spikeMessage ),
      bSpike( false )
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
    PoissonNeuron( std::string aName, std::string aParentName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, aParentName, spikeMessage ),
      bSpike( false )
    {
      initialise();
    }

    /// Constructor for normal neuron connected to the current context
    /**
     * @param aName unique name for neuron
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    PoissonNeuron( std::string aName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, spikeMessage ),
      bSpike( false )
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
      new PoissonNeuron( aName );
    }

    /// Indicates whether node can receive different messages
    /**
     * @param aMessageType type of message to be sent
     * @return true if message can be received
     */
    virtual bool canReceive( MessageType aMessageType )
    {
      switch (aMessageType)
	{
	case noMessage:
	case analogueMessage:
	case spikeMessage:
	  return true;
	  break;
	default:
	  return Node::canReceive( aMessageType );
	  break;
	}
    }

    /// Receives and processes message
    /**
     * @param aMessage
     * @exception bad_message thrown if inappropriate message is sent
     */
    virtual void doReceive( Message& aMessage )
    {
      switch (aMessage.getType())
	{
	case analogueMessage:
	  iState += static_cast< double >
	    ( dynamic_cast< AnalogueMessage& >( aMessage ).
	      getAnalogue() );
	  break;
	case spikeMessage:
	  bSpike=true;
	  break;
	case noMessage:
	  break;
	default:
	  Node::doReceive( aMessage );
	  break;
	}
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
    /// @var width of interval for poisoon spiking
    double iWidth;

    /// @var lower end of interval for poisson spiking
    double iMin;

    /// @var was there an incoming spike since the last timestep
    bool bSpike;

    /// @var for spike inputs, chance of retransmission
    double iProbBase;
    double iProb;
    /// @var fractional change per unit time of spike to above
    double iProbFrac;
    /// @var decay of iProb back to iProbBase
    double iTProb;

    int iTRefract; // Refractory period is number of timesteps since last spike
    int iAxon; // lesser of no. of timesteps since last spike and iTRefract
    double iSumAxon; // sum of iAxons since last output
    double iSpikes; // number of spikes since last output

    double iState; // current internal state (needed for multiple inputs)

    SpikeMessage iMessage; // message which will be sent to parent.
    
  };
}

#endif //ndef __POISSONNEURON_HH
