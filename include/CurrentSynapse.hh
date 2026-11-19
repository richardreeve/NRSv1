// Basic type of synapse
// $Id: CurrentSynapse.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __CURRENTSYNAPSE_HH
#define __CURRENTSYNAPSE_HH

#include <string>
#include <cmath>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{
  
  /// Current injection synapses
  class CurrentSynapse : public Node
  {
  public:
    /// Constructor for current synapse
    /**
     * @param aName unique name for synapse
     * @param aParent pointer to parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    CurrentSynapse( std::string aName, Node* aParent,
			SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      Node( aName, aParent, currentMessage, false ),
      iSign( aSign )
    {
      initialise();
    }

    /// Constructor for current synapse
    /**
     * @param aName unique name for synapse
     * @param aParentName name of parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    CurrentSynapse( std::string aName, std::string aParentName,
			SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      Node( aName, aParentName, currentMessage, false ),
      iSign( aSign )
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
      CurrentSynapse *aCS = new CurrentSynapse( aName, aParentName, aSign );

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
	case spikeMessage:
	case analogueMessage:
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
      double ana;
      switch (aMessage.getType())
	{
	case noMessage:
	  break;
	case spikeMessage:
	  // update synaptic current
	  iIActual_tc += (iIBase_tc + iIFacReal_tc) * iDepRealFrac;
	  // and increase facilitation/depression
	  iIFacReal_tc += iIFac_tc;
	  iDepRealFrac *= iDepFrac;
	  break;
	case analogueMessage:
	  ana = dynamic_cast< AnalogueMessage& >( aMessage ).
	    getAnalogue();
	  // update synaptic current
	  iIActual_tc += (iICBase_tc + iIFacReal_tc) * iDepRealFrac * ana;
	  // and increase facilitation/depression
	  iIFacReal_tc += iICFac_tc * ana;
	  iDepRealFrac *= exp( iCDepFrac_ln * ana );
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

    CurrentSynapse( std::string aName ) throw ( bad_node ) :
      Node( aName ) 
    {
      throw bad_node( iName );
    }

  protected:
    
    SynapseSign iSign;

    double iIBase_tc; // base current [ei]cSynI
    double iICBase_tc; // same for continuous input a[ei]cSynIT*stepT
    double iDecay; // Decay rate for current (fraction left)
    // decay=exp(ln(1/2)*stepT/expSynT)
    double iIActual_tc; // actual current after facilitiation/depression
    // scaled facilitatory current increase
    double iSumIActual_tc; // Sum of iIActual_tcs since last output
    double iIFac_tc; // increase in current after a spike (cSynFacI)
    double iICFac_tc; // increase in current per input per timestep 
    // (acSynFacIT*stepT)
    double iTFac;  // decay rate of facilitatory current (fraction left)
    // t_fac=exp(ln(1/2)*stepT/synFacT)
    double iIFacReal_tc; // actual current due to facilitation
    double iSumIFacReal_tc; // Sum of iIFacReal_tcs since last output
    double iDepFrac; //fractional decrease in current after spike (cSynDepFrac)
    double iCDepFrac_ln; // log of decrease in current per timestep per unit 
    //                     =ln(acSynDepFracT)*stepT
    double iTDep; // decay rate of depressive fraction to 1
    // t_dep=1-exp(ln(1/2)*stepT/synDepT)
    double iDepRealFrac; // actual fraction of current left after depression
    // i_actual=(i_base+i_fac_real)*i_dep_real_frac;
    double iSumDepRealFrac; // Sum of iDepRealFracs since last output

    CurrentMessage iMessage; // message to be sent to parent.
    
  };
}

#endif //ndef __CURRENTSYNAPSE_HH
