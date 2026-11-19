// $Id: Router.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <map>
#include <list>
#include <string>

#include <cstring>

#pragma implementation

#include "Router.hh"
#include "Params.hh"
#include "Message.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void router_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "Network", "enum" );
      }
    catch ( bad_param& bp )
      {
	std::cerr << bp.what() << " already defined - ignoring..." << std::endl;
	already=true;
      }
    
    if (!already)
      {
	try
	  {
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
 * @param aRoot the node whose parameters are to be initialised
 * @exception bad_param thrown if the parameter already exists
 */
void Router::initParams() throw ( bad_param, bad_file )
{
  router_init( gRoot );
}

/// Do one timestep for self only
bool Router::nodeTimestep()
{
  if (!iEvents.empty())
    {
      std::list< std::pair< Node*, Message* > >& aList = iEvents.front();
      
      std::list< std::pair< Node*, Message* > >::iterator anIter
	= aList.begin();
      
      while (anIter != aList.end())
	{
#ifdef DEBUG
	  std::cout << "Sending a message to " << anIter->first->getName() << std::endl;
#endif
	  anIter->first->doReceive( *(anIter->second) );
	  anIter++;
	}
      iEvents.pop_front();
    }
  return true;
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void Router::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a router called " << iName << std::endl;
#endif
}

/// Output initialisation script for router
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& Router::externalise( std::ostream& anOStream, 
				   std::string indent )
{
  bool extras = false;
#ifdef DEBUG
  anOStream << "// Configuring a router called " << iName;
#endif
  if (isRoot())
    {
      std::string aFile = iParams.getInitialisationFile();
//        std::string::size_type anST = aFile.find( PROJDIR );
//        if (anST == 0)
//  	aFile.erase( 0, strlen(PROJDIR) );
      anOStream << "Initialise( \"" << aFile << "\" );"
		<< std::endl << indent;
    }
  anOStream << "Network " << iName 
	    << std::endl << indent << "{";
  indent.push_back( ' ' );
  // output differences...
  if (!isRoot())
    {
      Params& pParams = getRouter()->getParams();
      for ( std::map< std::string, double >::iterator pIter 
	      = iParams.getParamSet().begin();
	    pIter != iParams.getParamSet().end(); pIter++ )
	{
	  if (pIter->second != pParams.getParam( pIter->first ))
	    {
	      if (!extras)
		{
		  extras=true;
		  anOStream << std::endl << indent << "SetParams" 
			    << std::endl << indent << "{";
		  indent.push_back( ' ' );
		  anOStream << std::endl;
		}
	      anOStream << indent;
	      iParams.put( anOStream, pIter->first );
	    }
	}
      if (extras)
	{
	  indent.erase( indent.size()-1 );
	  anOStream << indent << "}";
	}
    }
  else
    {
      for ( std::map< std::string, double >::iterator pIter
	      = iParams.getParamSet().begin();
	    pIter != iParams.getParamSet().end(); pIter++ )
	{
	  if (pIter->second != iParams.getInitialValue( pIter->first ))
	    {
	      if (!extras)
		{
		  extras=true;
		  anOStream << std::endl << indent << "SetParams" 
			    << std::endl << indent << "{";
		  indent.push_back( ' ' );
		  anOStream << std::endl;
		}
	      anOStream << indent;
	      iParams.put( anOStream, pIter->first );
	    }
	}
      if (extras)
	{
	  indent.erase( indent.size()-1 );
	  anOStream << indent << "}";
	}
    }  
  
  for ( std::list< Node* >::const_iterator anIter = iList.begin();
	anIter != iList.end(); anIter++ )
    (*anIter)->externalise( anOStream << std::endl << indent, indent );

  indent.erase( indent.size()-1 );
  anOStream << std::endl << indent << "}";
  return anOStream;
}

/// Output variable names for router
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& Router::write( std::ostream& anOStream,
			     OutputType aType )
{
  if (isRoot())
    {
      switch (aType)
	{
	case outTypeMatlab:
	  anOStream << "global time;" << std::endl
		    << "load " << iName << ";" << std::endl
                    << getName() << ".time = time;" << std::endl
		    << getName() << ".size = size( " << getName()
		    << ".time, 2 );" << std::endl;
	  break;
	case outTypeText:
	  anOStream << "time\t";
	  break;
	default:
	  std::cerr << "Router::write: Cannot write this output type"
		    << std::endl;
	  break;
	}
    }

  for ( std::list< Node* >::const_iterator anIter = iList.begin();
	anIter != iList.end(); anIter++ )
    {
      (*anIter)->write( anOStream, aType );
    }

  if (isRoot())
    {
      switch (aType)
	{
	case outTypeMatlab:
	  anOStream << "clear time" << std::endl
		    << "save " << getName() << " " << getName() << std::endl;
	  break;
        default:
          break;
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
double* Router::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  for ( std::list< Node* >::const_iterator anIter = iList.begin();
	anIter != iList.end(); anIter++ )
    {
      thePtrPos = (*anIter)->put( thePtrPos, average );
    }
  return thePtrPos;
}
