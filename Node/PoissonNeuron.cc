// $Id: PoissonNeuron.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <string>
#include <iostream>
#include <cstdio>

#include <cmath>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#pragma implementation
#include "Neuron.hh"
#include "PoissonNeuron.hh"
#include "Params.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void pneuron_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    Neuron::initParams();
    
    // Using random numbers: must initialise
    int fd = open("/dev/urandom", O_RDONLY);
    if (fd==-1)
      {
	perror("Poisson neuron initialise: opening random source");
	throw bad_file("/dev/urandom");
      }
    unsigned int seed=0;
    ssize_t retval = read(fd, (void *)&seed, sizeof(unsigned int));
    if (retval<0)
      perror("Poisson neuron initialise: reading random source");
    else if (retval==0)
      std::cout << "Got nothing..." << std::endl;
    else while (retval < (ssize_t) sizeof(unsigned int))
      retval += read(fd, (void *)seed, sizeof(unsigned int)-retval);
    srand(seed);
    close(fd);

    try
      {
	aParams.addParam( "Poisson", "enum" );
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
	    aParams.addParam( "noSpike", "units" );
	    aParams.addParam( "sureSpike", "units" );
	    aParams.addParam( "spikeProb", "prob" );
	    aParams.addParam( "spikeProbFrac", "/ms" );
	    aParams.addParam( "spikeProbT", "ms" );
	    //aParams.addParam( "refractT", "ms" );
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
void PoissonNeuron::initParams() throw ( bad_param, bad_file )
{
  pneuron_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void PoissonNeuron::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a neuron called " << iName << std::endl;
#endif

  // Set name for spike message
  iMessage.newSource( iName );

  // set max and min
  iMin = iParams.getParam( "noSpike" );
  iWidth = iParams.getParam( "sureSpike" ) - iMin;


  // Refractory period is number of timesteps since last spike
  iTRefract = static_cast< int >( rint( iParams.getParam( "refractT" )
					/ iParams.getParam( "stepT" ) ) );

  // PoissonNeuron is initially not refractory
  iAxon = iTRefract >? 1;

  // No spikes at start
  iSpikes = 0;

  // Initial internal state
  iState = -iMin;

  // No timesteps since last output
  iCount = 0.0;
  iSumAxon = 0.0;

  // Set spiking prob to base
  iProbBase = iParams.getParam( "spikeProb" );
  iProb = iProbBase;
  // scaled prob of spiking decrease
  iProbFrac = iParams.getParam( "spikeProbFrac" );
  // and associated decay
  iTProb = exp( log( 0.5 ) * iParams.getParam( "stepT" ) /
		iParams.getParam( "spikeProbT" ) );

  // No incoming spike
  bSpike=false;
}

/// Do one timestep for self only
bool PoissonNeuron::nodeTimestep()
{
  if ((iTRefract <= iAxon) && 
      (((rand() * iWidth)/(RAND_MAX + 1.0) < iState) ||
       (bSpike && (rand() < iProb*(RAND_MAX + 1.0)))))
    {
      iParent->doReceive( iMessage );
      iSpikes++;
      iAxon = 0;
      // Update spiking probability
      if (bSpike)
	iProb *= iProbFrac;
    }
  else
    {
      // Check whether still in refractory period after spike
      if ((iTRefract > iAxon) || (iAxon == 0))
	iAxon++;
    }
  
      // Update spiking probability
//        if (bSpike)
//  	iProb *= iProbFrac;
  iProb -= iProbBase;
  iProb *= iTProb;
  iProb += iProbBase;
  bSpike = false;

  iState = -iMin;

  // output update
  iSumAxon += iAxon;
  iCount++;
  return true;
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& PoissonNeuron::externalise( std::ostream& anOStream, 
					  std::string indent )
{
  bool extras = false;
  Params& pParams = getRouter()->getParams();

#ifdef DEBUG
  anOStream << "// Configuring a poisson neuron called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Neuron " << iName << " Poisson";

  if (!iTarget.empty())
    anOStream << " targets " << iTarget;
  
  if (iParams.getParam( "noSpike" ) != pParams.getParam( "noSpike" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "noSpike" );
    }

  if (iParams.getParam( "sureSpike" ) != pParams.getParam( "sureSpike" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "sureSpike" );
    }

  if (iParams.getParam( "spikeProb" ) != pParams.getParam( "spikeProb" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "spikeProb" );
    }

  if (iParams.getParam( "spikeProbFrac" ) !=
      pParams.getParam( "spikeProbFrac" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "spikeProbFrac" );
    }

  if (iParams.getParam( "spikeProbT" ) != pParams.getParam( "spikeProbT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "spikeProbT" );
    }

  if (iParams.getParam( "refractT" ) != pParams.getParam( "refractT" ))
    {
      do_params( extras, anOStream, indent );
      anOStream << indent;
      iParams.put( anOStream, "refractT" );
    }

  if (extras)
    {
      indent.erase( indent.size()-1 );
      anOStream << indent << "}";
    }
  else
    anOStream << ";";
  
  return anOStream;
}

/// Output variable names for neuron
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& PoissonNeuron::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputSpike ))
	anOStream << myName << ".iSpikes\t";
      if (gOutputLevel.test( outputAxon ))
	anOStream << myName << ".iAxon\t";
      break;
    case outTypeMatlab:
      if ((gOutputLevel.test( outputSpike )) ||
	  (gOutputLevel.test( outputAxon )))
	{
	  anOStream << aName << ".poissonneuron = 1;" << std::endl;
	  anOStream << aName << ".refractT = "
		    << iParams.getParam( "refractT" ) << ";" << std::endl;
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
double* PoissonNeuron::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = iSpikes/iCount;
	    }
	  if (gOutputLevel.test( outputAxon ))
	    {
	      *(thePtrPos++) = iSumAxon/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputSpike ))
	    {
	      *(thePtrPos++) = (iAxon == 0 ? 1.0 : 0.0);
	    }
	  if (gOutputLevel.test( outputAxon ))
	    {
	      *(thePtrPos++) = static_cast< double >( iAxon );
	    }
	}      
      iCount = 0;
      iSpikes = 0;
      iSumAxon = 0;
    }
  else
    {
      if (gOutputLevel.test( outputSpike ))
	{
	  *(thePtrPos++) = (iAxon == 0 ? 1.0 : 0.0);
	}
      if (gOutputLevel.test( outputAxon ))
	{
	  *(thePtrPos++) = static_cast< double >( iAxon );
	}
    }      
  return thePtrPos;
}
