// Peripheral model for robot
// $Id: Peripheral.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __PERIPHERAL_HH
#define __PERIPHERAL_HH

#include <valarray>
#include <iostream>
#include <string>

#include "base.h"
#include "peripheral.h"
#include "Message.hh"
#include "Exception.hh"
#include "low_interface.h"
#include "bad_peripheral.hh"

#pragma interface

namespace Runtime
{
  /// Robot peripherals
  class Peripheral
  {
  public:
    /// Full constructor for Peripheral
    /**
     * rarely used, as vector of peripherals is initialised at start
     * @param aName name of Peripheral
     * @param anInput is the peripheral an input or an output
     * @param aType the peripheral type
     * @param aVarArg arguments used to create peripheral
     */
    Peripheral( std::string aName, bool anInput, PeripheralType aType,
		VarArg aVarArg ): bDeleted( false ), iName( aName ),
				  bInput( anInput ), bLowFrequency( false ),
				  iPeriod( 1 ), iPerType( aType ),
				  iState( 0 ), iSumState( 0 ), iCount( 0 )
    {
      INT aBuff[PERIPHERAL_DATA];

      if (Peripheral_robot[ static_cast< int >( iPerType ) ])
	{
	  if (!low_peripheral_add( (bInput ? TRUE : FALSE), iPerType, aVarArg,
				   aBuff ) )
	    throw bad_peripheral( iName );
	}
      else
	throw bad_peripheral( iName ); // these are robot peripherals only!
      
      iRef = aBuff[PERIPHERAL_REF]; // robot reference
      bBits = (aBuff[PERIPHERAL_BITS] == FALSE ? false : true );
      // is it sending bits?
      iNumElts = aBuff[PERIPHERAL_NUM_ELTS]; // How many?

      if ((bBits) && (iNumElts == 1))
	iMessageType = spikeMessage;
      else
	iMessageType = analogueMessage;

      bArgInts = static_cast< bool >( aVarArg.is_int );
      if (bArgInts)
	{
	  iArgInts.resize( aVarArg.num_var );
	  for ( int i = 0; i < aVarArg.num_var; i++ )
	    iArgInts[i] = aVarArg.INT_ptr[i];
	}
      else
	iArgString = aVarArg.CHAR_ptr;

      if (Peripheral_lowfreq[ static_cast< int >( iPerType ) ])
	{
	  bLowFrequency = true;
	  if ((!bArgInts) || (aVarArg.num_var < 1))
	    throw bad_peripheral( iPerType );
	  iPeriod = iArgInts[OPT_FREQ_ARG];
	}
    } 
    
    /// Default constructor for Peripheral
    Peripheral(): bDeleted( true ), iClock( 0 ), iState( 0 ),
		  iSumState( 0 ), iCount( 0 )
    {
    }
    
    /// Set peripheral to new value
    /**
     * @param aName name of Peripheral
     * @param anInput is the peripheral an input or an output
     * @param aType the peripheral type
     * @param aVarArg arguments used to create peripheral
     * @exception bad_peripheral thrown if peripheral not currently deleted
     */
    void renew( std::string aName, bool anInput, PeripheralType aType,
		VarArg aVarArg ) throw (bad_peripheral)
    {
      if (!bDeleted)
	throw bad_peripheral( aName );

      INT aBuff[PERIPHERAL_DATA];
      
      iPerType = aType;

      if (Peripheral_robot[ static_cast< int >( aType ) ])
	{
	  if (!low_peripheral_add( (anInput ? TRUE : FALSE), aType, aVarArg,
				   aBuff ) )
	    throw bad_peripheral( iName );
	}
      else
	throw bad_peripheral( iName ); // these are robot peripherals only!
      
      iRef = aBuff[PERIPHERAL_REF]; // robot reference
      bBits = (aBuff[PERIPHERAL_BITS] == FALSE ? false : true );
      // is it sending bits?
      iNumElts = aBuff[PERIPHERAL_NUM_ELTS]; // How many?

      bDeleted = false;
      iName = aName;
      bInput = anInput;
      iClock = 0;
      iPerType = aType;
      iState = 0;
      iSumState = 0;
      iCount = 0;

      bArgInts = static_cast< bool >( aVarArg.is_int );
      if (bArgInts)
	{
	  iArgInts.resize( aVarArg.num_var );
	  for ( int i = 0; i < aVarArg.num_var; i++ )
	    iArgInts[i] = aVarArg.INT_ptr[i];
	}
      else
	iArgString = aVarArg.CHAR_ptr;

      if (Peripheral_lowfreq[ static_cast< int >( iPerType ) ])
	{
	  bLowFrequency = true;
	  if ((!bArgInts) || (aVarArg.num_var < 1))
	    throw bad_peripheral( iPerType );
	  iPeriod = iArgInts[OPT_FREQ_ARG];
	}
      else
	{
	  bLowFrequency = false;
	  iPeriod = 1;
	}

      if ((bBits) && (iNumElts == 1))
	iMessageType = spikeMessage;
      else
	iMessageType = analogueMessage;
    } 


    /// Default destructor for Peripheral
    ~Peripheral()
    {
      if (!bDeleted)
	low_peripheral_remove( (bInput ? TRUE : FALSE), iRef );
      bDeleted = true;
      bLowFrequency = false;
      iNumElts = 0;
    }
    
    /// Delete peripheral
    void destroy()
    {
      if (!bDeleted)
	low_peripheral_remove( (bInput ? TRUE : FALSE), iRef );
      bDeleted = true;
      bLowFrequency = false;
      iNumElts = 0;
    }

    /// Initialise peripheral by resetting internal parameters
    void initialise()
    {
      iState = 0;
      iSumState = 0;
      iCount = 0;
      iClock = 0;
    }

    /// Accessor for whether peripheral has been deleted 
    /**
     * @returns whether Peripheral has been deleted
     */
    const bool isDeleted() { return bDeleted; }
    
    /// Accessor for name
    /**
     * @returns name of peripheral
     */
    const std::string getName() { return iName; }
    
    /// Accessor for robot reference
    /**
     * @returns robot reference
     */
    const int getRef() { return iRef; }
    
    /// Accessor for internal state
    /**
     * @returns current state
     */
    const long getState() { return iState; }
    
    /// Accessor for message type
    /**
     * @returns type of message sent or received
     */
    const MessageType getType() { return iMessageType; }
    
    /// Accessor for input/output
    /**
     * @returns whether peripheral is an input
     * @exception bad_peripheral if deleted
     */
    const bool isInput()
    {
      if (bDeleted)
	throw bad_peripheral();
      return bInput;
    }

    /// Returns value from bit input
    /**
     * extracts bit from input and updates arguments for next sensor
     * @param anInputRef pointer to current char with bits in
     * @param currentMask mask for current bit in that byte
     * @param nextFreeInput pointer to next unused char
     * @returns bool extracted from input, or false if none retrieved
     * @exception bad_peripheral if deleted or not an input, or not a bit
     */
    bool retrieveBit( char*& anInputRef, int& currentMask,
		      char*& nextFreeInput )
    {
      bool aReturn;
      if (bDeleted)
	throw bad_peripheral();
      if ((!bInput) || (iMessageType != spikeMessage))
	throw bad_peripheral( iName );

      if (bLowFrequency)
	{
	  if (anInputRef == NULL)
	    {
	      anInputRef = nextFreeInput++;
	      currentMask = 1;
	    }
	  bool success = ((*anInputRef & currentMask) != 0);
	    
	  currentMask <<= 1;
	  if (currentMask == 256)
	    anInputRef = NULL;
	  
	  if (!success)
	    return false;
	}

      if (anInputRef == NULL)
	{
	  anInputRef = nextFreeInput++;
	  currentMask = 1;
	}
      
      aReturn = (((*anInputRef & currentMask) == currentMask) ?
		 true : false );
      iState = (aReturn ? 1 : 0 );
      iSumState += iState;
      iCount++;

      currentMask <<= 1;
      if (currentMask == 256)
	anInputRef = NULL;

      return aReturn;
    }

    /// Returns value from byte input
    /**
     * extracts byte(s) from input and updates arguments for next sensor
     * @param anInputRef pointer to current char with bits in
     * @param currentMask mask for current bit in that byte
     * @param nextFreeInput pointer to next unused char
     * @param outputVal pointer to value extracted from input
     * @returns whether a value was extracted
     * @exception bad_peripheral if deleted or not an input, or not a byte
     */
    bool retrieveBytes( char*& anInputRef, int& currentMask,
			char*& nextFreeInput, long &outputVal );

    /// Update value for bit output
    /**
     * @param aVal new output value for peripheral
     * @exception bad_peripheral if an input or deleted
     */
    void updateOutput( int aVal = 1 ) throw ( bad_peripheral )
    {
      if (bDeleted)
	throw bad_peripheral();
      if (bInput)
	throw bad_peripheral( iName );

      if (!bLowFrequency)
	{
	  iState = aVal;
	  iSumState += iState;
	  iCount++;
	}
      else
	{
	  if (iClock == 0)
	    iState = aVal;
	  else
	    iState += aVal;
	  
	  iSumState = iState;
	  iCount = iClock + 1;
	}
    }

    /// Returns the number of timesteps till the next output
    /**
     * @returns 1 if not bLowFrequency, otherwise iPeriod - iClock
     */
    int getNextOutput()
    {
      if (bDeleted)
	throw bad_peripheral();

      if (!bLowFrequency)
	return 1;

      return iPeriod - iClock;
    }

    /// Sets value for outputs
    /**
     * inserts bits/bytes into output and updates arguments for next motor
     * @param anOutputRef pointer to current char with bits in
     * @param currentMask mask for current bit in that byte
     * @param nextFreeOutput pointer to next unused char
     * @returns whether there was an output
     * @exception bad_peripheral if deleted or not an output, or not a bit
     */
    bool output( char*& anOutputRef, int& currentMask,
		 char*& nextFreeOutput );
    
    /// Output initialisation script for node
    /**
     * @param anOStream desired output stream
     * @return output stream after output
     */
    std::ostream& externalise( std::ostream& anOStream, 
			       std::string indent = "" );

    /// Output variable names for node
    /**
     * This is pure virtual, because nodes do not have variables
     * @param anOStream desired output stream
     * @return output stream after output
     */
    std::ostream& write( std::ostream& anOStream, std::string aParentName,
			 OutputType aType = outTypeText )
    {
      std::string totalName( aParentName );

      // must be at least one '.' (between root name and parent name)
      std::string::size_type anST = totalName.find( '.' );
      std::string parentName = totalName.substr( anST + 1, totalName.npos );
      std::string aName( parentName );
      anST = aName.find( '.' );
      while (anST != aName.npos)
	{
	  aName.replace( anST, 1, "_" );
	  anST = aName.find( '.' );
	}
      switch ( aType )
	{
	case outTypeText:
	  if (bBits)
	    anOStream << parentName << "." << iName << ".iSpikes\t";
	  else
	    anOStream << parentName << "." << iName << ".iAnalogue\t";
	  break;
	case outTypeMatlab:
	  if (bBits)
	    {
	      anOStream << totalName << "." << iName; 
	      if (bInput)
		anOStream << ".spikinginput = 1;" << std::endl;
	      else
		anOStream << ".spikingoutput = 1;" << std::endl;

	      anOStream << totalName << "." << iName << ".iSpikes = "
			<< aName << "_" << iName << ".iSpikes;" << std::endl;
	    }
	  else
	    {
	      anOStream << totalName << "." << iName; 
	      if (bInput)
		anOStream << ".analogueinput = 1;" << std::endl;
	      else
		anOStream << ".analogueoutput = 1;" << std::endl;
	      anOStream << totalName << "." << iName << ".iAnalogue = "
			<< aName << "_" << iName << ".iAnalogue;" << std::endl;
	    }
	  anOStream << "clear " << aName << "_" << iName << std::endl;
	  break;
	default:
	  std::cerr << "Peripheral::write: Cannot write this output type"
		    << std::endl;
	  break;
	}	  
      return anOStream;
    }

    /// Output current state of node to array
    /**
     * This is used by all the other put methods to gather data
     * @param anArray array of doubles to store data in
     * @param average true to get average since last reading, else current
     * @return pointer to end of used part of array
     */
    double* put( double* anArray, bool average = true )
    {
      double *aPtr = anArray;
      if (!bDeleted)
	{
	  if (average)
	    {
	      if (iCount > 1.5)
		anArray[0] = static_cast< double >( iSumState ) /
		  static_cast< double >( iCount );
	      else
		anArray[0] = static_cast< double >( iState );
	      iSumState = 0;
	      iCount = 0;
	    }
	  else
	    anArray[0] = static_cast< double >( iState );
	  aPtr++;
	}

      return aPtr;
    }

  private:
    /// @var has this peripheral been deleted
    bool bDeleted;
    
    /// @var name of peripheral
    std::string iName;
    
    /// @var is it an input?
    bool bInput;
    
    /// @var is it of low frequency (ie output not every timestep)
    bool bLowFrequency;
    
    /// @var period of output if low frequency
    int iPeriod;

    /// @var internal clock to compare with period if low frequency
    int iClock;

    /// @var iRef the robot reference for the sensor/actuator
    int iRef;

    /// @var type of peripheral
    PeripheralType iPerType;
   
    /// @var does it use bits or bytes?
    bool bBits;
    
    /// @var how many bits/bytes does it require
    int iNumElts;
    
    /// @var type of output
    MessageType iMessageType;
    
    /// @var current state of peripheral
    long iState;

    /// @var sum of current states since last output
    long iSumState;

    /// @var count since last output
    int iCount;

    /// @var Are the Arguments ints?
    bool bArgInts;
    
    /// @var vector of arguments if they are ints
    std::valarray< INT > iArgInts;
    
    /// @var string if argument is a string
    std::string iArgString;
  };
}

#endif//ndef __PERIPHERAL_HH
