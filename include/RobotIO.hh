// Basic type of robotio
// $Id: RobotIO.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __ROBOTIO_HH
#define __ROBOTIO_HH

#include <string>
#include <vector>
#include <map>
#include <list>
#include <iostream>

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"
#include "Peripheral.hh"
#include "peripheral.h"
#include "interface.h"
#include "low_interface.h"

extern InterfaceType Default_sensor_conn;

#pragma interface

namespace Runtime
{
  /// Robot router which contains robot sensors and motors
  class RobotIO : public Node
  {
  public:
    /// Constructor robotio attached to current context
    /**
     * @param aName unique name for robotio
     * @param aFilename name of file to input data from
     * @exception bad_node thrown if name already used
     */
    RobotIO( PeripheralType aType, std::string aName )
      throw ( bad_node, bad_message, bad_param, bad_peripheral ) :
      Node( aName ), iType( aType ), bAlive( false ), bActive( false )
    {
      switch (iType)
	{
	case ROBOT_KOALA:
	  Connection = CON_KOALA;
	  ConnectionSpeed = CON_KOALA;
	  iParams.setParam( "motorSpeedScale", 
			    iParams.getParam( "koalaMotorSpeedScale" ) );
	  iParams.setParam( "motorPosScale", 
			    iParams.getParam( "koalaMotorPosScale" ) );
	  break;
	case ROBOT_OLD_KOALA:
	  Connection = CON_KOALA;
	  ConnectionSpeed = CON_KHEP;
	  iParams.setParam( "motorSpeedScale", 
			    iParams.getParam( "koalaMotorSpeedScale" ) );
	  iParams.setParam( "motorPosScale", 
			    iParams.getParam( "koalaMotorPosScale" ) );
	  break;
	case ROBOT_KHEPERA:
	  Connection = CON_KHEP;
	  ConnectionSpeed = CON_KHEP;
	  iParams.setParam( "motorSpeedScale", 
			    iParams.getParam( "kheperaMotorSpeedScale" ) );
	  iParams.setParam( "motorPosScale", 
			    iParams.getParam( "kheperaMotorPosScale" ) );
	  break;
	default:
	  throw bad_peripheral( iType );
	}

      if (call_start( LEVEL_SENSOR, Default_sensor_conn ))
	bAlive = true;
      else
	throw bad_peripheral( iType );

      iInputs.resize( PERIPHERALS_MAX );
      iOutputs.resize( PERIPHERALS_MAX );
      iInputNumConns.resize( PERIPHERALS_MAX );
      iOutputNumConns.resize( PERIPHERALS_MAX );
      iMessages.resize( PERIPHERALS_MAX );

      initialise();
    }

    /// Constructor for sub-robotio
    /**
     * @param aName unique name for robotio
     * @param aParentName name of parent router
     * @param aFilename name of file to input data from
     * @exception bad_node thrown if no such parent, or name used
     */
    RobotIO( PeripheralType aType, std::string aName, std::string aParentName ) 
      throw ( bad_node, bad_message, bad_param ) 
      : Node( aName, aParentName ), iType( aType ), bAlive( false ),
      bActive( false )
    {
      switch (iType)
	{
	case ROBOT_KOALA:
	  Connection = CON_KOALA;
	  ConnectionSpeed = CON_KOALA;
	  break;
	case ROBOT_OLD_KOALA:
	  Connection = CON_KOALA;
	  ConnectionSpeed = CON_KHEP;
	  break;
	case ROBOT_KHEPERA:
	  Connection = CON_KHEP;
	  ConnectionSpeed = CON_KHEP;
	  break;
	default:
	  throw bad_peripheral( iType );
	}

      if (call_start( LEVEL_SENSOR, Default_sensor_conn ))
	bAlive = true;
      else
	throw bad_peripheral( iType );

      iInputs.resize( PERIPHERALS_MAX );
      iOutputs.resize( PERIPHERALS_MAX );
      iInputNumConns.resize( PERIPHERALS_MAX );
      iOutputNumConns.resize( PERIPHERALS_MAX );
      iMessages.resize( PERIPHERALS_MAX );

      initialise();
    }

    /// Static constructor for robotio
    /**
     * @param aName unique name for robotio
     * @exception bad_node thrown if name used
     */
    static void newRobotIO( PeripheralType aType, std::string aName )
      throw ( bad_node, bad_message, bad_param )
    {
      new RobotIO( aType, aName );
    }

    /// Destructor for robotio
    ~RobotIO()
    {
      int i;
      int val;
      int steps = -1;
      char sensors[PERIPHERALS_MAX];

      if (bAlive)
	{
	  if (bActive)
	    {
	      low_motors_stop();
	      if (Low_interface_type != INTERFACE_FIFO)
		{
		  for ( i = 0; i < PERIPHERALS_MAX; i++ )
		    {
		      if (!iOutputs[i].isDeleted())
			{
			  val = iOutputs[i].getNextOutput();
			  if (steps > val)
			    steps = val;
			}
		    }
		  if (steps != -1)
		    {
		      for ( i = 0; i <= steps; i++ )
			low_sensors_acquire( sensors );
		    }
		}
	      else
		low_fifo_stop();
	    }
	  low_call_quit();
	}

      if (call_close( LEVEL_SENSOR ))
	bAlive = false;
      else
	throw bad_peripheral( iType );
    }

    /// Called on end of experiment for cleanup
    /**
     * Stop sensor communication
     */
    virtual void cleanup();

    /// remove all external links
    /**
     * Kills branches, then kills all remote links - an essential precursor
     * to calling delete
     * @param aName name of node to be stripped - usually self
     * @return whether node can be deleted
     */
    virtual bool removeAll( std::string aName = "" ) throw ( bad_node );

    /// Get node by name from router
    /**
     * @param aName name of node
     * @return pointer to node
     * @exception bad_node thrown if name not used
     */
    virtual Node* getNode( std::string aName ) throw ( bad_node );

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
	case newConnectionMessage:
	case removeConnectionMessage:
	case spikeMessage:
	case analogueMessage:
	  return true;
	  break;
	default:
	  return false;
	}
    }

    /// Receives and processes message
    /**
     * @param aMessage
     * @exception bad_message thrown if inappropriate message is sent
     */
    virtual void doReceive( Message& aMessage );
    
    /// Initialise parameter names for this node type
    /**
     * @exception bad_param thrown if the parameter already exists
     */
    static void initParams() throw ( bad_param, bad_file );

    /// Initialises and configures the node
    /**
     * This is called by constructors, but can be called at any point
     * (if parameters have been changed, for instance).
     */
    virtual void initialise();

    /// Add a new peripheral to a robot
    /**
     * @param aName name of Peripheral
     * @param anInput is the peripheral an input or an output
     * @param aType the peripheral type
     * @param aVA arguments used to create peripheral
     * @exception bad_peripheral thrown if reference doesn't match
     */
    void add_peripheral( std::string aName, bool anInput, PeripheralType aType,
			 VarArg aVA ) throw (bad_peripheral)
    {
      int aRef = 0;
      if (anInput)
	{
	  while ((aRef < PERIPHERALS_MAX) && (!iInputs[aRef].isDeleted()))
	    aRef++;

	  iInputs[aRef].renew( aName, anInput, aType, aVA );
	  if (iInputs[aRef].getRef() != aRef)
	    {
	      std::cerr << "wrong reference! " << iInputs[aRef].getRef() 
			<< std::endl;
	      throw bad_peripheral( aName );
	    }
	}
      else
	{
	  while ((aRef < PERIPHERALS_MAX) && (!iOutputs[aRef].isDeleted()))
	    aRef++;
	  iOutputs[aRef].renew( aName, anInput, aType, aVA );
	  if (iOutputs[aRef].getRef() != aRef)
	    throw bad_peripheral( aName );
	}

    }

    /// Removes an existing connection controlled by the robotio
    /**
     * @param aSource the branch the message came from
     * @param aDestination the node which the message went to
     * @exception bad_node if the connection or the recipient
     * does not exist
     */
    void removeConnection( std::string aSource, std::string aDestination )
      throw ( bad_node );

    /// Do one timestep for self only
    virtual bool nodeTimestep();

    /// Output initialisation script for robotio
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for robotio
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& write( std::ostream& anOStream,
				 OutputType aType = outTypeText );

    /// Output current state of node and children to array
    /**
     * This is used by all the other put methods to gather data
     * @param anArray array of doubles to store data in
     * @param average true to get average since last reading, else current
     * @return pointer to end of used part of array
     */
    virtual double* put( double* anArray, bool average = true );

  protected:
    /// @var type of robot
    PeripheralType iType;

    /// @var Is the software downloaded, and the robot running?
    bool bAlive;

    /// @var is the connection active?
    bool bActive;

    /// @var Number of bytes coming from robot
    unsigned int iNumInputBytes;

    /// @var vector of input Peripherals
    std::vector< Peripheral > iInputs;

    /// @var vector of number of connections for each input
    std::vector< int > iInputNumConns;
    
    /// @var Map of names of targets of messages
    std::map< int, std::list< std::string > > iTargets;
    
    /// @var Map of columns with messages to be sent to parent
    std::vector< Message* > iMessages;
    
    /// @var array of current inputs
    char iInputArray[PERIPHERALS_MAX];

    /// @var vector of output Peripherals
    std::vector< Peripheral > iOutputs;

    /// @var vector of number of connections for each output
    std::vector< int > iOutputNumConns;
    
    /// @var Map of names of sources of messages
    std::map< int, std::list< std::string > > iSources;
    
    /// @var Map of names of recipients of messages
    std::map< std::string, std::list< int > > iRecipients;
    
    /// @var vector of current outputs
    char iOutputArray[PERIPHERALS_MAX];

  private:
    /// Constructor for normal robotio not allowed, so private
    /**
     * @param aName unique name for robotio
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent, or must be root...
     */
    RobotIO( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message ) : 
      Node( aName, aParent, noMessage )
    {
      throw bad_node( iName );
    }
  };

}

#endif //ndef __ROBOTIO_HH
