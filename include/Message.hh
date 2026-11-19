// Base for runtime messages
// $Id: Message.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __MESSAGE_HH
#define __MESSAGE_HH

#include <string>
#include <list>
#include "Exception.hh"

#pragma interface

namespace Runtime
{
  class Node;
  

  /// Message enums
  /**
   * Type of messages which can be sent
   */
  typedef enum MessageType
  {
    spikeMessage,            ///< Spike transmission
    currentMessage,          ///< Current injection
    analogueMessage,         ///< Analogue input
    conductanceMessage,      ///< Membrane conductance change
    noMessage,               ///< No message is sent
    newConnectionMessage,    ///< Request for new connection
    removeConnectionMessage, ///< Request for removal of connection
    newRobotConnMessage,     ///< Request for new connection to robot
    removeRobotConnMessage,  ///< Request for removal of robot connection
    manySpikeMessage         ///< Multiple spike transmission
  } MessageType;

  /// Exception class indicating message was inappropriate
  class bad_message : public runtime_exception
  {
  public:
    /// constructor for specific message type
    bad_message( MessageType aMessageType )
    {
      switch (aMessageType)
	{
	case spikeMessage:
	  iName = "spikeMessage";
	  break;
	case currentMessage:
	  iName = "currentMessage";
	  break;
	case analogueMessage:
	  iName = "analogueMessage";
	  break;
	case conductanceMessage:
	  iName = "conductanceMessage";
	  break;
	case noMessage:
	  iName = "noMessage";
	  break;
	case newConnectionMessage:
	  iName = "newConnectionMessage";
	  break;
	case removeConnectionMessage:
	  iName = "removeConnectionMessage";
	  break;
	default:
	  iName = "*** unknown message type ***";
	}
    }
  };

  /// Base class for messages transmitted by nodes
  /**
   * defaults to noMessage type, carrying nothing
   */
  class Message
  {
  public:
    /// default constructor for messages
    Message() : iType( noMessage ), iUpdate( 0 ), iTargets( NULL ),
		iRouter( NULL ) {}

    /// default constructor for specific messages
    /**
     * @param aMessageType the type of message being sent
     */
    Message( MessageType aMessageType ) : iType( aMessageType ),
					  iUpdate( 0 ), iTargets( NULL ),
					  iRouter( NULL ) {}

    /// default constructor for specific messages
    /**
     * @param aMessageType the type of message being sent
     * @param aSource the source of the message
     */
    Message( MessageType aMessageType, std::string& aSource ) 
      : iSource( aSource ), iType( aMessageType ),
	iUpdate( 0 ), iTargets( NULL ), iRouter( NULL ) {}

    /// standard destructor
    virtual ~Message() {}

    /// accessor for message type
    MessageType getType()
    {
      return iType;
    }


    /// Accessor for name of source
    /**
     * This method adds the message to the input current
     * @param aCurrent subtotal of current flow
     */
    std::string getSource()
    {
      return iSource;
    }
    
    /// Mutator for name of source
    /**
     * This method sets the source for the message
     * @param aSource new source
     * @return reference to self
     */
    Message& newSource( std::string& aSource )
    {
      iSource=aSource;
      return *this;
    }
    
    /// Accessor for current update level
    /**
     * @return whether message is up to date
     */
    bool needUpdate( unsigned int anUpdate )
    {
      return (iUpdate != anUpdate);
    }

    /// Accessor for targets for message
    /**
     * @return list of targets of message
     */
    std::list< std::pair< Node*, unsigned int > >*  getTargets()
    {
      return iTargets;
    }

    /// Mutator for list of targets
    /**
     * This method sets the list of targets, which needs to be updated
     * whenever there is a update to the router
     * @param aList the list of targets for the message
     * @param anUpdate the new update reference
     */
    void newTargets( std::list< std::pair< Node*, unsigned int > >* theTargets,
		     unsigned int anUpdate )
    {
      iTargets = theTargets;
      iUpdate = anUpdate;
    }

    /// Check whether router is immediate parent, set if none
    /**
     * if not we will ignore the target info in the message
     * @param aRouter the router to be tested
     * @return whether the router is immediate
     */
    bool checkRouter( Node* aRouter )
    {
      if (iRouter == aRouter)
	return true;
      if (iRouter == NULL)
	{
	  iRouter = aRouter;
	  return true;
	}
      return false;
    }

  protected:
    /// Origin of message (optional)
    std::string iSource;

    /// MessageType of message
    MessageType iType;

    /// list of targets for this message
    unsigned int iUpdate;
    std::list< std::pair< Node*, unsigned int > >* iTargets;
    Node* iRouter;

  };

  /// Class for spike messages transmitted by nodes
  /**
   * these are used by neurons to communicate to synapses
   */
  class SpikeMessage : public Message
  {
  public:
    /// default constructor for SpikeMessage
    SpikeMessage( std::string aSource ) : Message( spikeMessage, aSource ) {}

    /// default empty constructor for SpikeMessage
    SpikeMessage() : Message( spikeMessage ) {}

    /// standard destructor
    virtual ~SpikeMessage() {}
  };

  /// Class for current injection messages transmitted by nodes
  /**
   * these are used by synapses to communicate to neurons
   */
  class CurrentMessage : public Message
  {
  public:
    /// default constructor for CurrentMessage
    CurrentMessage( double aCurrent ) : Message( currentMessage ),
				     iCurrent( aCurrent ) {}

    /// default empty constructor for CurrentMessage
    CurrentMessage( std::string aSource ) : Message( currentMessage,
						       aSource ) {}
    CurrentMessage() : Message( currentMessage ) {}

    /// standard destructor
    virtual ~CurrentMessage() {}

    /// Accessor for current, adding to input. 
    /**
     * This method adds the message to the input current
     * @param aCurrent subtotal of current flow
     */
    void addCurrent( double& aCurrent )
    {
      aCurrent += iCurrent;
    }
    
    /// Accessor for current
    /**
     * @return current in message
     */
    double getCurrent()
    {
      return iCurrent;
    }
    
    /// Mutator for current 
    /**
     * This method sets the current for the message
     * @param aCurrent new current flow
     * @return reference to self
     */
    Message& newCurrent( double aCurrent )
    {
      iCurrent=aCurrent;
      return dynamic_cast< Message& >( *this );
    }
    
  protected:
    /// @var Current transmitted
    double iCurrent;
  };
  
  /// Class for analogue input messages transmitted by nodes
  /**
   * these are used by sensors to synapses
   */
  class AnalogueMessage : public Message
  {
  public:
    /// default constructor for AnalogueMessage
    AnalogueMessage( double anAnalogue ) : Message( analogueMessage ),
      iAnalogue( anAnalogue ) {}
    AnalogueMessage( std::string aSource, double anAnalogue ) : 
      Message( analogueMessage, aSource ),
      iAnalogue( anAnalogue ) {}

    AnalogueMessage( int anAnalogue ) : Message( analogueMessage ),
      iAnalogue( static_cast< double >( anAnalogue ) ) {}
    AnalogueMessage( std::string aSource, int anAnalogue ) :
      Message( analogueMessage, aSource ),
      iAnalogue( static_cast< double >( anAnalogue ) ) {}
    AnalogueMessage( std::string aSource ) :
      Message( analogueMessage, aSource ) {}
    
    /// default empty constructor for AnalogueMessage
    AnalogueMessage() : Message( analogueMessage ) {}
    
    /// standard destructor
    virtual ~AnalogueMessage() {}
    
    /// Accessor for analogue, adding to input. 
    /**
     * This method adds the message to the input analogue
     * @param anAnalogue subtotal of analogue flow
     */
    void addAnalogue( double& anAnalogue )
    {
      anAnalogue += iAnalogue;
    }
    
    /// Accessor for analogue
    /**
     * @return analogue in message
     */
    double getAnalogue()
    {
      return iAnalogue;
    }
    
    /// Mutator for analogue 
    /**
     * This method sets the analogue for the message
     * @param anAnalogue new analogue flow
     * @return reference to self
     */
    Message& newAnalogue( double anAnalogue )
    {
      iAnalogue = anAnalogue;
      return dynamic_cast< Message& >( *this );
    }
    
    Message& newAnalogue( int anAnalogue )
    {
      iAnalogue = static_cast< double >( anAnalogue );
      return dynamic_cast< Message& >( *this );
    }
    
  protected:
    /// @var Analogue transmitted
    double iAnalogue;
  };
  
  /// Class for conductance change messages transmitted by nodes
  /**
   * these are used by synapses to communicate to neurons
   */
  class ConductanceMessage : public Message
  {
  public:

    /// default constructor for ConductanceMessage
    ConductanceMessage( double aConductance, double aCurrent ) :
      Message( conductanceMessage ), 
      iG( aConductance ), iGV( aCurrent ) {}

    /// default empty constructor for ConductanceMessage
    ConductanceMessage( std::string aSource ) :
      Message( conductanceMessage, aSource ) {}
    ConductanceMessage() : Message( conductanceMessage ) {}

    /// standard destructor
    virtual ~ConductanceMessage() {}
    
    /// Accessor for conductance, adding to inputs. 
    /**
     * This method adds the message to the input conductance and current
     * @param aG subtotal of conductance
     * @param aGV subtotal of current flow
     */
    void addConductance( double& aG, double& aGV )
    {
      aG+=iG;
      aGV+=iGV;
    }

    /// Mutator for conductance
    /**
     * This method sets the conductance for the message
     * @param aG new conductance
     * @param aGV new current flow
     * @return reference to self
     */
    Message& newConductance( double aG, double aGV )
    {
      iG=aG;
      iGV=aGV;
      return dynamic_cast< Message& >( *this );
    }

  protected:
    /**
     * @var Conductance to add onto total membrane conductance
     * and current flow to add onto total current flow.
     */
    double iG, iGV;
  };
  
  /// Class for new connection messages transmitted by nodes
  /**
   * these are used by nodes to request connections to other nodes
   */
  class NewConnectionMessage : public Message
  {
  public:
    /// default constructor for NewConnectionMessage
    NewConnectionMessage( std::string& aSource, std::string& aDestination,
			  unsigned int aDelay,
			  bool aFromSource = true ) :
      Message( newConnectionMessage, aSource ),
      iDestination( aDestination ), iDelay( aDelay ),
      bFromSource( aFromSource ) {}
    
    /// standard destructor
    virtual ~NewConnectionMessage() {}

    /// Accessor for destination
    /**
     * @return destination in message
     */
    std::string& getDestination()
    {
      return iDestination;
    }
    
    /// Accessor for direction of travel
    /**
     * @return is message travelling from source?
     */
    bool fromSource()
    {
      return bFromSource;
    }
    
    /// Accessor for delay
    unsigned int getDelay()
    {
      return iDelay;
    }

    /// Mutator for destination 
    /**
     * This method sets the destination for the connection
     * @param aDestination new destination
     */
    void newDestination( std::string& aDestination )
    {
      iDestination = aDestination;
    }
    
  protected:
    /// @var Name of connection destination
    std::string iDestination;
    /// @var delay in timesteps for message transmission
    unsigned int iDelay;
    /// @var Direction of travel of message
    bool bFromSource;
  };
  
  /// Class for remove connection messages transmitted by nodes
  /**
   * these are used by nodes to request connection removals from other nodes
   */
  class RemoveConnectionMessage : public Message
  {
  public:
    /// default constructor for RemoveConnectionMessage
    RemoveConnectionMessage( std::string aSource, std::string aDestination,
			     bool aFromSource = true ) :
      Message( removeConnectionMessage, aSource ),
      iDestination( aDestination ), bFromSource( aFromSource ) {}
    
    /// standard destructor
    virtual ~RemoveConnectionMessage() {}

    /// Accessor for destination
    /**
     * @return destination in message
     */
    std::string getDestination()
    {
      return iDestination;
    }
    
    /// Accessor for direction of travel of message
    /**
     * @return whether message comes from source
     */
    bool fromSource()
    {
      return bFromSource;
    }

    /// Mutator for destination 
    /**
     * This method sets the destination for the connection
     * @param aDestination new destination
     */
    void newDestination( std::string& aDestination )
    {
      iDestination = aDestination;
    }
    
  protected:
    /// @var Name of connection destination
    std::string iDestination;
    /// @var Has source been accepted
    bool bFromSource;
  };
  
  /// Class for new robot connection messages
  /**
   * these are used by nodes to request connections to the robot
   */
//   class NewRobotConnMessage : public Message
//   {
//   public:
//     /// default constructor for NewConnectionMessage
//     NewRobotConnMessage( std::string& aSource, int anID, bool isBytes,
// 			 int aNum, bool aFromSource = true ) :
//       Message( newRobotConnMessage, aSource ),
//       iDestination( aDestination ), iDelay( aDelay ),
//       bFromSource( aFromSource ) {}
    
//     /// standard destructor
//     virtual ~NewRobotConnMessage() {}

//     /// Accessor for destination
//     /**
//      * @return destination in message
//      */
//     std::string& getDestination()
//     {
//       return iDestination;
//     }
    
//     /// Accessor for direction of travel
//     /**
//      * @return is message travelling from source?
//      */
//     bool fromSource()
//     {
//       return bFromSource;
//     }
    
//     /// Accessor for delay
//     unsigned int getDelay()
//     {
//       return iDelay;
//     }

//     /// Mutator for destination 
//     /**
//      * This method sets the destination for the connection
//      * @param aDestination new destination
//      */
//     void newDestination( std::string& aDestination )
//     {
//       iDestination = aDestination;
//     }
    
//   protected:
//     /// @var Name of connection destination
//     std::string iDestination;
//     /// @var delay in timesteps for message transmission
//     unsigned int iDelay;
//     /// @var Direction of travel of message
//     bool bFromSource;
//   };
  
//   /// Class for remove robot connection messages
//   /**
//    * these are used by nodes to request connection removals from robots
//    */
//   class RemoveRobotConnMessage : public Message
//   {
//   public:
//     /// default constructor for RemoveConnectionMessage
//     RemoveRobotConnMessage( std::string aSource, std::string aDestination,
// 			    bool aFromSource = true ) :
//       Message( removeRobotConnMessage, aSource ),
//       iDestination( aDestination ), bFromSource( aFromSource ) {}
    
//     /// standard destructor
//     virtual ~RemoveRobotConnMessage() {}
    
//     /// Accessor for destination
//     /**
//      * @return destination in message
//      */
//     std::string getDestination()
//     {
//       return iDestination;
//     }
    
//     /// Accessor for direction of travel of message
//     /**
//      * @return whether message comes from source
//      */
//     bool fromSource()
//     {
//       return bFromSource;
//     }

//     /// Mutator for destination 
//     /**
//      * This method sets the destination for the connection
//      * @param aDestination new destination
//      */
//     void newDestination( std::string& aDestination )
//     {
//       iDestination = aDestination;
//     }
    
//   protected:
//     /// @var Name of connection destination
//     std::string iDestination;
//     /// @var Has source been accepted
//     bool bFromSource;
//   };
  
}

#endif //ndef __MESSAGE_HH
