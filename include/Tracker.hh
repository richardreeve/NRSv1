// Basic type of tracker
// $Id: Tracker.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __TRACKER_HH
#define __TRACKER_HH

#include <string>
#include <iostream>

#include "base.h"
#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"
#include "bad_peripheral.hh"

#pragma interface

namespace Runtime
{

  /// Artificial Generated Inputs
  class Tracker : public Node
  {
  public:
    /// Constructor Tracker attached to current context
    /**
     * @param aName unique name for Tracker
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if name already used, bad_peripheral
     * if called with an illegal peripheral type */
    Tracker( std::string aName, PeripheralType aType, VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral ) :
      Node( aName, analogueMessage, false ), iType( aType ),
      iFD( -1 ), iRunning( false ), iID(0)
    {
      switch (aType)
	{
	case TRACKER_CAMERA:
	case TRACKER_REEL:
	  if (!aVA.is_int)
	    {
	      iHost = aVA.CHAR_ptr;
	      iHost = iHost.substr(iHost.find_last_of('/')+1);
	    }
	  else if (aVA.num_var == 0)
	    {
	      iHost = "localhost";
	    }
	  else
	    {
	      throw bad_peripheral( aType );
	    }
	  break;
	default:
	  throw bad_peripheral( aType );
	  break;
	}
      initialise();
    }

    /// Constructor for sub-Tracker
    /**
     * @param aName unique name for Tracker
     * @param aParentName name of parent router
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if no such parent, or name used
     */
    Tracker( std::string aName, std::string aParentName, PeripheralType aType,
		    VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral ) 
      : Node( aName, aParentName, analogueMessage, false ),
      iType( aType ), iFD( -1 ), iRunning( false ), iID(0)
    {
      switch (aType)
	{
	case TRACKER_CAMERA:
	case TRACKER_REEL:
	  if (aVA.is_int)
	    {
	      iHost = aVA.CHAR_ptr;
	      iHost = iHost.substr(iHost.find_last_of('/')+1);
	    }
	  else if (aVA.num_var == 0)
	    {
	      iHost = "localhost";
	    }
	  else
	    {
	      throw bad_peripheral( aType );
	    }
	  break;
	default:
	  throw bad_peripheral( aType );
	  break;
	}
      initialise();
    }

    /// Static constructor for Tracker
    /**
     * @param aName unique name for Tracker
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if name used
     */
    static void newTracker( std::string aName, PeripheralType aType,
				   VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral )
    {
      Tracker *aR;
      aR = new Tracker( aName, aType, aVA );
    }

    /// Destructor for Tracker
    ~Tracker();

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
    virtual void initialise() throw ( bad_peripheral );

    /// Called on end of experiment for cleanup
    /**
     * Reopens file at end of experiment
     */
    virtual void cleanup()
    {
      initialise();
    }

    /// Do one timestep for self only
    virtual bool nodeTimestep();

    /// Output initialisation script for Tracker
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for Tracker
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
    /// @var type of input to be generated
    PeripheralType iType;
    /// @var Host where tracker is sitting
    std::string iHost;
    /// @var Current x coordinate
    double iX;
    /// @var Current y coordinate
    double iY;
    /// @var Current angle
    double iTheta;
    /// @var Sum of iX since last output
    double iSumX;
    /// @var Sum of iY since last output
    double iSumY;
    /// @var Sum of iTheta since last output
    double iSumTheta;
    /// @var Tracker's file descriptor
    int iFD;
    /// @var Is tracker running
    bool iRunning;
    /// @var Tracker's current id
    int iID;
    /// @var Message which will be sent to parent
    AnalogueMessage iMessage;    
  private:
    /// Constructor for normal Tracker not allowed, so private
    /**
     * @param aName unique name for Tracker
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent, or must be root...
     */
    Tracker( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message ) : 
      Node( aName, aParent, noMessage )
    {
      throw bad_node( iName );
    }
  };

}

#endif //ndef __NODE_HH
