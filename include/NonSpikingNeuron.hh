// Basic type of neuron
// $Id: NonSpikingNeuron.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __NONSPIKINGNEURON_HH
#define __NONSPIKINGNEURON_HH

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

  /// Non-spiking neuron
  class NonSpikingNeuron : public Neuron
  {
  public:
    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    NonSpikingNeuron( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, aParent, analogueMessage )
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
    NonSpikingNeuron( std::string aName, std::string aParentName )
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, aParentName, analogueMessage )
    {
      initialise();
    }

    /// Constructor for normal neuron connected to current context
    /**
     * @param aName unique name for neuron
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    NonSpikingNeuron( std::string aName )
      throw ( bad_node, bad_message, bad_param ) : 
      Neuron( aName, analogueMessage )
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
      NonSpikingNeuron *aN;
      aN = new NonSpikingNeuron( aName );
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
	case currentMessage:
	case conductanceMessage:
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
	case currentMessage:
	  dynamic_cast< CurrentMessage& >( aMessage ).
	    addCurrent( iSumI_tc );
	  break;
	case conductanceMessage:
	  dynamic_cast< ConductanceMessage& >( aMessage ).
	    addConductance( iInputG_tc, iSumGV_tc );
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
    // all values compensate to make base voltage=0  
    double iVBase; // Base level for neuron
    double iVTarget;    // current potential across membrane 
    //                  v_actual will decay to this value over time
    double iDecay;  // decay rate to target - proportion of (iVActual-iVTarget)
    //                                         removed after 1 t'step
    double iVActual;  // current potential inside cell 
    //                  (actual value is (iVActual*resV+membV)/10 mV)
    double iSumVActual;  // Sum of iVActuals since last output
    int iAxon; // lesser of no. of timesteps since last spike and iTRefract
    // iTCMemb is a useful number: from V(t)=V(0)*exp(-t*iSumG/membC)
    //                  (iSumG is conductance across membrane and synapse)
    //                  useful to have -stepT/membC precalculated
    //                  so iTCMemb = -timestep/membrane capacitance
    //                             = -(stepT/resT)/(membC/resC)
    double iGMemb_tc; // membrane conductance (membG/resG) scaled by iTCMemb
    //                  V(t+stepT)=V(t)*exp(iGMemb*iTCMemb=iGMemb_tc)
    //                  when no synaptic inputs
    double iBaseDecay; // base value for decay when no voltage dependent inputs
    //                  base_decay = 1 - exp(iGMemb*iTCMemb)
    
    // NOTE: all sums are now prescaled by iTCMemb (shown by _tc postfix)
    
    // These are the parameters affected by the voltage dependent synapses
    double iInputG_tc;  // sum of voltage dependent synaptic input conductances
    //                  = sum(syn_v[]->iGActual)
    double iSumG_tc;  // total conductance
    //        = iGMemb + iInput_tc
    double iSumSumG_tc;  // Sum of total conductances since last output
    double iSumGV_tc; // total current flow across membrane
    //             = sum of current inputs from voltage dependent synapses
    //             iSumG=0+sum(syn_v[]->iGActual*syn_v[]->iVBattery)
    // iVTarget=iSumGV/iSumG
    // decay=1-exp(-stepT/resT*iSumG/(membC/resC))=1-exp(iSumG*iTCMemb)
    
    // This is the one affected by current injection 
    //                             and non-spiking sensor synapses
    double iSumI_tc;  // total current flow introduced by all current synapses
    //            sum_I = sum(syn(i/s)[]->iIActual)
    // iVActual=iVActual-(iVActual-iVTarget)*decay+iSumI*stepT/(resT*iCMemb)
    // => iVActual-=(iVActual-iVTarget)*decay+sum_I*iTCMemb

    // NOTE: iSpike if a virtual variable, 1 iff iAxon == 0,
    // which is convenient for output
    
    AnalogueMessage iMessage; // message which will be sent to parent.
    
  };
}

#endif //ndef __NONSPIKINGNEURON_HH
