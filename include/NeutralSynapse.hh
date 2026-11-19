// Basic type of synapse
// $Id: NeutralSynapse.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __NEUTRALSYNAPSE_HH
#define __NEUTRALSYNAPSE_HH

#include <string>
#include <iostream>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{

  /// Minimal Synapse
  class NeutralSynapse : public Node
  {
  public:
    /// Constructor for neutral synapse
    /**
     * @param aName unique name for synapse
     * @param aParent pointer to parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    NeutralSynapse( std::string aName, Node* aParent,
		    SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      Node( aName, aParent, analogueMessage, false ),
      bAMessage( false ), bSMessage( false )
    {
      initialise();
    }

    /// Constructor for neutral synapse
    /**
     * @param aName unique name for synapse
     * @param aParentName name of parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    NeutralSynapse( std::string aName, std::string aParentName,
		    SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      Node( aName, aParentName, analogueMessage, false ),
      bAMessage( false ), bSMessage( false )
    {
      initialise();
    }

    /// Static constructor
    /**
     * @param aName unique name for synapse
     * @param aSourceName name of origin of synapse
     * @param aParentName name of destination of synapse
     * @param aSign sign of synapse
     * @param aDelay delay on synapse
     * @exception bad_node thrown if name already used,
     * or problem with parent or source
     */
    static void newSynapse( std::string aName, std::string aSourceName,
			    std::string aParentName, SynapseSign aSign,
			    double aDelay = -1.0 ) 
      throw ( bad_node, bad_message, bad_param )
    {
      NeutralSynapse *aCS = new NeutralSynapse( aName, aParentName,
						aSign );
      
      // Is delay set?
      if (aDelay >= 0.0)
	aCS->getParams().setParam( "synDelayT", aDelay );
      
      unsigned int delay = static_cast< unsigned int >
			  ( rint( aCS->getParams().getParam( "synDelayT" ) /
				  aCS->getParams().getParam( "stepT" ) ) );
      if (delay == 0)
	{
	  std::cerr << "newSynapse: Effective delay of 0 timesteps requested"
		    << std::endl
		    << "            - rejected, set to 1 for "
		    << aCS->getName(true) << std::endl;  
	  delay = 1;
	}

      aCS->addConnection( aSourceName, aName, delay );
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
	case noMessage:
	  break;
	case analogueMessage:
	  iAnalogue = dynamic_cast< AnalogueMessage& >
	    ( aMessage ).getAnalogue();
	  iMessage.newAnalogue( iAnalogue );
	  bAMessage = true;
	  break;
	case spikeMessage:
	  bSMessage=true;
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

    /// Output initialisation script for synapse
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for synapse
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

  private:
    /// Constructor for root synapse not allowed
    /**
     * Nor is one for synapse connected direct to root...
     * @param aName unique name for router
     * @exception bad_node thrown if synapse cannot be root
     */

    NeutralSynapse( std::string aName ) throw ( bad_node ) :
      Node( aName ) 
    {
      throw bad_node( iName );
    }

  protected:
    
    bool bAMessage; // did a message arrive
    bool bSMessage; // did a message arrive
    double iAnalogue; // current message to pass on
    double iSumAnalogue; // Sum of above

    AnalogueMessage iMessage; // message to be sent to parent.
    SpikeMessage iSpikeMessage; // message to be sent to parent.
    
  };
}

#endif //ndef __NEUTRALSYNAPSE_HH
