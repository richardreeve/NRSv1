// Basic type of neuron
// $Id: MotorNeuron.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __MOTOR_HH
#define __MOTOR_HH

#include <string>
#include <iostream>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "NonSpikingNeuron.hh"
#include "Neuron.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{
  /// Class for motor neurons
  class MotorNeuron : public NonSpikingNeuron
  {
  public:
    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    MotorNeuron( std::string aName, Node* aParent )
      throw ( bad_node, bad_message, bad_param ) : 
      NonSpikingNeuron( aName, aParent )
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
    MotorNeuron( std::string aName, std::string aParentName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      NonSpikingNeuron( aName, aParentName )
    {
      initialise();
    }
    
    /// Constructor for normal neuron connected to current context
    /**
     * @param aName unique name for neuron
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    MotorNeuron( std::string aName ) 
      throw ( bad_node, bad_message, bad_param ) : 
      NonSpikingNeuron( aName )
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
      new MotorNeuron( aName );
    }
    
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
    
  };
}

#endif //ndef __MOTOR_HH
