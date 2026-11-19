// $Id: GeneratedInput.cc,v 1.2 2003/12/06 00:07:34 darrens Exp $

#include <iostream>
#include <string>

#include <cmath>
#pragma implementation
#include "GeneratedInput.hh"
#include "Params.hh"

using namespace Runtime;

namespace
{
  // This function does the initialisation
  void ginput_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();

    try
      {
        aParams.addParam( "GeneratedInput", "enum" );
      }
    catch ( bad_param& bp )
      {
        already=true;
        std::cerr << bp.what() << " already defined!" << std::endl;
      }

    if (!already)
      {
        try
          {
            aParams.addParam( "constant", "enum" );
            aParams.addParam( "random", "enum" );
            aParams.addParam( "interval", "enum" );
            aParams.addParam( "latch", "enum" );
          }
        catch ( bad_param& bp )
          {
            std::cerr << bp.what() << " already defined!" << std::endl;
            throw;
          }
      }
  }
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void GeneratedInput::initParams() throw ( bad_param, bad_file )
{
  ginput_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void GeneratedInput::initialise() throw ( bad_peripheral )
{
#ifdef DEBUG
  std::cout << "Initialising a generated input called " << iName << std::endl;
#endif

  // Set name for spike message
  iMessage.newSource( iName );

  switch (iType)
    {
    case INPUT_LATCH:
      iLatch_isUp = false;
      iSpike = false;
      break;
    case INPUT_RANDOM:
    case INPUT_CONSTANT:
      break;
    case INPUT_INTERVAL:
      iSteps = 1.0/iParams.getParam( "stepT" );
      iSyllable = 0;
      bInSyll = true;
      iPos = 0;
      break;
    default:
      std::cerr << "GeneratedInput::initialise: illegal input type"
                << std::endl;
      throw bad_peripheral( iType );
      break;
    }
  iState = 0.0;
  iSumState = 0.0;
  iCount = 0.0;
}

/// Do one timestep for self only
bool GeneratedInput::nodeTimestep()
{
  bool bFinished = false;

  switch (iType)
    {
    case INPUT_RANDOM:
      iState = iArgs[0] + ((rand() * (iArgs[1] - iArgs[0]))/(RAND_MAX + 1.0));
      break;
    case INPUT_CONSTANT:
      iState = iArgs[0];
      break;
    case INPUT_LATCH:
      if (iSpike)
        {
          iLatch_isUp = !iLatch_isUp;
          iSpike = false;
        }
      iState = (iLatch_isUp)? iArgs[1] : iArgs[0];
      break;
    case INPUT_INTERVAL:
      while (!bFinished)
        {
          if (iSyllable < iArgs[4])
            {
              if (bInSyll)
                {
                  if (++iPos < iArgs[1] * iSteps + 0.5)
                    {
                      iState = iArgs[0];
                      bFinished = true;
                    }
                  else
                    {
                      bInSyll = false;
                      iPos = 0;
                    }
                }
              if (!bInSyll)
                {
                  if (++iPos < iArgs[3] * iSteps + 0.5)
                    {
                      iState = iArgs[2];
                      bFinished = true;
                    }
                  else
                    {
                      iSyllable++;
                      bInSyll = true;
                      iPos = 0;
                    }
                }
            }
          else
            {
              if (++iPos < iArgs[5] * iSteps + 0.5)
                {
                  iState = iArgs[2];
                  bFinished = true;
                }
              else
                {
                  iSyllable = 0;
                  bInSyll = true;
                  iPos = 0;
                }
            }
        }
      break;
    default:
      std::cerr << "GeneratedInput::nodeTimestep: illegal input type"
                << std::endl;
      //      throw bad_peripheral( iType );
      break;
    }
  iParent->doReceive( iMessage.newAnalogue( iState ) );
  iSumState += iState;
  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& GeneratedInput::externalise( std::ostream& anOStream,
					   std::string indent )
{
#ifdef DEBUG
  anOStream << "// Configuring a generated input called " << iName
            << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Input ";

  switch (iType)
    {
    case INPUT_RANDOM:
      anOStream << "random " << iName << "( " << iArgs[0] << ", "
                << iArgs[1] << " );";
      break;
    case INPUT_LATCH:
      anOStream << "latch " << iName << "( " << iArgs[0] << ", "
                << iArgs[1] << " );";
      break;
    case INPUT_CONSTANT:
      anOStream << "constant " << iName << "( " << iArgs[0] << " );";
      break;
    case INPUT_INTERVAL:
      anOStream << "interval " << iName << "( " << iArgs[0] << ", "
                << iArgs[1] << ", " << iArgs[2] << ", "
                << iArgs[3] << ", " << iArgs[4] << ", "
                << iArgs[5] << " );";
      break;
    default:
      std::cerr << "GeneratedInput::externalise: illegal input type"
                << std::endl;
      //      throw bad_peripheral( iType );
      break;
    }


  return anOStream;
}

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& GeneratedInput::write( std::ostream& anOStream,
				     OutputType aType )
{
  std::string aName = getName( true );
  std::string myName = aName;
  std::string totalName = getRoot()->getName() + '.' + aName;
  std::string::size_type anST = aName.find( '.' );
  while (anST != aName.npos)
    {
      aName.replace( anST, 1, "_" );
      anST = aName.find( '.' );
    }

  switch ( aType )
    {
    case outTypeText:
      if (gOutputLevel.test( outputInput ))
        anOStream << myName << ".iAnalogue\t";
      break;
    case outTypeMatlab:
      if (gOutputLevel.test( outputInput ))
        {
          anOStream << aName << ".generatedinput = 1;" << std::endl;
          anOStream << totalName << " = node_convert( "
                    << aName << " );" << std::endl;
          anOStream << "clear " << aName << std::endl;
        }
      break;
    default:
      std::cerr << "Unknown output type" << std::endl;
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
double* GeneratedInput::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
        {
          if (gOutputLevel.test( outputInput ))
            {
              *(thePtrPos++) = iSumState/iCount;
            }
        }
      else
        {
          if (gOutputLevel.test( outputInput ))
            {
              *(thePtrPos++) = iState;
            }
        }

      iCount = 0;
      iSumState = 0;
    }
  else
    {
      if (gOutputLevel.test( outputInput ))
        {
          *(thePtrPos++) = iState;
        }
    }
  return thePtrPos;
}
