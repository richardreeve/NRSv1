// Basic type of neuron
// $Id: Neuron.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __NEURON_HH
#define __NEURON_HH

#include <string>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{

  /// Base class for all Neurons
  class Neuron : public Node
  {
  public:
    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParent pointer to parent for attachment
     * @param anMType type of message it will send to parent by default
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
     Neuron( std::string& aName, Node* aParent, MessageType anMType ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Node( aName, aParent, anMType, false ),
      iTarget( "" )
    {}

    /// Constructor for normal neuron
    /**
     * @param aName unique name for neuron
     * @param aParentName name of parent for attachment
     * @param anMType type of message it will send to parent by default
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    Neuron( std::string aName, std::string aParentName, MessageType anMType ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Node( aName, aParentName, anMType ),
      iTarget( "" )
    {}

    /// Constructor for normal neuron connected to the current context
    /**
     * @param aName unique name for neuron
     * @param anMType type of message it will send to parent by default
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    Neuron( std::string aName, MessageType anMType ) 
      throw ( bad_node, bad_message, bad_param ) : 
      Node( aName, anMType, false ),
      iTarget( "" )
    {}

    /// Set the main target of the neuron (particularly for motorneurons)
    /**
     * @param aName the name of the target of the neuron 
     */
    virtual void setTarget( std::string aName )
    {
      if (!iTarget.empty())
	removeConnection( iTarget );
      iTarget = aName;
      if (aName != "")
	addConnection( iTarget, true, 1 );
      doInitialise();
    }

    /// Initialise parameter names for this node type
    /**
     * @exception bad_param thrown if the parameter already exists
     */
    static void initParams() throw ( bad_param, bad_file );

  protected:
    std::string iTarget;
  };
}

#endif //ndef __NEURON_HH
