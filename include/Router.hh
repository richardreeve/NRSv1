// Basic type of router
// $Id: Router.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __ROUTER_HH
#define __ROUTER_HH

#include <string>
#include <list>
#include <map>
#include <deque>
#include <iostream>

#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{

  /// Basic Router class
  class Router : public Node
  {
  public:
    /// Constructor for root router, or attached to current context
    /**
     * @param aName unique name for router
     * @exception bad_node thrown if name already used
     */
    Router( std::string aName ) throw ( bad_node, bad_message, bad_param ) :
      Node( aName ), iUpdate( 0 )
    {
      initialise();
    }

    /// Constructor for sub-router
    /**
     * @param aName unique name for router
     * @param aParentName name of parent router
     * @exception bad_node thrown if no such parent, or name used
     */
    Router( std::string aName, std::string aParentName ) 
      throw ( bad_node, bad_message, bad_param ) 
      : Node( aName, aParentName ), iUpdate( 0 )
    {
      initialise();
    }

    /// Static constructor for router
    /**
     * @param aName unique name for router
     * @exception bad_node thrown if name used
     */
    static void newRouter( std::string aName = ROUTER ) 
      throw ( bad_node, bad_message, bad_param )
    {
      Router *aR;
      aR = new Router( aName );
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
      std::list< std::pair< Node*, unsigned int > >* theNodes;
      std::list< std::pair< Node*, unsigned int > >::iterator anIter;
      std::string aStr;
      std::string aSource;
      switch (aMessage.getType())
	{
	case noMessage:
	  break;
	case spikeMessage:
	case analogueMessage:
#ifdef DEBUG
	  if (iRoutes.find( aMessage.getSource() ) == iRoutes.end())
	    {
	      std::cerr << "Received a spike from " << aMessage.getSource()
			<< " with no destination" << std::endl;
	    }
#endif
	  if (aMessage.checkRouter( this ))
	    {
	      if(aMessage.needUpdate( iUpdate ))
		{
		  aSource = aMessage.getSource();
		  theNodes = &iRoutes[ aSource ];
		  aMessage.newTargets( theNodes, iUpdate );
		}
	      else
		theNodes = aMessage.getTargets();
	    }
	  else
	    {
	      theNodes = &iRoutes[ aMessage.getSource() ];
	    }

	  if (theNodes != NULL)
	    {
	      anIter = theNodes->begin();
	      while (anIter != theNodes->end())
		{
#ifdef DEBUG
		  aSource = aMessage.getSource();
		  std::cout << "Sending a spike from " << aSource
			    << " to " << anIter->first->getName() 
			    << " in " << anIter->second << " timesteps."
			    << std::endl;
#endif
		  if (iEvents.size() <= anIter->second)
		    {
		      iEvents.resize( (anIter->second * 50)>?1000 );
#ifdef DEBUG
		      std::cout << "Resizing event queue to " 
				<< ((anIter->second * 50)>?1000) << std::endl;
#endif
		    }
		  if (anIter->first == iParent)
		    {
		      aSource = stripName(aMessage.getSource());
		      aStr = iName + '.' + aSource;
		      if (anIter->second == 0)
			{
			  iParent->doReceive( aMessage.newSource( aStr ) );
			  //aMessage.newSource( aSource );
			}
		      else
			iEvents[anIter->second-1]
			  .push_back( std::pair< Node*, 
				      Message* >( iParent,
						  &(aMessage
						    .newSource( aStr ))
						  )
				      );
		    }
		  else
		    {
		      iEvents[anIter->second-1].push_back( std::pair< Node*, 
							   Message* >
							   ( anIter->first,
							     &aMessage ) );
		    }
		  anIter++;
		}
	    }
	  break;
	default:
	  Node::doReceive( aMessage );
	  break;
	}
    }
    
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

    /// Adds a new connection controlled by the router
    /**
     * When one of the routers' branches passes a message to it, the
     * message will be rerouted according to what this connection
     * method has specified. More that more than one rerouting can
     * happen for each message (it can form a 1:many connection) 
     * @param aSource the branch the message will come from
     * @param aDestination the node which the message should go to
     * @param aDelay the delay before passing on the message
     * @exception bad_node if the connection already exists or the destination
     * does not
     */
    void addConnection( std::string aSource, std::string aDestination, 
			unsigned int aDelay ) throw ( bad_node, bad_message )
    {
      Node* theRecipient = getNode( aDestination );
      // [] will create an empty vector if it doesn't exist
      iRoutes[aSource].push_back( std::pair< Node*, 
				  unsigned int >( theRecipient,
						  aDelay ) );
      // We have changed something, so need to update messages coming in
      iUpdate++;
    }

    /// Removes an existing connection controlled by the router
    /**
     * @param aSource the branch the message came from
     * @param aDestination the node which the message went to
     * @exception bad_node if the connection or the recipient
     * does not exist
     */
    void removeConnection( std::string aSource, std::string aDestination )
      throw ( bad_node, bad_message )
    {
      Node* theRecipient = getNode( aDestination );
      if (iRoutes.empty())
	throw bad_node( aSource );

      if (iRoutes.find( aSource ) == iRoutes.end())
	throw bad_node( aSource );

      std::list< std::pair< Node*, unsigned int > >& theNodes
	= iRoutes[aSource];
      std::list< std::pair< Node*, unsigned int > >::iterator anIter 
	= theNodes.begin();
      bool found = false;
      while ((anIter != theNodes.end()) && !found )
	{
	  if (anIter->first == theRecipient)
	    {
	      theNodes.erase( anIter );
	      found = true;
	    }
	  else
	    anIter++;
	}

      if (!found)
	throw bad_node( aDestination );
      else if (theNodes.begin() == theNodes.end())
	{
	  iRoutes.erase( iRoutes.find( aSource ) );
	}
      iUpdate++;

      // Now get rid of active events - much more annoying...
      for ( std::deque< std::list< std::pair< Node*, Message* > > >::iterator
	      aMess = iEvents.begin(); aMess != iEvents.end(); aMess++ )
	{
	  for ( std::list< std::pair< Node*, Message* > >::iterator
		  anLI = aMess->begin(); anLI != aMess->end(); anLI++ )
	    {
	      if ((anLI->first->matchName( aDestination )) &&
		  (anLI->second->getSource() == aSource))
		{
		  std::list< std::pair< Node*, Message* > >::iterator
		    li = anLI;
		  anLI--;
		  aMess->erase( li );
		}
	    }
	}
    }

    /// Do one timestep for self only
    virtual bool nodeTimestep();

    /// Output initialisation script for router
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for router
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
    /// @var Map of routes from sources to destinations
    std::map< std::string, std::list< std::pair< Node*, unsigned int > > > iRoutes;

    /// @var Queue of currently active events
    std::deque< std::list< std::pair< Node*, Message* > > > iEvents;

    /// @var Reference for this update 
    /**
     * Increased with every change to the map to check whether messages 
     * own lists are up to date
     */
    unsigned int iUpdate;

  private:
    /// Constructor for normal router not allowed, so private
    /**
     * @param aName unique name for router
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent, or must be root...
     */
    Router( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message ) : 
      Node( aName, aParent, noMessage ), iUpdate( 0 )
    {
      throw bad_node( iName );
    }
  };

}

#endif //ndef __ROUTER_HH
