// $Id: RobotIO.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <list>
#include <string>
#include <iostream>

#include "Params.hh"
#include "Message.hh"
#pragma implementation
#include "RobotIO.hh"
#include "low_interface.h"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void robotio_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "Robot", "enum" );
      }
    catch ( bad_param& bp )
      {
	std::cerr << bp.what() << " already defined - ignoring..."
		  << std::endl;
	already=true;
      }
    
    if (!already)
      {
	try
	  {
	    aParams.addParam( "koala", "enum" );
	    aParams.addParam( "khepera", "enum" );
	    aParams.addParam( "motorSpeedScale", "s/m" );
	    aParams.addParam( "motorPosScale", "/m" );
	    aParams.addParam( "koalaMotorSpeedScale", "s/m" );
	    aParams.addParam( "koalaMotorPosScale", "/m" );
	    aParams.addParam( "kheperaMotorSpeedScale", "s/m" );
	    aParams.addParam( "kheperaMotorPosScale", "/m" );
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
  }
}

/// Called on end of experiment for cleanup
/**
 * Stop sensor communication
 */
void RobotIO::cleanup()
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
		      if (steps < val)
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
      bActive = false;
    }
}

/// remove all external links
/**
 * Kills branches, then kills all remote links - an essential precursor
 * to calling delete
 */
bool RobotIO::removeAll( std::string aName ) throw ( bad_node )
{
  bool deleteMe = false;
  if ((aName == "") || (matchName( aName, false )))
    {
      for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	{
	  while (iInputNumConns[i] != 0)
	    {
	      RemoveConnectionMessage rMess( iName + "." + 
					     iInputs[i].getName(),
					     iTargets[i].front(), true );
	      iParent->doReceive( rMess );
	      removeConnection( iName + "." + iInputs[i].getName(),
				iTargets[i].front());
	    }
	  iInputs[i].destroy();
	}
      for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	{
	  while (iOutputNumConns[i] != 0)
	    {
	      RemoveConnectionMessage rMess( iSources[i].front(),
					     iName + "." +
					     iOutputs[i].getName(),
					     false );
	      iParent->doReceive( rMess );
	      removeConnection( iSources[i].front(),
				iName + "." + iOutputs[i].getName() );
	    }
	  iOutputs[i].destroy();
	}
      deleteMe = true;
    }
  else if (!matchName( aName, true ))
    throw bad_node( aName );
  else
    {
      bool found = false;
      std::string strippedName = stripName( aName );
      for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	{
	  if (iInputs[i].getName() == strippedName)
	    {
	      while (iInputNumConns[i] != 0)
		{
		  RemoveConnectionMessage rMess( iName + "." +
						 iInputs[i].getName(),
						 iTargets[i].front(),
						 true );
		  iParent->doReceive( rMess );
		  removeConnection( iName + "." + iInputs[i].getName(),
				    iTargets[i].front());
		}
	      iInputs[i].destroy();
	      found = true;
	    }
	}
      for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	{
	  if (iOutputs[i].getName() == strippedName)
	    {
	      while (iOutputNumConns[i] != 0)
		{
		  RemoveConnectionMessage rMess( iSources[i].front(),
						 iName + "." +
						 iOutputs[i].getName(),
						 false );
		  iParent->doReceive( rMess );
		  removeConnection( iSources[i].front(),
				    iName + "." + iOutputs[i].getName() );
		}
	      iOutputs[i].destroy();
	      found = true;
	    }
	}
      if (!found)
	throw bad_node( aName );
    }
  return deleteMe;
}
  
/// Get node by name from router
/**
 * @param aName name of node
 * @return pointer to node
 * @exception bad_node thrown if name not used
 */
Node* RobotIO::getNode( std::string aName ) throw ( bad_node )
{
  if (!matchName( aName, true ))
    throw bad_node( aName );
  std::string aChild = stripName( aName );
  for ( unsigned int anEntry = 0; anEntry < PERIPHERALS_MAX; anEntry++ )
    {
      if ((!iInputs[anEntry].isDeleted() &&
	   (iInputs[anEntry].getName() == aChild)) ||
	  (!iOutputs[anEntry].isDeleted() &&
	   (iOutputs[anEntry].getName() == aChild)))
	return this;
    }
  throw bad_node( aName );
  return NULL;
}

/// Receives and processes message
/**
 * @param aMessage
 * @exception bad_message thrown if inappropriate message is sent
 */
void RobotIO::doReceive( Message& aMessage )
{
  NewConnectionMessage* nc;
  RemoveConnectionMessage* rc;
  unsigned int theLocal;
  std::string aStr;
  std::list< int >* aListPtr;
  int aVal;

  switch (aMessage.getType())
    {
    case noMessage:
      break;
    case spikeMessage:
      aListPtr = &iRecipients[aMessage.getSource()];
      for ( std::list< int >::iterator anIter = aListPtr->begin();
	    anIter != aListPtr->end(); anIter++ )
	iOutputs[*anIter].updateOutput();
      break;
    case analogueMessage:
      aListPtr = &iRecipients[aMessage.getSource()];
      aVal =
	static_cast< int >( dynamic_cast< AnalogueMessage* >
			    ( &aMessage )->getAnalogue() );
      for ( std::list< int >::iterator anIter = aListPtr->begin();
	    anIter != aListPtr->end(); anIter++ )
	iOutputs[*anIter].updateOutput( aVal );
      break;
    case newConnectionMessage:
      nc = dynamic_cast< NewConnectionMessage* >( &aMessage );
#ifdef DEBUG
      std::cout << "New connection from " << nc->getSource()
	   << " to " << nc->getDestination() << " received by "
	   << getName( true ) << std::endl;
#endif //def DEBUG
      if (nc->fromSource())
	{
	  if (matchName( nc->getDestination() ))
	    {
	      std::cerr << "Connection to RobotIO itself not allowed" << std::endl;
	      throw bad_message( aMessage.getType() );
	    }
	  
	  if (!matchName( nc->getDestination(), true ))
	    {
	      throw bad_message( aMessage.getType() );
	    }

	  theLocal = PERIPHERALS_MAX;
	  aStr = stripName( nc->getDestination() );
	  for (unsigned int i = 0; i < PERIPHERALS_MAX; i++)
	    {
	      if ((!iOutputs[i].isDeleted()) &&
		  (iOutputs[i].getName() == aStr) && 
		  (iOutputs[i].getType() != noMessage))
		{
		  theLocal = i;
		}
	    }
	  if (theLocal == PERIPHERALS_MAX)
	    throw bad_node( nc->getDestination() );

	  iSources[theLocal].push_back(nc->getSource());
	  iOutputNumConns[theLocal]++;
	  iRecipients[aMessage.getSource()].push_back( theLocal );
	}
      else // !nc->fromSource()
	{
	  if (matchName( nc->getSource() ))
	    {
	      std::cerr << "Connection to RobotIO itself not allowed" << std::endl;
	      throw bad_message( aMessage.getType() );
	    }
	  
	  if (!matchName( nc->getSource(), true ))
	    {
	      throw bad_message( aMessage.getType() );
	    }
	  theLocal = PERIPHERALS_MAX;
	  aStr = stripName( nc->getSource() );
	  for (unsigned int i = 0; i < PERIPHERALS_MAX; i++)
	    {
	      if ((!iInputs[i].isDeleted()) &&
		  (iInputs[i].getName() == aStr) && 
		  (iInputs[i].getType() != noMessage))
		{
		  theLocal = i;
		}
	    }
	  if (theLocal == PERIPHERALS_MAX)
	    throw bad_node( nc->getSource() );
	  iTargets[theLocal].push_back(nc->getDestination());
	  if (++iInputNumConns[theLocal] == 1)
	    {
	      switch ( iInputs[theLocal].getType() )
		{
		case spikeMessage:
		  iMessages[ theLocal ] 
		    = new SpikeMessage( nc->getSource() );
		  break;
		case analogueMessage:
		  iMessages[ theLocal ] 
		    = new AnalogueMessage( nc->getSource() );
		  break;
		default:
		  throw bad_message( iInputs[theLocal].getType() );
		}
	    }
	}
      break;
    case removeConnectionMessage:
      rc = dynamic_cast< RemoveConnectionMessage* >( &aMessage );
#ifdef DEBUG
      std::cout << "Remove connection from " << rc->getSource()
	   << " to " << rc->getDestination() << " received by "
	   << getName( true ) << std::endl;
#endif //def DEBUG
      removeConnection( rc->getSource(), rc->getDestination() );
      break;	
    default:
      throw bad_message( aMessage.getType() );
      break;
    }
}


/// Initialise parameter names for this node type
/**
 * @param aRoot the node whose parameters are to be initialised
 * @exception bad_param thrown if the parameter already exists
 */
void RobotIO::initParams() throw ( bad_param, bad_file )
{
  robotio_init( gRoot );
}

/// Removes an existing connection controlled by the robotio
/**
 * @param aSource the branch the message came from
 * @param aDestination the node which the message went to
 * @exception bad_node if the connection or the recipient
 * does not exist
 */
void RobotIO::removeConnection( std::string aSource, std::string aDestination )
  throw ( bad_node )
{
  if ((matchName( aSource )) || (matchName( aDestination )))
    {
      std::cerr << "Connection to RobotIO itself not allowed" << std::endl;
      throw bad_node( aSource );
    }
  
  std::string aStr;
  unsigned int theLocal = PERIPHERALS_MAX;
  bool found = false;

  if (matchName( aSource, true ))
    {
      aStr = stripName( aSource );
      for (unsigned int i = 0; i < PERIPHERALS_MAX; i++)
	{
	  if ((!iInputs[i].isDeleted()) &&
	      (iInputs[i].getName() == aStr) && 
	      (iInputs[i].getType() != noMessage))
	    {
	      theLocal = i;
	    }
	}
      
      if (theLocal == PERIPHERALS_MAX)
	throw bad_node( aSource );
      
      if (iInputNumConns[theLocal] == 0)
	throw bad_node( aSource );
      
      for ( std::list< std::string >::iterator anIter = iTargets[theLocal].begin();
	    anIter != iTargets[theLocal].end(); anIter++ )
	{
	  if ((*anIter) == aDestination)
	    {
	      std::list< std::string >::iterator a = anIter;
	      anIter--;
	      iTargets[theLocal].erase( a );
	      found = true;
	    }
	}
      
      if (!found)
	throw bad_node( aDestination );
      
      if (--iInputNumConns[theLocal] == 0)
	{
	  delete iMessages[theLocal];
	  iTargets.erase( iTargets.find( theLocal ) );
	}
    }
  else if (matchName( aDestination, true ))
    {
      aStr = stripName( aDestination );
      for (unsigned int i = 0; i < PERIPHERALS_MAX; i++)
	{
	  if ((!iOutputs[i].isDeleted()) &&
	      (iOutputs[i].getName() == aStr))
	    //	    && (iOutputs[i].getType() != noMessage))
	    {
	      theLocal = i;
	    }
	}
      
      if (theLocal == PERIPHERALS_MAX)
	throw bad_node( aDestination );
      if (iOutputNumConns[theLocal] == 0)
	throw bad_node( aDestination );
      
      for ( std::list< std::string >::iterator
	      anIter = iSources[theLocal].begin();
	    anIter != iSources[theLocal].end(); anIter++ )
	{
	  if ((*anIter) == aSource)
	    {
	      std::list< std::string >::iterator a = anIter;
	      anIter--;
	      iSources[theLocal].erase( a );
	      found = true;
	    }
	}

      iRecipients.erase( iRecipients.find( aSource ) );

      if (!found)
	throw bad_node( aDestination );

      if (--iOutputNumConns[theLocal] == 0)
	{
	  iSources.erase( iSources.find( theLocal ) );
	}
    }

  if (!found)
    throw bad_node( aDestination );
}

/// Do one timestep for self only
bool RobotIO::nodeTimestep()
{
  char* bitRef = iInputArray;
  char* nextRef = iInputArray + 1;
  int mask = 2;
  bool doOutput = false;
  int val;
  int steps = -1;
  int i;
  long ana;

  if (!bAlive)
    throw bad_node( iName );
  
  if (!bActive)
    {
      if (Low_interface_type == INTERFACE_FIFO)
	{
	  low_fifo_period( static_cast< LONG >( iParams.getParam( "stepT" ) *
						US_PER_MS ) );
	  low_fifo_start();
	}
      else
	{
	  low_send_uchar( (UCHAR) LOW_PREPARE_OLD );
	  doOutput = true;
	}
      bActive = true;
    }

  low_sensors_acquire( iInputArray );

  for ( i = 0; i < PERIPHERALS_MAX; i++ )
    {
      if (!iInputs[i].isDeleted())
	{
	  if (iInputs[i].getType() == spikeMessage)
	    {
	      if (iInputs[i].retrieveBit( bitRef, mask, nextRef ))
		{
		  if (iInputNumConns[i] != 0)
		    iParent->doReceive( *iMessages[i] );
		}
	    }
	  else // analogue
	    {
	      if (iInputs[i].retrieveBytes( bitRef, mask, nextRef, ana ))
		if (iInputNumConns[i] != 0)
		  iParent->doReceive( dynamic_cast< AnalogueMessage* >
				      ( iMessages[i] )->
				      newAnalogue( static_cast<double>(ana)) );
	    }
	}
    }

  bitRef = NULL;
  nextRef = iOutputArray;
  mask = 1;

  for ( i = 0; i < PERIPHERALS_MAX; i++)
    {
      if (!iOutputs[i].isDeleted())
	doOutput |= iOutputs[i].output( bitRef, mask, nextRef );
    }

  if (doOutput)
    {
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
	  if (steps == -1) // no motors and normal serial is not allowed
	    throw bad_peripheral( iType );

	  low_motors_update_old( static_cast< int >( nextRef - iOutputArray ),
				 iOutputArray, steps );
	}
      else // fifo
	low_motors_update_fifo( static_cast< int >( nextRef - iOutputArray ),
				iOutputArray );
    }
      
  return true;
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void RobotIO::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a RobotIO called " << iName << std::endl;
#endif
  //  low_motors_stop();
}

/// Output initialisation script for robotio
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& RobotIO::externalise( std::ostream& anOStream, 
				    std::string indent )
{
#ifdef DEBUG
  anOStream << "// Configuring a robot called " << iName;
#endif
  anOStream << "Robot ";
  switch (iType)
    {
    case ROBOT_KOALA:
      anOStream << "koala ";
      break;
    case ROBOT_OLD_KOALA:
      anOStream << "old_koala ";
      break;
    case ROBOT_KHEPERA:
      anOStream << "khepera ";
      break;
    default:
      anOStream << "<unknown> ";
      break;
    }

  anOStream << iName << "()" << std::endl << indent << "{";
  indent.push_back( ' ' );

  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
    {
      if (!iInputs[i].isDeleted())
	iInputs[i].externalise( anOStream << std::endl << indent, indent );
    }
  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
    {
      if (!iOutputs[i].isDeleted())
	iOutputs[i].externalise( anOStream << std::endl << indent, indent );
    }

  indent.erase( indent.size()-1 );
  anOStream << std::endl << indent << "}";

  return anOStream;
}

/// Output variable names for robotio
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& RobotIO::write( std::ostream& anOStream,
			      OutputType aType )
{
  std::string totalName = getRoot()->getName() + '.' + getName( true );

      if (gOutputLevel.test( outputInput ))
	{
	  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	    {
	      if (!iInputs[i].isDeleted())
		iInputs[i].write( anOStream, totalName, aType );
	    }
	}
      if (gOutputLevel.test( outputMotor ))
	{
	  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	    {
	      if (!iOutputs[i].isDeleted())
		iOutputs[i].write( anOStream, totalName, aType );
	    }
	}
	  
  return anOStream;
}

/// Output current state of node and children to array
/**
 * This is used by all the other put methods to gather data
 * @param anArray array of doubles to store data in
 * @param average true to get average since last reading, else current
 * @return pointer to end of used part of array
 */
double* RobotIO::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

      if (gOutputLevel.test( outputInput ))
	{
	  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	    {
	      if (!iInputs[i].isDeleted())
		thePtrPos = iInputs[i].put( thePtrPos, average );
	    }
	}
      if (gOutputLevel.test( outputMotor ))
	{
	  for ( unsigned int i = 0; i < PERIPHERALS_MAX; i++ )
	    {
	      if (!iOutputs[i].isDeleted())
		thePtrPos = iOutputs[i].put( thePtrPos, average );
	    }
	}

  return thePtrPos;
}
