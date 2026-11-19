// Basic type of generatedinput
// $Id: GeneratedInput.hh,v 1.2 2003/12/06 00:07:34 darrens Exp $

#ifndef __GENERATEDINPUT_HH
#define __GENERATEDINPUT_HH

#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>
#include "Exception.hh"
#include "Message.hh"
#include "Node.hh"
#include "bad_peripheral.hh"

#pragma interface

namespace Runtime
{

  /// Artificial Generated Inputs
  class GeneratedInput : public Node
  {
  public:
    /// Constructor generatedinput attached to current context
    /**
     * @param aName unique name for generatedinput
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if name already used, bad_peripheral
     * if called with an illegal peripheral type */
    GeneratedInput( std::string aName, PeripheralType aType, VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral ) :
      Node( aName, analogueMessage, false ), iType( aType )
    {
      switch (aType)
	{
	case INPUT_RANDOM:
	  iArgs.resize(2);
	  if (aVA.num_var == 0)
	    {
	      iArgs[0] = 0; // Uniform [0,1]
	      iArgs[1] = 1;
	    }
	  else if (aVA.num_var == 1)
	    {
	      iArgs[0] = 0; // Uniform [0,arg(0)]
	      iArgs[1] = aVA.INT_ptr[0];
	    }
	  else if (aVA.num_var == 2)
	    {
	      iArgs[0] = aVA.INT_ptr[0];
	      iArgs[1] = aVA.INT_ptr[1];
	    }
	  else
	    {
	      throw bad_peripheral( aType );
	    }
	  break;
        case INPUT_LATCH:
            iArgs.resize(2);
            iArgs[0] = aVA.INT_ptr[0];
            iArgs[1] = aVA.INT_ptr[1];
            break;
	case INPUT_CONSTANT:
	  iArgs.resize(1);
	  if (aVA.num_var == 0)
	    iArgs[0] = 1; // constant = 1
	  else if (aVA.num_var == 1)
	    iArgs[0] = aVA.INT_ptr[0];
	  else
	    throw bad_peripheral( aType );
	  break;
	case INPUT_INTERVAL:
	  iArgs.resize(6);
	  if (aVA.num_var == 2)
	    {
	      iArgs[0] = 1; // Fixed volume trill
	      iArgs[1] = aVA.INT_ptr[0];
	      iArgs[2] = 0;
	      iArgs[3] = aVA.INT_ptr[1];
	      iArgs[4] = 1;
	      iArgs[5] = 0;
	    }
	  else if (aVA.num_var == 4)
	    {
	      iArgs[0] = aVA.INT_ptr[0]; // Trill
	      iArgs[1] = aVA.INT_ptr[1];
	      iArgs[2] = aVA.INT_ptr[2];
	      iArgs[3] = aVA.INT_ptr[3];
	      iArgs[4] = 1;
	      iArgs[5] = 0;
	    }
	  else if (aVA.num_var == 6)
	    {
	      iArgs[0] = aVA.INT_ptr[0]; // Chirps
	      iArgs[1] = aVA.INT_ptr[1];
	      iArgs[2] = aVA.INT_ptr[2];
	      iArgs[3] = aVA.INT_ptr[3];
	      iArgs[4] = aVA.INT_ptr[4];
	      iArgs[5] = aVA.INT_ptr[5];
	    }
	  else
	    throw bad_peripheral( aType );
	  break;
	default:
	  throw bad_peripheral( aType );
	  break;
	}
      initialise();
    }

    /// Constructor for sub-generatedinput
    /**
     * @param aName unique name for generatedinput
     * @param aParentName name of parent router
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if no such parent, or name used
     */
    GeneratedInput( std::string aName, std::string aParentName, PeripheralType aType,
		    VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral ) 
      : Node( aName, aParentName, analogueMessage, false ),
      iType( aType )
    {
      switch (aType)
	{
	case INPUT_RANDOM:
	  iArgs.resize(2);
	  if (aVA.num_var == 0)
	    {
	      iArgs[0] = 0; // Uniform [0,1]
	      iArgs[1] = 1;
	    }
	  else if (aVA.num_var == 1)
	    {
	      iArgs[0] = 0; // Uniform [0,arg(0)]
	      iArgs[1] = aVA.INT_ptr[0];
	    }
	  else if (aVA.num_var == 2)
	    {
	      iArgs[0] = aVA.INT_ptr[0];
	      iArgs[1] = aVA.INT_ptr[1];
	    }
	  else
	    {
	      throw bad_peripheral( aType );
	    }
	  break;
	case INPUT_CONSTANT:
	  iArgs.resize(1);
	  std::cout << aVA.num_var << std::endl;
	  if (aVA.num_var == 0)
	    iArgs[0] = 1; // constant = 1
	  else if (aVA.num_var == 1)
	    iArgs[0] = aVA.INT_ptr[0];
	  else
	    throw bad_peripheral( aType );
	  break;
        case INPUT_LATCH:
            iArgs.resize(2);
            iArgs[0] = aVA.INT_ptr[0]; 
            iArgs[1] = aVA.INT_ptr[1]; 
            break;
	case INPUT_INTERVAL:
	  iArgs.resize(6);
	  if (aVA.num_var == 2)
	    {
	      iArgs[0] = 1; // Fixed volume trill
	      iArgs[1] = aVA.INT_ptr[0];
	      iArgs[2] = 0;
	      iArgs[3] = aVA.INT_ptr[1];
	      iArgs[4] = 1;
	      iArgs[5] = 0;
	    }
	  else if (aVA.num_var == 4)
	    {
	      iArgs[0] = aVA.INT_ptr[0]; // Trill
	      iArgs[1] = aVA.INT_ptr[1];
	      iArgs[2] = aVA.INT_ptr[2];
	      iArgs[3] = aVA.INT_ptr[3];
	      iArgs[4] = 1;
	      iArgs[5] = 0;
	    }
	  else if (aVA.num_var == 6)
	    {
	      iArgs[0] = aVA.INT_ptr[0]; // Chirps
	      iArgs[1] = aVA.INT_ptr[1];
	      iArgs[2] = aVA.INT_ptr[2];
	      iArgs[3] = aVA.INT_ptr[3];
	      iArgs[4] = aVA.INT_ptr[4];
	      iArgs[5] = aVA.INT_ptr[5];
	    }
	  else
	    throw bad_peripheral( aType );
	  break;
	default:
	  throw bad_peripheral( aType );
	  break;
	}
      initialise();
    }

    /// Static constructor for generatedinput
    /**
     * @param aName unique name for generatedinput
     * @param aType type of input to be modelled
     * @param aVA array of arguments for input
     * @exception bad_node thrown if name used
     */
    static void newGeneratedInput( std::string aName, PeripheralType aType,
				   VarArg aVA )
      throw ( bad_node, bad_message, bad_param, bad_file, bad_peripheral )
    {
      GeneratedInput *aR;
      aR = new GeneratedInput( aName, aType, aVA );
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

    /// Output initialisation script for generatedinput
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    virtual std::ostream& externalise( std::ostream& anOStream, 
				       std::string indent = "" );

    /// Output variable names for generatedinput
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

    /// Indicates whether node can receive different messages
    /**
     * @param aMessageType type of message to be sent
     * @return true if message can be received
     */
    virtual bool canReceive( MessageType aMessageType )
    {
      switch (aMessageType)
	{
	case spikeMessage:
	  return true;
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
      switch (aMessage.getType())
	{
	case spikeMessage:
          iSpike = true;
	  break;
	default:
	  Node::doReceive( aMessage );
	  break;
	}
    }


  protected:
    /// @var type of input to be generated
    PeripheralType iType;
    /// @var Arguments for generated inputs
    std::vector< double > iArgs;
    /// @var Current state of generated input
    double iState;
    /// @var Sum of iState since last output
    double iSumState;
    /// @var Syllable in chirp for interval input
    int iSyllable;
    /// @var high or low
    bool bInSyll;
    /// @var Time step in Stage for interval input
    double iPos;
    /// @var Timestep of simulation
    double iSteps;
    /// @var Latch spike received
    bool iSpike;
    /// @Latch is up?
    bool iLatch_isUp;
    /// @var Message which will be sent to parent
    AnalogueMessage iMessage;    
  private:
    /// Constructor for normal generatedinput not allowed, so private
    /**
     * @param aName unique name for generatedinput
     * @param aParent pointer to parent for attachment
     * @exception bad_node thrown if name already used, 
     * or problem with parent, or must be root...
     */
    GeneratedInput( std::string aName, Node* aParent ) 
      throw ( bad_node, bad_message ) : 
      Node( aName, aParent, noMessage )
    {
      throw bad_node( iName );
    }
  };

}

#endif //ndef __GENERATED_INPUT_HH
