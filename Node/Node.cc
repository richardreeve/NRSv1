// $Id: Node.cc,v 1.7 2003/10/08 19:52:54 darrens Exp $

#include <iostream>
#include <stack>
#include <string>
#include <bitset>
#include <map>
#include <iterator>
#include <cstdio>

#include "Params.hh"
#include "Exception.hh"
#pragma implementation
#include "Node.hh"

using namespace Runtime;

std::ostream& operator<<( std::ostream& anOStream, Runtime::Node& anOutput )
{
  return anOutput.put( anOStream );
}

std::istream& operator>>( std::istream& anIStream, Runtime::Node& anInput )
{
  return anInput.get( anIStream );
}

namespace 
{
  //  This function does the initialisation
  void node_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "node", "enum" );
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
	    aParams.addParam( "stepT", "ms" );
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
  }
}

Node* Node::gRoot = NULL;
std::stack< Node* > Node::gContext = std::stack< Node* >();
std::bitset< MAX_TYPES > Node::gOutputLevel = 0;


/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void Node::initParams() throw ( bad_param, bad_file )
{
  node_init( gRoot );
}

/// remove all external links
/**
 * Kills branches, then kills all remote links - an essential precursor
 * to calling delete
 * @param aName name of node to be stripped - usually self
 * @return whether node can be deleted
 */
bool Node::removeAll( std::string aName ) throw ( bad_node )
{
  if ((aName != "") && (!matchName( aName, false )))
    throw bad_node( aName );
  
  std::list< Node* >::iterator vi = iBranches.begin();
  while (vi != iBranches.end())
    {
#ifdef DEBUG
      std::cerr << "Deleting node " << (*vi)->getName() << " from "
		<< iName << std::endl;
#endif
      if ((*vi)->removeAll())
	delete *vi;
      vi=iBranches.begin();
    }
  
  RemoveConnectionMessage* aMessage;
  for ( std::map< std::string, bool >::iterator anIter = iRemote.begin();
	anIter != iRemote.end(); anIter++ )
    {
      std::string aDest = anIter->first;
      if (anIter->second)
	aMessage = new RemoveConnectionMessage( iName, aDest, true );
      else
	aMessage = new RemoveConnectionMessage( aDest, iName,
						       false );
      iParent->doReceive( *aMessage );
      delete aMessage;
    }
  iRemote.clear();
  
  if (isRoot())
    {
      while (!gContext.empty())
	gContext.pop();
    }
  return true;
}

/// Get node by name from router
/**
 * @param aName name of node
 * @return pointer to node
 * @exception bad_node thrown if name not used
 */
Node* Node::getNode( std::string aName ) throw ( bad_node )
{
  if ((!matchName( aName, true )) && (!isRoot()))
    return iParent->getNode( aName );


  if (matchName( aName ))
    return this;
  
  std::string strippedName = stripName( aName );
  if ( iNodes.find( strippedName ) != iNodes.end() )
    {
      return iNodes[strippedName];
    }
  else
    {
      // Could it be below one of my branches?
      std::string::size_type anST = strippedName.find_first_of( '.' );
      if (anST != strippedName.npos)
	{
	  std::string aNewName = std::string( strippedName, 0, anST );
	  if (iNodes.find( aNewName ) != iNodes.end())
	    {
	      return iNodes[aNewName]->getNode( strippedName );
	    }
//  	    return iNodes[aNewName]->getNode( strippedName.erase( 0,
//  								  anST + 1 ) );
	}
    }
  // We have failed...
  throw bad_node("Node could not be found", aName, __FILE__, __LINE__ );
  
  return NULL;
}

/// Adds a remote connection between two named nodes
/**
 * This is a virtual method is intended to be overwritten by
 * Routers - as it stands it will throw an exception unless
 * one of the source and destination nodes are the current node
 * @param aSource the source of the connection
 * @param aDestination the destination of the connection
 * @param aDelay the delay in timesteps for the connection
 * @exception bad_node if either name cannot be found or
 * this node is not one of them.
 */
void Node::addConnection( std::string aSource, std::string aDestination,
			  unsigned int aDelay ) throw ( bad_node,
							bad_message )
{
  if (matchName( aSource ))
    addConnection( aDestination, true, aDelay );
  else if (matchName( aDestination ))
    addConnection( aSource, false, aDelay );
  else 
    throw bad_node("Node is neither source or destination", 
                   aSource, __FILE__, __LINE__ );
}

/// Adds a remote connection to node
/**
 * This is used for remote connections (via a router or similar).
 * Only one connection is allowed from any one node to another currently,
 * not forgetting that multiple connections between neurons are simple
 * via an equal number of synapses.
 * @param anOther other end of connection
 * @param aSource is this the source or the destination?
 * @param aDelay the delay in timesteps for the message
 * @exception bad_node thrown if connection already exists 
 * from this node, or other end does not exist
 */

void Node::addConnection( std::string& anOther, bool aSource,
			  unsigned int aDelay ) throw ( bad_node,
							bad_message )
{
  NewConnectionMessage* aMessage;
  
  if (isRoot())
    throw bad_node("Node is root", iName, __FILE__, __LINE__ );
    // throw bad_node( iName );
  
  std::string myName = getName( true );
  std::string myJustName = getName( false );
  std::string otherName = getNode( anOther )->getName( true );
  if ((anOther.size() > otherName.size()) ||
      (anOther != otherName.substr( otherName.size() - anOther.size(),
				    otherName.size() )))
    { // otherName can create one or more subnames, anOther is one of these
      std::string::size_type anOverlap = anOther.find( '.' );
      std::string::size_type theOverlap = 0;
      while (anOverlap != anOther.npos)
	{
	  if ((anOverlap <= otherName.size()) &&
	      (anOther.substr( 0, anOverlap ) 
	       == otherName.substr( otherName.size() - anOverlap,
				    //  otherName.size() ))
				    anOverlap )))
	    {
	      theOverlap = anOverlap;
	    }
	  anOverlap = anOther.find( '.', anOverlap + 1 );
	}
      if (theOverlap == 0)
        throw bad_node("", anOther, __FILE__, __LINE__ );
      //throw bad_node( anOther );
      otherName += anOther.substr( theOverlap, anOther.npos );
    }
  std::string::size_type anST = myName.find_first_of('.');
  while ((anST != myName.npos) &&
	 (myName.substr( 0, anST ) == otherName.substr( 0, anST )))
    {
      myName.erase( 0, anST+1 );
      otherName.erase( 0, anST+1 );
      anST = myName.find_first_of('.');
    }
  
  if (iRemote.find( otherName ) != iRemote.end())
    throw bad_node("", otherName, __FILE__, __LINE__ );
  //throw bad_node( otherName );
  iRemote[otherName] = aSource;
  
  if (aSource)
    aMessage = new NewConnectionMessage( myJustName,
					 anOther, aDelay,
					 true );
  else
    aMessage = new NewConnectionMessage( anOther,
					 myJustName, aDelay,
					 false );
  
  getRouter()->doReceive( *aMessage );
  
  delete aMessage;
}

/// Removes a remote connection between two named nodes
/**
 * This is a virtual method is intended to be overwritten by
 * Routers - as it stands it will throw an exception unless
 * one of the source and destination nodes are the current node
 * @param aSource the source of the connection
 * @param aDestination the destination of the connection
 * @param aDelay the delay in timesteps for the connection
 * @exception bad_node if either name cannot be found or
 * this node is not one of them.
 */
void Node::removeConnection( std::string aSource, std::string aDestination )
  throw ( bad_node, bad_message )
{
  if (matchName( aSource ))
    removeConnection( aDestination );
  else if (matchName( aDestination ))
    removeConnection( aSource );
  else
    throw bad_node("Node is neither source or destination", 
                   aSource, __FILE__, __LINE__ );
  // throw bad_node( aSource );
}

/// Removes the record of a remote connection from a node
/**
 * @param anOther other end of connection
 * @param first is this the first end of the connection to be told?
 * @exception bad_node thrown if connection or node at other end
 * does not already exist
 */
void Node::removeConnection( std::string anOther )
  throw ( bad_node, bad_message )
{
  RemoveConnectionMessage* aMessage;
  
  if (isRoot())
    throw bad_node("Node is root", iName, __FILE__, __LINE__ );
    //throw bad_node( iName );
  
  std::string myName = getName( true );
  std::string myJustName = getName( false );
  std::string otherName = getNode( anOther )->getName( true );
  if ((anOther.size() > otherName.size()) ||
      (anOther != otherName.substr( otherName.size() - anOther.size(),
				    otherName.size() )))
    { // otherName can create one or more subnames, anOther is one of these
      std::string::size_type anOverlap = anOther.find( '.' );
      std::string::size_type theOverlap = 0;
      while (anOverlap != anOther.npos)
	{
	  if (anOther.substr( 0, anOverlap ) 
	      == otherName.substr( otherName.size() - anOverlap,
				   otherName.size() ))
	    theOverlap = anOverlap;
	  anOverlap = anOther.find( '.', anOverlap + 1 );
	}
      if (theOverlap == 0)
	throw bad_node( anOther );
      otherName += anOther.substr( theOverlap, anOther.npos );
    }
  std::string::size_type anST = myName.find_first_of('.');
  while (myName.substr( 0, anST ) == otherName.substr( 0, anST ))
    {
      myName.erase( 0, anST );
      otherName.erase( 0, anST );
      anST = myName.find_first_of('.');
    }
  std::map< std::string, bool >::iterator anIter = iRemote.find( otherName );
  
  if (anIter == iRemote.end())
    throw bad_node( otherName );
  
  bool aSource = iRemote[otherName];
  iRemote.erase( anIter );
  
  if (aSource)
    aMessage = new RemoveConnectionMessage( myJustName, anOther, true );
  else
    aMessage = new RemoveConnectionMessage( anOther, myJustName, false );
  
  getRouter()->doReceive( *aMessage );
  
  delete aMessage;
}

/// Receives and processes message
/**
 * @param aMessage
 * @exception bad_message thrown if inappropriate message is sent
 */
void Node::doReceive( Message& aMessage )
{
  NewConnectionMessage* nc;
  RemoveConnectionMessage* rc;
  std::map< std::string, bool >::iterator anIter;
  std::string aStr;
  
  switch (aMessage.getType())
    {
    case noMessage:
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
	      if (iRemote.find( nc->getSource() ) != iRemote.end())
		throw bad_node( nc->getSource() );
	      iRemote[nc->getSource()] = false;
	    }
	  else if (matchName( nc->getDestination(), true))
	    {
	      if (matchName( nc->getSource(), true))
		{
//  		  std::cout << "At top level mediating connection from "
//  		       << nc->getSource() << " to " << nc->getDestination()
//  		       << std::endl;
		  addConnection( nc->getSource(), nc->getDestination(),
				 nc->getDelay() );
		}
//  	      std::cout << "Passing connection down from "
//  		   << nc->getSource() << " to " << nc->getDestination()
//  		   << std::endl;
	      aStr = stripName( nc->getDestination() );
	      iNodes.find( aStr.substr( 0, aStr.find_first_of( '.' ) ) )
		->second->doReceive( aMessage );
	    }
	  else if (matchName( nc->getSource(), true))
	    {
//  	      std::cout << "Passing connection up from "
//  		   << nc->getSource() << " to " << nc->getDestination()
//  		   << std::endl;
	      addConnection( nc->getSource(),
			     iParent->getName(), 0 );
	      if (bRouter)
		aStr = iName + '.' + stripName( aMessage.getSource() );
	      else
		aStr = stripName( aMessage.getSource() );
	      iParent->doReceive( aMessage.newSource( aStr ) );
	    }
	  else
	    { // not related to the source or destination
	      throw bad_message( aMessage.getType() );
	    }
	}
      else // (!nc->fromSource())
	{
	  if (matchName( nc->getSource() ))
	    {
	      if (iRemote.find( nc->getDestination() ) != iRemote.end())
		throw bad_node( nc->getDestination() );
	      iRemote[nc->getDestination()] = true;
	    }
	  else if (matchName( nc->getSource(), true))
	    {
	      if (matchName( nc->getDestination(), true))
		addConnection( nc->getSource(), nc->getDestination(),
			       nc->getDelay() );
	      else
		addConnection( stripName( nc->getSource() ),
			       iParent->getName(), 0 );
	      aStr = stripName( nc->getSource() );
	      iNodes.find( aStr.substr( 0, aStr.find_first_of( '.' ) ) )
		->second->doReceive( aMessage );
	    }
	  else if (matchName( nc->getDestination(), true))
	    {
	      if (bRouter)
		aStr = iName + '.' + stripName( nc->getDestination() );
	      else
		aStr = stripName( nc->getDestination() );
	      nc->newDestination( aStr );
	      iParent->doReceive( aMessage );
	    }
	  else
	    { // not related to the source or destination
	      throw bad_message( aMessage.getType() );
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
      if (rc->fromSource())
	{
	  if (matchName( rc->getDestination() ))
	    {
	      anIter = iRemote.find( rc->getSource() );
	      if (anIter == iRemote.end())
		throw bad_node( rc->getSource() );
	      iRemote.erase( anIter );
	    }
	  else if (matchName( rc->getDestination(), true))
	    {
	      if (matchName( rc->getSource(), true))
		{
		  removeConnection( rc->getSource(), rc->getDestination() );
		}
	      aStr = stripName( rc->getDestination() );
	      iNodes.find( aStr.substr( 0, aStr.find_first_of( '.' ) ) )
		->second->doReceive( aMessage );
	    }
	  else if (matchName( rc->getSource(), true))
	    {
	      removeConnection( stripName( rc->getSource() ), 
				iParent->getName() );
	      if (bRouter)
		aStr = iName + '.' + stripName( aMessage.getSource() );
	      else
		aStr = stripName( aMessage.getSource() );
	      iParent->doReceive( aMessage.newSource( aStr ) );
	    }
	  else
	    { // not related to the source or destination
	      throw bad_message( aMessage.getType() );
	    }
	}
      else // (!rc->fromSource())
	{
	  if (matchName( rc->getSource() ))
	    {
	      anIter = iRemote.find( rc->getDestination() );
	      if (anIter == iRemote.end())
		throw bad_node( rc->getDestination() );
	      iRemote.erase( anIter );
	    }
	  else if (matchName( rc->getSource(), true))
	    {
	      if (matchName( rc->getDestination(), true))
		removeConnection( rc->getSource(), rc->getDestination() );
	      else
		removeConnection( stripName( rc->getSource() ), 
				  iParent->getName() );
	      aStr = stripName( rc->getSource() );
	      iNodes.find( aStr.substr( 0, aStr.find_first_of( '.' ) ) )
		->second->doReceive( aMessage );
	    }
	  else if (matchName( rc->getDestination(), true))
	    {
	      if (bRouter)
		aStr = iName + '.' + stripName( rc->getDestination() );
	      else
		aStr = stripName( rc->getDestination() );
	      rc->newDestination( aStr );
	      iParent->doReceive( aMessage );
	    }
	  else
	    { // not related to the source or destination
              _WARN_("About to throw exception. RemoveConnection message "
                     << "was not handled");
	      throw bad_message( aMessage.getType() );
	    }
	}
      break;
    default:
      {
        _WARN_("The base class, called 'Node', for node=" << getName()
               << " received a message type (" << aMessage.getType()
               << ") from source=" << aMessage.getSource()
               << " which was not handled. Throwing an exception. ");
        throw bad_message( aMessage.getType() );
      }
    }
}

/// Get name of node
/**
 * @param full do we want the simple name of the node or its full name 
 * @param router are we only looking for router names now?
 * @return name of node
 */
std::string Node::getName( bool full, bool router ) throw ()
{
  if ((!full) || isRoot())
    return iName;
  
  Node* aNode = iParent;
  bool last = true;
  while (!isRoot( aNode ))
    {
      if (aNode->bRouter)
	last=false;
      aNode = aNode->iParent;
    }
  if (last)
    return iName;
  
  if (bRouter || !router)
    return iParent->getName( true, true ) + '.' + iName;
  else
    return iParent->getName( true, true );
}

/// Match name with own name or potential child's
/**
 * @param aName name to be compared
 * @param aChild can this match a child or just node itself
 * @return whether name matches
 */
bool Node::matchName( std::string aName, bool aChild )
{
  // is iName in there somewhere?
  if (aName.find( iName ) != aName.npos)
    {
      std::string fullName = getName( true );
      std::string::size_type anST = fullName.find( aName );
      // is the full name a completion of aName?
      if ((anST == fullName.size() - aName.size()) &&
	  ((anST == 0) || (fullName[anST-1]=='.')))
	return true;
      
      std::string childName = iName + '.';
      
      if (aChild)
	{
	  // Is my name in there?
	  for ( std::string::size_type i = aName.find( childName );
		i != aName.npos;
		i = aName.find( childName, i+1 ) )
	    {
	      // If it's just the name we're done
	      if (i == 0)
		return true;
	      // Otherwise must match part of fullName
	      anST = fullName.find( aName.substr( 0, i + iName.size() ) );
	      if ((anST == fullName.size() - i - iName.size()) && 
		  ((anST == 0) || (fullName[anST-1]=='.')))
		return true;
	    }
	}
    }
  if (aChild)
    {
      // Or finally may just be a child's name on its own
      std::string aStr = aName.substr( 0, aName.find_first_of( '.' ) );
      if (iNodes.find( aStr ) != iNodes.end() )
	return true;
      for ( std::list< Node* >::iterator anIter =
	      iBranches.begin(); anIter != iBranches.end(); anIter++ )
	{
	  if ((*anIter)->iName == aStr)
	    return true;

          // To allow for synapse-on-synapse connections to work, the
          // check of iBranches has to be more sophisticated: we have to
          // actually descend into the branch by using matchName(...,
          // true) again.  Without this line, there is a problem when a
          // synapse-on-synapse sends a message up the node tree: the
          // message causes an error when it arrives at the associated
          // parent (of parent) neuron, because the source (the synapse
          // on synapse) can not be 'seen' from the neuron. This extra
          // line (below) fixeds that problem.
          // DJS 08/10/2003
          if ((*anIter)->matchName(aName, aChild))
              return true;
        }
    }
  return false;
}

/// Returns name of child with own name removed if a child, else unchanged
/**
 * @param aName name to be adjusted
 * @return adjusted name
 */
std::string Node::stripName( std::string aName )
{
  // Is it a child?
  if ((!matchName( aName, false )) && (matchName( aName, true )))
    {
      std::string::size_type anST;
      std::string childName = iName + '.';
      std::string fullName = getName( true );
      for ( std::string::size_type i = aName.find( childName );
	    i != aName.npos; i = aName.find( childName, i+1 ) )
	{
	  // If it's just the name we're done
	  if (i == 0)
	    return aName.substr( childName.size(), aName.npos );
	  // Otherwise must match fullname
	  anST = fullName.find( aName.substr( 0, i + iName.size() ) );
	  if ((anST == fullName.size() - i - iName.size()) && 
	      ((anST == 0) || (fullName[anST-1]=='.')))
	    return aName.substr( i + childName.size(), aName.npos );
	}
    }
  return aName;
}

/// Output current state of node to stream
/**
 * @param anOStream desired output stream
 * @param average true to get average since last reading, else current
 * @return output stream after output
 */
std::ostream& Node::put( std::ostream& anOStream, bool average )
{
  double anArray[BUFF_MAX];
  double* theEndPtr = put( anArray, average );
  char output[BUFF_MAX];

  for ( double* aPtr = anArray; aPtr < theEndPtr; aPtr++)
    {
      sprintf( output + strlen( output ), "%f\t", *aPtr );
    }

  return anOStream.write( output, strlen( output ) );
}

/// Output current state of node using stream iterator
/**
 * @param anOSIter desired output iterator
 * @param average true to get average since last reading, else current
 * @return iterator after output
 */
void Node::put( std::ostream_iterator< double >& anOSIter,
		bool average )
{
  double anArray[BUFF_MAX];
  double* theEndPtr = put( anArray, average );

  for ( double* aPtr = anArray; aPtr < theEndPtr; aPtr++ )
    {
      *(anOSIter++) = *aPtr;
    }
}

/// Output current state of node to buffer
/**
 * @param aBuffer desired output buffer
 * @param average true to get average since last reading, else current
 * @return pointer in buffer to end of output
 */
char* Node::put( char* aBuffer, bool average )
{
  double anArray[BUFF_MAX];
  double* theEndPtr = put( anArray, average );
  char* theBufferPtr = aBuffer;

  for ( double* aPtr = anArray; aPtr < theEndPtr; aPtr++)
    {
      sprintf( theBufferPtr, "%f\t", *aPtr );
      theBufferPtr += strlen( theBufferPtr );
    }

  return theBufferPtr;
}

/// Output current state of node using file descriptor
/**
 * @param anFD desired file descriptor to send output to
 * @param average true to get average since last reading, else current
 */
void Node::put( int anFD, bool average )
{
  double anArray[BUFF_MAX];
  double* theEndPtr = put( anArray, average );
  ::write( anFD, (void *) anArray, 
	   (size_t) sizeof( double ) * (theEndPtr - anArray) );
}

/// Handy extra function for outputting parameters
/**
 * @param started have we already started parameters
 * @param anOStream stream for output
 */
void Runtime::do_params( bool& started, std::ostream& anOStream,
			 std::string& anIndent )
{
  if (!started)
    {
      started = true;
      anOStream << std::endl << anIndent << "{" << std::endl;
      anIndent.push_back( ' ' );
    } 
}
