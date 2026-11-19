// Basic type of synapse
// $Id: MotorSynapse.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __MOTORSYNAPSE_HH
#define __MOTORSYNAPSE_HH

#include <string>
#include <iostream>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"
#include "ConductanceSynapse.hh"

#pragma interface

namespace Runtime
{

  /// Motor Synapse based on Conductnace Synapse
  class MotorSynapse : public ConductanceSynapse
  {
  public:
    /// Constructor for motor synapse
    /**
     * @param aName unique name for synapse
     * @param aParent pointer to parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    MotorSynapse( std::string aName, Node* aParent, SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      ConductanceSynapse( aName, aParent, aSign )
    {
      initialise();
    }

    /// Constructor for motor synapse
    /**
     * @param aName unique name for synapse
     * @param aParentName name of parent for attachment
     * @param aSign sign of synapse
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    MotorSynapse( std::string aName, std::string aParentName, SynapseSign aSign )
      throw ( bad_node, bad_message, bad_param ) :
      ConductanceSynapse( aName, aParentName, aSign )
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
      MotorSynapse *aCS = new MotorSynapse( aName, aParentName,
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

  };
}

#endif //ndef __MOTORSYNAPSE_HH
