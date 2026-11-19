// Basic type of fileinput
// $Id: FileInput.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __FILEINPUT_HH
#define __FILEINPUT_HH

#include <unistd.h>
#include <string>
#include <vector>
#include <map>
#include <list>
#include <valarray>
#include <fstream>
#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"

#pragma interface

namespace Runtime
{

  /// Input from old file recordings
  class FileInput : public Node
  {
  public:
    /// Constructor fileinput attached to current context
    /**
     * @param aName unique name for fileinput
     * @param aFilename name of file to input data from
     * @exception bad_node thrown if name already used
     */
    FileInput( std::string aName, std::string aFilename )
      throw ( bad_node, bad_message, bad_param, bad_file ) :
      Node( aName ), iFilename( aFilename )
    {
      popContext();
      initialise();
    }

    /// Constructor for sub-fileinput
    /**
     * @param aName unique name for fileinput
     * @param aParentName name of parent router
     * @param aFilename name of file to input data from
     * @exception bad_node thrown if no such parent, or name used
     */
    FileInput( std::string aName, std::string aParentName, std::string aFilename ) 
      throw ( bad_node, bad_message, bad_param, bad_file ) 
      : Node( aName, aParentName ), iFilename( aFilename )
    {
      popContext();
      initialise();
    }

    /// Static constructor for fileinput
    /**
     * @param aName unique name for fileinput
     * @exception bad_node thrown if name used
     */
    static void newFileInput( std::string aName, std::string aFilename ) 
      throw ( bad_node, bad_message, bad_param, bad_file )
    {
      FileInput *aR;
      aR = new FileInput( aName, aFilename );
    }

    /// Destructor for fileinput
    ~FileInput()
    {
      if (iBinary)
	{
	  if (iFD != -1)
	    ::close( iFD );
	}
      else
	{
	  if (iFileStream.is_open())
	    iFileStream.close();
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

    /// Add a node to router's lookup for reference
    /**
     * @param aNode pointer to new node
     * @exception bad_node thrown if name already used
     */
    virtual void addNode( Node* aNode ) throw ( bad_node )
    {
      throw bad_node( aNode->getName( true ));
    }

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

    /// Called on end of experiment for cleanup
    /**
     * Reopens file at end of experiment
     */
    virtual void cleanup()
    {
      initialise();
    }

    // Adds a new connection controlled by the fileinput
    /*
     * NOT IMPLEMENTED
     * When one of the fileinputs' branches passes a message to it, the
     * message will be rerouted according to what this connection
     * method has specified. More that more than one rerouting can
     * happen for each message (it can form a 1:many connection) 
     * @param aSource the branch the message will come from
     * @param aDestination the node which the message should go to
     * @param aDelay the delay before passing on the message
     * @exception bad_node if the connection already exists or the destination
     * does not
     */
//      void addConnection( std::string aSource, std::string aDestination, 
//  			unsigned int aDelay ) throw ( bad_node );

    /// Removes an existing connection controlled by the fileinput
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

    /// Output initialisation script for fileinput
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for fileinput
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
    /// @var Name of file root to get data from
    std::string iFilename;

    /// @var The filestream opened
    std::ifstream iFileStream;

    /// @var file descriptor for filestream if input is binary
    int iFD;

    /// @var Is the input binary or text
    bool iBinary;

    /// @var Number of entries in the names file
    unsigned int iNumEntries;

    /// @var vector of names by column
    std::vector< std::string > iNames;

    /// @var vector of names of nodes by column
    std::vector< std::string > iNodeNames;

    /// @var vector of types of input by column
    std::vector< MessageType > iTypes;

    /// @var vector of number of connections for each input
    std::vector< int > iNumConns;
    
    /// @var Map of names of targets of messages
    std::map< int, std::list< std::string > > iTargets;
    
    /// @var vector of current values
    std::valarray< double > iCurrent;

    /// @var sum of iCurrent since last output
    std::valarray< double > iSum;

    /// @var updates since last output
    double iCount;

    /// @var Map of columns with messages to be sent to parent
    std::map< int, Message* > iMessages;
    
  private:
    /// Constructor for normal fileinput not allowed, so private
    /**
     * @param aName unique name for fileinput
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent, or must be root...
     */
    FileInput( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message ) : 
      Node( aName, aParent, noMessage )
    {
      throw bad_node( iName );
    }
  };

}

#endif //ndef __NODE_HH
