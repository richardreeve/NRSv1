// $Id: ConditionableSynapse.hh,v 1.4 2003/09/25 17:40:15 darrens Exp $

#ifndef __CONDITIONABLESYNAPSE_HH
#define __CONDITIONABLESYNAPSE_HH

#include <string>
#include <cmath>

#include "base.h"

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"
#include "PlainSynapse.hh"

#pragma interface

namespace Runtime
{

  /// ConditionableSynapse 
  class ConditionableSynapse : public PlainSynapse
  {
  public:
    /// Constructor
    /**
     * @param aName unique name for synapse
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    ConditionableSynapse( std::string aName, Node* aParent)
      throw ( bad_node, bad_message, bad_param ) :
      PlainSynapse(aName, aParent)
    {
      initialise();
    }

    /// Constructor
    /**
     * @param aName unique name for synapse
     * @param aParentName name of parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent
     */
    ConditionableSynapse( std::string aName, std::string aParentName)
      throw ( bad_node, bad_message, bad_param ) :
      PlainSynapse(aName, aParentName)
    {
      initialise();
    }

    /// Static constructor
    /**
     * @param aName unique name for synapse
     * @param aSourceName name of origin of synapse
     * @param aParentName name of destination of synapse
     * @param aDelay delay on synapse
     * @exception bad_node thrown if name already used,
     * or problem with parent or source
     */
    static void newSynapse( std::string aName, std::string aSourceName,
			    std::string aParentName, double aDelay = -1.0 ) 
      throw ( bad_node, bad_message, bad_param )
    {
      ConditionableSynapse *aCS = new ConditionableSynapse(aName, aParentName);

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
          // Call base class
	  return PlainSynapse::canReceive(aMessageType);
	  break;
	}
    }

    /// Receives and processes message
    /**
     * @param aMessage
     * @exception bad_message thrown if inappropriate message is sent
     */
    virtual void doReceive(Message& aMessage)
    {
      switch (aMessage.getType())
	{
	case spikeMessage:
	  // Update synaptic conductance with facilitatory conductance
          // level. The spike is then passed to the base class, which
          // adds on the base conductance.
          iGActual_tc += iFacG_tc;

          // Increment the CS-activity-trace indicator
          iCSTrace += 1.0;

	  break;
          
	case analogueMessage:
          {
            double Gchange = 0;
            dynamic_cast<AnalogueMessage&>(aMessage).addAnalogue(Gchange);

            // Now convert this conductance change value into _tc
            // dimension
            double Gchange_tc = Gchange * iTC;

            // Finally increase the facilitation conductance
            iFacG_tc += Gchange_tc + (iTraceFactor * iCSTrace * Gchange_tc);

            //_TRACE_(   "Me=" << this
            //        << "; iFacG_tc=" << iFacG_tc 
            //        << "; Gchange_tc=" << Gchange_tc
            //        << "; Gchange=" << Gchange
            //        << "; iGActual_tc=" << iGActual_tc
            //        << "; iGBase_tc=" << PlainSynapse::iGBase_tc);

            break;
          }
	default:
          // Usually we call the Node::doReceive(...) method here.  It's
          // not needed for this member since a call to the most
          // immediate base class is made further down.  The base class
          // member called will handle 'default'
	  break;
	}

      // Pass message to base class for further processing
      PlainSynapse::doReceive(aMessage);
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
     *
     * Since this constructor is not allowed, I have left it undefined.
     * So if an attempt is made to use this constructor, there should be
     * build errors.
     */
    ConditionableSynapse( std::string aName ) throw ( bad_node );

  protected:

    // Current level of facilitatory conductance. The underscore tc
    // denotes that this value needs to be divided by - stepT / membC to
    // retreive the conductance value in 'nS' dimension.
    double iFacG_tc;

    // Current value of the CS trace which reflects the prior amount of
    // CS spike activity in this synapse.
    double iCSTrace;
   
    // Decay constant for facilitatory-conductance. 
    double iFacDecay;
   
    // Decay constant for the CS activity trace. 
    double iTraceDecay;

    // The factor which trace activity is multiplied
    double iTraceFactor;

    // Sum of facilitatory conductances since last output - used to
    // output averages.  The underscore tc denotes that this value needs
    // to be divided by - stepT / membC to retreive the conductance
    // value in 'nS' dimension.
    double iSumFacG_tc;

    // Store the product of stepT and membrance Conductance.  This
    // quantity is usually calculated locally within ::initialise()
    // members, and does not need to be stored.  However, with
    // ConditionableSynapse, the conductance-change messages recevied
    // are not in the _tc format, whereas internally stored conductance
    // values - specifically 'G-Actual' and 'iFacG_tc' - are. So they
    // can't be immediately combined.  To combine the conductance-change
    // message with a _tc value, the former needs to be multiplied by
    // the TC value. This is, of course, slow and undesirable.  Further
    // work might allow the modulatory synapse which sends the
    // conductance-change messages to determine TC for itself.  But
    // until then, the conversion needs to take place here, and storing
    // the TC value is a slight optimisation.
    double iTC;

  };
}

#endif //ndef __CONDITIONABLESYNAPSE_HH
