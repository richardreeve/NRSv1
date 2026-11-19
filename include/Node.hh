// Basic unit of simulation
// $Id: Node.hh,v 1.2 2003/09/11 19:01:33 darrens Exp $

#ifndef __NODE_HH
#define __NODE_HH

#include <unistd.h>
#include <string>
#include <list>
#include <map>
#include <stack>
#include <algorithm>
#include <bitset>
#include <iostream>
#include <iterator>

#include "Exception.hh"
#include "Params.hh"
#include "Message.hh"
#include "base.h"

#pragma interface

// Types of output
typedef enum
{
  outputSensor=0,
  outputSynapse,
  outputFacDep,
  outputSpike,
  outputMembV,
  outputMembG,
  outputAxon,
  outputMotor,
  outputTrack,
  outputInput
} OutputLevel;
#define MAX_TYPES 10

/// The Runtime namespace contains all of the elements of a controller.
/** 
 * Node is the base class for nodes in the runtime tree. All elements,
 * whether simulated or not, are derived from the Node class, and all
 * exceptions thrown by nodes are derived from the bad_node class.
 */
namespace Runtime
{
  /// Exception class indicating node was inappropriate for task chosen
  class bad_node : public runtime_exception
  {
  public:
    bad_node() : runtime_exception() { }
    bad_node( std::string aName ) : runtime_exception( aName ) { }
    bad_node( std::string aName,
              std::string aHint,
              std::string aOrigin_file,
              int aOrigin_line ) : runtime_exception( aName,
                                                      aHint,
                                                      aOrigin_file,
                                                      aOrigin_line) { }
  };

  /// Base class for all Runtime elements in controller
  class Node
  {
  public:
    /// Constructor for a node with unspecified parent
    /**
     * This node will be a child of the current context if root exists
     * otherwise it will be root if it is allowed
     * @param aName unique name for node
     * @param aMType type of message it will send to parent by default
     * @param aRouter is it a router, i.e. can it be root,
     * or will it be a subroot if not
     */
    Node( std::string& aName, MessageType anMType = noMessage,
	  bool aRouter = true )
      throw ( bad_node ) : iName( aName ), iMessageType( anMType ),
			   iParent( this ), iCount( 0.0 ),
			   bRouter( aRouter )
    {
      if (iName.find_first_of( " .\t\n" ) != iName.npos)
	throw bad_node( iName );

      if (isRoot( NULL ))
	{
	  if (!bRouter)
	    throw bad_node( iName );
	  setRoot( this );
	}
      else
	{
	  iParent = getContext();
	  iParent->addNode( this );
	  iParent->addBranch( this );
	  iParams = getContext()->getParams();
	}
      if (bRouter)
	pushContext( this );
    }

    /// Constructor for a node with specified parent
    /**
     * This node will be a child of the specified parent
     * @param aName unique name for node
     * @param aParent pointer to parent node
     * @param anMType type of message it will send to parent by default
     * @param aRouter is it a router, i.e. can it be a subroot
     */
    Node( std::string& aName, Node* aParent, MessageType anMType = noMessage,
	  bool aRouter = true )
      throw ( bad_node ) : iName( aName ), iMessageType( anMType ),
			   iParent( aParent ), iCount( 0.0 ),
			   bRouter( aRouter )
    {
      if (iName.find_first_of( " .\t\n" ) != iName.npos)
	throw bad_node( iName );

      iParent->addNode( this );
      iParent->addBranch( this );
      iParams = getContext()->getParams();
      if (bRouter)
	pushContext( this );
    }
    
    /// Constructor for a node with named parent
    /**
     * This node will be a child of the named parent if it exists
     * @param aName unique name for node
     * @param aParentName name of parent node
     * @param anMType type of message it will send to parent by default
     * @param aRouter is it a router? i.e. can it be a subroot
     */
    Node( std::string& aName, std::string& aParentName, MessageType anMType = noMessage,
	  bool aRouter = true )
      throw ( bad_node ) : iName( aName ), iMessageType( anMType ),
			   iCount( 0.0 ), bRouter( aRouter )
    {
      if (iName.find_first_of( " .\t\n" ) != iName.npos)
	throw bad_node( iName );
      
      if (isRoot( NULL ))
	throw bad_node( iName );

      iParent = getContext()->getNode( aParentName );

      iParent->addNode( this );
      iParent->addBranch( this );
      iParams = getContext()->getParams();
      if (bRouter)
	pushContext( this );
    }

    /// default destructor
    /**
     * Kills branches, then kills self by removing link to parent and
     * any cross-references governed by root (unless it is root)
     */
    virtual ~Node()
    {
      if (!iBranches.empty())
	{
	  std::cerr << "~Node:: Must call removeAll before deleting nodes..."
		    << std::endl;
	  throw bad_node( iName );
	}

      if (!iRemote.empty())
	{
	  std::cerr << "~Node:: Must call removeAll before deleting nodes..."
		    << std::endl;
	  throw bad_node( iName );
	}

      if (!isRoot())
	{
	  iParent->removeBranch( this );
	  iParent->removeNode( iName );
	  if (getContext() == this)
	    popContext();
	}
      else
	{
	  setRoot( NULL );
	}
    }

    /// remove all external links
    /**
     * Kills branches, then kills all remote links - an essential precursor
     * to calling delete
     * @param aName name of node to be stripped - usually self
     * @return whether node can be deleted
     */
    virtual bool removeAll( std::string aName = "" ) throw ( bad_node );

    /// Static destructor
    /**
     * Uses default destructor, but accepts name as input
     * @param aName name of node to be destroyed
     */
    static void destroy( std::string aName ) throw ( bad_node )
    {
      if (isRoot( NULL ))
	throw bad_node( aName );
      else
	if (getContext( true )->getNode( aName )->removeAll( aName ))
	  delete getContext( true )->getNode( aName );
    }

    /// Add a node to router's lookup for reference
    /**
     * @param aNode pointer to new node
     * @exception bad_node thrown if name already used
     */
    virtual void addNode( Node* aNode ) throw ( bad_node )
    {
      if (!bRouter)
	iParent->addNode( aNode );
      else
	{
	  std::string aName = aNode->getName();
	  if ( iNodes.find( aName ) != iNodes.end() )
	    throw bad_node( aName );
	  iNodes[aName] = aNode;
	  iList.push_back( aNode );
	}
    }

    /// Get node by name from router
    /**
     * @param aName name of node
     * @return pointer to node
     * @exception bad_node thrown if name not used
     */
    virtual Node* getNode( std::string aName ) throw ( bad_node );

    /// Delete node from router's lookup table
    /**
     * @param aName name of node
     * @exception bad_node thrown if name not used
     */
    void removeNode( std::string aName ) throw ( bad_node )
    {
      if (!bRouter)
        iParent->removeNode( aName );
      else
	{
	  if ( iNodes.find( aName ) == iNodes.end() )
	    throw bad_node( aName );
	  iList.remove( iNodes[aName] );
	  iNodes.erase( iNodes.find( aName ) );
	}
    }

    /// Add a branch to the current node
    /**
     * @param aNode branch to be added
     */
    void addBranch( Node *aNode )
    {
      iBranches.push_back( aNode );
    }

    /// Delete a branch from the current node
    /**
     * @param aNode branch to be deleted
     */
    void removeBranch( Node *aNode ) throw ( bad_node )
    {
      std::list< Node* >::iterator anIter = find( iBranches.begin(),
						  iBranches.end(), aNode );

      if (anIter == iBranches.end())
	throw bad_node( iName );
      else
	iBranches.erase( anIter );
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
    virtual void addConnection( std::string aSource, std::string aDestination,
				unsigned int aDelay ) throw ( bad_node,
							      bad_message );

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

    void addConnection( std::string& anOther, bool aSource, unsigned int aDelay )
      throw ( bad_node, bad_message  );

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
    virtual void removeConnection( std::string aSource, std::string aDestination )
      throw ( bad_node, bad_message );

    /// Removes the record of a remote connection from a node
    /**
     * @param anOther other end of connection
     * @param first is this the first end of the connection to be told?
     * @exception bad_node thrown if connection or node at other end
     * does not already exist
     */
    void removeConnection( std::string anOther )
      throw ( bad_node, bad_message );

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
    
    /// Retrieves a reference to the Params
    Params& getParams()
    {
      return iParams;
    }
    
    /// Sets the Params to a new value
    /**
     * @param aParams set of parameters
     */
    void setParams( const Params& aParams )
    {
      iParams = aParams;
    }
    
    /// Get name of node
    /**
     * @param full do we want the simple name of the node or its full name 
     * @param router are we only looking for router names now?
     * @return name of node
     */
    std::string getName( bool full = false, bool router = false ) throw ();

    /// Match name with own name or potential child's
    /**
     * @param aName name to be compared
     * @param aChild can this match a child or just node itself
     * @return whether name matches
     */
    bool matchName( std::string aName, bool aChild = false );

    /// Returns name of child with own name removed if a child, else unchanged
    /**
     * @param aName name to be adjusted
     * @return adjusted name
     */
    std::string stripName( std::string aName );

    /// Do one timestep for entire branch
    /**
     * This carries out one timestep for its branches and then for
     * itself using nodeTimestep()
     * @returns whether we should continue the experiment
     */
    bool doTimestep()
    {
      bool doContinue = true;
      for ( std::list< Node* >::iterator anIter = iBranches.begin();
	    anIter != iBranches.end(); anIter++ )
	{
	  //	  std::cerr << (*anIter)->getName(true) << std::endl;
	  doContinue &= (*anIter)->doTimestep();
	}
      doContinue &= nodeTimestep();
      return doContinue;
    }

    /// Do one timestep for self only
    /**
     * This carries out one timestep for itself alone. It is a pure
     * virtual function as all node must do something to update.
     * @returns whether we should continue the experiment
     */
    virtual bool nodeTimestep()=0;

    /// Do initialise for entire branch
    /**
     * This carries out initialise for its branches and then for
     * itself using initialise()
     */
    void doInitialise()
    {
      if ((!isRoot()) && (iParent->getParams().getParam( "stepT" ) !=
			  iParams.getParam( "stepT" )))
	{
	  std::cerr << "Warning: changing timestep on " 
		    << getName( true ) << " to " 
		    << iParent->getParams().getParam( "stepT" )
		    << Params::getParameterUnits( "stepT" ) << std::endl;
	  iParams.setParam( "stepT", iParent->getParams().getParam( "stepT" ));
	}

      initialise();
      if (!iBranches.empty())
	for_each ( iBranches.begin(), iBranches.end(), 
		   std::mem_fun( &Node::doInitialise ) );
    }

    /// Initialises and configures the node
    /**
     * This is a pure virtual function as every node must have some
     * initialisation procedures. It is called by constructors, but
     * can be called at any point (if parameters have been changed,
     * for instance). */
    virtual void initialise()=0;

    /// Do cleanup for entire branch
    /**
     * This carries out cleanup for its branches and then for
     * itself using cleanup()
     */
    void doCleanup()
    {
      cleanup();
      if (!iBranches.empty())
	for_each ( iBranches.begin(), iBranches.end(), 
		   std::mem_fun( &Node::doCleanup ) );
    }
    /// Called on end of experiment for cleanup
    /**
     * This method is generally empty, but allows cleanup to be
     * managed at the end of an experiment if required
     */
    virtual void cleanup()
    {
    }

    /// Initialise parameter names for this node type
    /**
     * @exception bad_param thrown if the parameter already exists */
    static void initParams() throw ( bad_param, bad_file );

    /// Accessor for global root
    /**
     * @return the global root node for the whole network
     */
    static Node *getRoot()
    {
      return gRoot;
    }

    /// Return a node's router
    /**
     * @return the immediate router for a node
     */
    Node* getRouter()
    {
      if (isRoot())
	return this;

      if (iParent->isRouter())
	return iParent;
      
      return iParent->getRouter();
    }

    /// Test whether a node is a router
    /**
     * @param aNode the pointer to the node to be tested (default this)
     * @return true if node is a router
     */
    bool isRouter()
    {
      return bRouter;
    }

    /// Test whether a node is root (default this node)
    /**
     * @param aNode the pointer to the node to be tested (default this)
     * @return true if node is the root node
     */
    bool isRoot()
    {
      return (getRoot() == this);
    }

    static bool isRoot( Node* aNode )
    {
      return (getRoot() == aNode);
    }

    /// Mutator for global root name
    /**
     * @param aName new name for root
     */
    static void setRootName( std::string aName )
    {
      if (isRoot( NULL ))
	throw bad_node( aName );
      
      if (aName.find_first_of( " .\t\n" ) != aName.npos)
	throw bad_node( aName );
      
      if ( getRoot()->iNodes.find( aName ) != getRoot()-> iNodes.end() )
	throw bad_node( aName );
      
      getRoot()->iName = aName;
    }

    /// Mutator for global root
    /**
     * @param aRoot the root node for the whole network
     */
    static void setRoot( Node *aRoot )
    {
      gRoot = aRoot;
      while (!gContext.empty())
	gContext.pop();
//        if (gRoot != NULL)
//  	gContext.push( gRoot );
    }

    /// Global destructor for network
    static void destroyAll()
    {
      if (!isRoot( NULL ))
	{
	  if (getRoot()->removeAll())
	    delete getRoot();
	}
    }
    
    /// Accessor for global context
    /**
     * @param bForce if there is no current context do we force return of root
     * @return the current context for the network (ie current router)
     */
    static Node *getContext( bool bForce = false )
    {
      if (isRoot( NULL ))
	throw bad_node( "Context" );
      if (gContext.empty())
	{
	  if (bForce)
	    return getRoot();
	  else
	    return NULL;
	}
      else
	return gContext.top();
    }

    /// Mutator for global context
    /**
     * @param aNode the current context for the network
     */
    static void pushContext( Node *aNode )
    {
      if (aNode->isRouter())
	gContext.push( aNode );
      else
	throw bad_node( aNode->getName( true ) );
    }

    /// Pops current global context
    static void popContext()
    {
      if (gContext.empty())
	{
	  if (!isRoot( NULL ))
	    throw bad_node( "Context" );
	}
      else
	gContext.pop();
      
//        if (gContext.empty())
//  	{
//  	  if (isRoot( NULL ))
//  	    throw bad_node( "Context" );
//  	  gContext.push( getRoot() );
//  	}
    }

    /// Accessor for output level
    /**
     * @return reference to output level
     */
    static std::bitset<MAX_TYPES>& getOutputLevel()
    {
      return gOutputLevel;
    }

    /// Mutator for output level
    /**
     * @param anOutputLevel new output level
     */
    static void setOutputLevel( std::bitset< MAX_TYPES >& anOutputLevel )
    {
      gOutputLevel = anOutputLevel;
    }
    static void setOutputLevel( int anOutputLevel )
    {
      gOutputLevel = static_cast< unsigned long int >( anOutputLevel );
    }

    /// Sets one bit in output level
    /**
     * @param anOutputLevel new output level
     */
    static void setOutput( OutputLevel anOutput, int aVal = 1 )
    {
      gOutputLevel.set( static_cast< int >( anOutput ), aVal );
    }
    static void setOutput( int anOutput, int aVal = 1 )
    {
      gOutputLevel.set( anOutput, aVal );
    }

    /// Clears one bit in output level
    /**
     * @param anOutputLevel new output level
     */
    static void resetOutput( OutputLevel anOutput )
    {
      gOutputLevel.reset( static_cast< int >( anOutput ) );
    }
    static void resetOutput( int anOutput )
    {
      gOutputLevel.reset( anOutput );
    }

    /// Output initialisation script for node
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" ) = 0;

    /// Output variable names for node
    /**
     * This is pure virtual, because nodes do not have variables
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& write( std::ostream& anOStream,
				 OutputType aType = outTypeText ) = 0;

    /// Output current state of node to array
    /**
     * This is used by all the other put methods to gather data
     * @param anArray array of doubles to store data in
     * @param average true to get average since last reading, else current
     * @return pointer to end of used part of array
     */
    virtual double* put( double* anArray, bool average = true ) = 0;

    /// Output current state of node to stream
    /**
     * @param anOStream desired output stream
     * @param average true to get average since last reading, else current
     * @return output stream after output
     */
    std::ostream& put( std::ostream& anOStream, bool average = true );

    /// Output current state of node using stream iterator
    /**
     * @param anOSIter desired output iterator
     * @param average true to get average since last reading, else current
     * @return iterator after output
     */
    void put( std::ostream_iterator< double >& anOSIter,
	      bool average = true );

    /// Output current state of node to buffer
    /**
     * @param aBuffer desired output buffer
     * @param average true to get average since last reading, else current
     * @return pointer in buffer to end of output
     */
    char* put( char *file, bool average = true );

    /// Output current state of node using file descriptor
    /**
     * @param anFD desired file descriptor to send output to
     * @param average true to get average since last reading, else current
     */
    void put( int anFD, bool average = true );

    /// Input current state of node
    /**
     * This throws an exception by default, as not all nodes have input
     * @param anOStream desired input stream
     * @return input stream after output
     */
    virtual std::istream& get( std::istream& anOStream )
    {
      throw bad_node();
    }

  private:
    /// no default constructor
    Node() {}

  protected:
    /// @var The global root
    static Node *gRoot;
    /// @var Current context
    static std::stack< Node* > gContext;
    /// @var The global output level
    static std::bitset< MAX_TYPES > gOutputLevel;
    /// @var name of node
    std::string iName;
    /// @var type of messages it sends
    MessageType iMessageType;
    /// @var reference to parent
    Node *iParent;
    /// @var Parameters for this node and its branches
    Params iParams;
    /// @var Map of all names onto nodes
    std::map< std::string, Node* > iNodes;
    /// @var List of nodes in order of definition
    std::list< Node* > iList;
    /// @var list of all branches
    std::list< Node* > iBranches;
    /// @var map of all remote connections, and whether this node is the source
    std::map< std::string, bool > iRemote;
    /// @var count since last output
    double iCount;
    /// @var Can this node be a root node? ie is it a router
    bool bRouter;
  };

  //  extern Node* Node::gRoot;
  
  //  extern std::bitset<MAX_TYPES> Node::gOutputLevel;

  std::ostream& operator<<( std::ostream& anOStream, Runtime::Node& anOutput );
  std::istream& operator>>( std::istream& anIStream, Runtime::Node& anInput );


  /// Handy extra function for outputting parameters
  /**
   * @param started have we already started parameters
   * @param anOStream stream for output
   */
  void do_params( bool& started, std::ostream& anOStream, std::string& anIndent );
}



#endif //ndef __NODE_HH
