// $Id: FileInput.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <fstream>
#include <iostream>
#include <map>
#include <list>
#include <string>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <cstring>
#pragma implementation
#include "FileInput.hh"
#include "Params.hh"
#include "Message.hh"

using namespace Runtime;

namespace 
{
  // This function does the initialisation
  void fileinput_init( Node* aRoot ) throw ( bad_param, bad_file )
  {
    bool already=false;
    Params& aParams=aRoot->getParams();
    
    try
      {
	aParams.addParam( "file", "enum" );
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
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
  }
}

/// remove all external links
/**
 * Kills branches, then kills all remote links - an essential precursor
 * to calling delete
 * @param aName name of node to be stripped - usually self
 * @return whether node can be deleted
 */
bool FileInput::removeAll( std::string aName ) throw ( bad_node )
{
  bool deleteMe = false;
  if ((aName == "") || (matchName( aName, false )))
    {
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  while (iNumConns[i] != 0)
	    {
	      RemoveConnectionMessage rMess( iName + "." + iNodeNames[i],
					     iTargets[i].front(), true );
	      iParent->doReceive( rMess );
	      removeConnection( iName + "." + iNodeNames[i],
				iTargets[i].front());
	    }
	}
      deleteMe = true;
    }
  else if (!matchName( aName, true ))
    throw bad_node( aName );
  else
    {
      bool found = false;
      std::string strippedName = stripName( aName );
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  if (iNodeNames[i] == strippedName)
	    {
	      while (iNumConns[i] != 0)
		{
		  RemoveConnectionMessage rMess( iName + 
						 "." + iNodeNames[i],
						 iTargets[i].front(),
						 true );
		  iParent->doReceive( rMess );
		  removeConnection( iName + "." + iNodeNames[i],
				    iTargets[i].front());
		}
	      found = true;
	    }
	}
      if (!found)
	throw bad_node( aName );
    }
  return deleteMe;
}

/// Get node by name from router
/**
 * @param aName name of node
 * @return pointer to node
 * @exception bad_node thrown if name not used
 */
Node* FileInput::getNode( std::string aName ) throw ( bad_node )
{
  if (!matchName( aName, true ))
    throw bad_node( aName );

  std::string aChild = stripName( aName );
  for ( unsigned int anEntry = 0; anEntry < iNumEntries; anEntry++ )
    if (iNodeNames[anEntry] == aChild)
      return this;

  throw bad_node( aName );
  return NULL;
}

/// Receives and processes message
/**
 * @param aMessage
 * @exception bad_message thrown if inappropriate message is sent
 */
void FileInput::doReceive( Message& aMessage )
{
  NewConnectionMessage* nc;
  RemoveConnectionMessage* rc;
  unsigned int theSource;
  std::string aStr;
  
  switch (aMessage.getType())
    {
    case noMessage:
      break;
    case newConnectionMessage:
      nc = dynamic_cast< NewConnectionMessage* >( &aMessage );
      //#ifdef DEBUG
      std::cout << "New connection from " << nc->getSource()
	   << " to " << nc->getDestination() << " received by "
	   << getName( true ) << std::endl;
      //#endif //def DEBUG
      if (nc->fromSource())
	{
	  std::cerr << "Connection arriving at FileInput aimed at child" << std::endl;
	  throw bad_message( aMessage.getType() );
	}

      if (matchName( nc->getSource() ))
	{
	  std::cerr << "Connection to FileInput itself not allowed" << std::endl;
	  throw bad_message( aMessage.getType() );
	}

      if (!matchName( nc->getSource(), true ))
	{
	  throw bad_message( aMessage.getType() );
	}
      theSource = iNumEntries;
      aStr = stripName( nc->getSource() );
      for (unsigned int i = 0; i < iNumEntries; i++)
	{
	  if ((iNodeNames[i] == aStr) && (iTypes[i] != noMessage))
	    {
	      if (theSource != iNumEntries)
		{
		  if (iTypes[i] == spikeMessage)
		    {
		      if ((iTypes[theSource] != spikeMessage) ||
			  (iNames[i].substr( iNodeNames[i].size() + 1,
					     iNames[i].npos )
			   == "iSpikes"))
			{
			  theSource = i;
			}
		    }
		}
	      else // theSource == iNumEntries
		theSource = i;
	    }
	}
      if (theSource == iNumEntries)
	throw bad_node( nc->getSource() );
      iNumConns[theSource]++;
      iTargets[theSource].push_back(nc->getDestination());
      if (iNumConns[theSource] == 1)
	{
	  switch ( iTypes[theSource] )
	    {
	    case spikeMessage:
	      iMessages[ theSource ] 
		= new SpikeMessage( nc->getSource() );
	      break;
	    case analogueMessage:
	      iMessages[ theSource ] 
		= new AnalogueMessage( nc->getSource() );
	      break;
	    case conductanceMessage:
	      iMessages[ theSource ] 
		= new ConductanceMessage( nc->getSource() );
	      break;
	    case currentMessage:
	      iMessages[ theSource ] 
		= new CurrentMessage( nc->getSource() );
	      break;
	    default:
	      throw bad_message( iTypes[theSource] );
	    }
	}
      break;
    case removeConnectionMessage:
      rc = dynamic_cast< RemoveConnectionMessage* >( &aMessage );
#ifdef DEBUG
      std::cout << "Remove connection from " << nc->getSource()
	   << " to " << nc->getDestination() << " received by "
	   << getName( true ) << std::endl;
#endif //def DEBUG
      if (rc->fromSource())
	{
	  std::cerr << "Connection arriving at FileInput aimed at child" << std::endl;
	  throw bad_message( aMessage.getType() );
	}
      removeConnection( rc->getSource(), rc->getDestination() );
      break;	
    default:
      throw bad_message( aMessage.getType() );
      break;
    }
}


/// Initialise parameter names for this node type
/**
 * @param aRoot the node whose parameters are to be initialised
 * @exception bad_param thrown if the parameter already exists
 */
void FileInput::initParams() throw ( bad_param, bad_file )
{
  fileinput_init( gRoot );
}

/// Removes an existing connection controlled by the fileinput
/**
 * @param aSource the branch the message came from
 * @param aDestination the node which the message went to
 * @exception bad_node if the connection or the recipient
 * does not exist
 */
void FileInput::removeConnection( std::string aSource,
				  std::string aDestination )
  throw ( bad_node )
{
  if (matchName( aSource ))
    {
      std::cerr << "Connection to FileInput should not exist!" << std::endl;
      return;
    }
  
  if (!matchName( aSource, true ))
    {
      std::cerr << "Can't find the node: " << aSource << std::endl;
      throw bad_node( aSource );
    }
  
  std::string aStr = stripName( aSource );
  unsigned int theSource = iNumEntries;
  for (unsigned int i = 0; i < iNumEntries; i++)
    {
      if ((iNodeNames[i] == aStr) && (iTypes[i] != noMessage))
	{
	  if (theSource != iNumEntries)
	    {
	      if (iTypes[i] == spikeMessage)
		{
		  if ((iTypes[theSource] != spikeMessage) ||
		      (iNames[i].substr( iNodeNames[i].size() + 1,
					 iNames[i].npos )
		       == "iSpikes"))
		    {
		      theSource = i;
		    }
		}
	    }
	  else // theSource == iNumEntries
	    theSource = i;
	}
    }

  if (theSource == iNumEntries)
    throw bad_node( aSource );

  if (iNumConns[theSource] == 0)
    throw bad_node( aSource );

  iNumConns[theSource]--;
  bool found = false;
  for ( std::list< std::string >::iterator anIter = iTargets[theSource].begin();
	anIter != iTargets[theSource].end(); anIter++ )
    {
      if ((*anIter) == aDestination)
	{
	  std::list< std::string >::iterator a = anIter;
	  anIter--;
	  iTargets[theSource].erase( a );
	  found = true;
	}
    }
  
  if (!found)
    throw bad_node( aDestination );

  if (iNumConns[theSource] == 0)
    {
      delete iMessages[theSource];
      iMessages.erase( iMessages.find( theSource ) );
      iTargets.erase( iTargets.find( theSource ) );
    }
  
}

/// Do one timestep for self only
bool FileInput::nodeTimestep()
{
  bool doContinue = true;
  if (iBinary)
    {
      size_t aSize = 0;
      ssize_t aVal = 100;
      double aBuff[BUFF_MAX];
      while ((aSize < iNumEntries * sizeof( double )) && (aVal != 0))
	{
	  aVal = ::read( iFD, (void *)aBuff, iNumEntries * sizeof( double ) );
	  aSize += aVal;
	}
      if (aVal == 0)
	doContinue = false;
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  iCurrent[i] = aBuff[i];
	}
    }
  else
    {
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  iFileStream >> iCurrent[i];
	}
    }
  iCount++;
  iSum += iCurrent;
  // get input first...
  std::map< int, Message* >::iterator anIter;
  for ( anIter = iMessages.begin(); anIter != iMessages.end(); anIter++ )
    {
      switch (anIter->second->getType())
	{
	case spikeMessage:
	  if (iCurrent[anIter->first] > 0.5) // a spike = 1, not = 0
	    {
	      iParent->doReceive( *(anIter->second) );
	    }
	  break;
	case analogueMessage:
	  iParent->doReceive( dynamic_cast< AnalogueMessage* >
			      ( anIter->second )
			      ->newAnalogue( iCurrent[anIter->first] ) );
	  break;
	default:
	  std::cerr << "bad message type in FileInput::nodeTimestep" << std::endl;
	  throw bad_message( anIter->second->getType() );
	  break;
	}
    }
  return doContinue;
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void FileInput::initialise()
{
#ifdef DEBUG
  std::cout << "Initialising a FileInput called " << iName << std::endl;
#endif

  std::string aNames( iFilename );
  std::string aBin( iFilename );
  std::string aText( iFilename );
  aNames += ".names";
  aBin += ".bin";
  aText += ".txt";

  std::ifstream nameStream;

  if (iFileStream.is_open())
    {
      iFileStream.close();
    }
  iFileStream.clear();

  nameStream.open( aNames.c_str() );
  if (!nameStream)
    {
      if (nameStream.is_open())
	nameStream.close();
      nameStream.clear();
      throw bad_file( aNames );
    }

  iBinary = true;
  iFD = open( aBin.c_str(), O_RDONLY );
  if (iFD == -1)
    {
      if (errno != ENOENT)
	{
	  throw bad_file( aBin );
	}
      iBinary = false;
      iFileStream.open( aText.c_str() );
      // Did that work?
      if (!iFileStream)
	{
	  if (iFileStream.is_open())
	    iFileStream.close();
	  iFileStream.clear();
	  throw bad_file( aText );
	}
    }
  
  std::string aName;
  std::string anEnding;
  std::string::size_type anST;
  iNumEntries = 0;
  iTypes.clear();
  iNodeNames.clear();
  getline( nameStream, aName, '\t' );
  while (!nameStream.eof())
    {
      if (iNames.size() <= iNumEntries)
	{
	  iNames.resize( iNumEntries + 1 );
	  iNames[iNumEntries] = aName;
	}
      else if (iNames[iNumEntries] != aName)
	throw bad_file( aNames );
      anST = aName.rfind( '.' );
      iNodeNames.push_back( aName.substr( 0, anST ) );
      anEnding = aName.substr( anST + 1, aName.npos );
      if ((anEnding == "iSpikes") || (anEnding == "iAxon"))
	iTypes.push_back( spikeMessage );
      else if ((anEnding == "iVActual") || (anEnding == "iAnalogue"))
	iTypes.push_back( analogueMessage );
      else if (anEnding == "iGActual_tc") 
	iTypes.push_back( conductanceMessage );
      else if (anEnding == "iIActual_tc")
	iTypes.push_back( currentMessage );
      else
	iTypes.push_back( noMessage );

      if (iNumConns.size() <= iNumEntries)
	iNumConns.resize( iNumEntries + 1, 0 );
      iNumEntries++;
      getline( nameStream, aName, '\t' );
    }

  iCurrent.resize( iNumEntries );
  iSum.resize( iNumEntries );

  iCurrent = 0;
  iSum = 0;
  iCount = 0;
  if (iBinary)
    {
      size_t aSize = 0;
      ssize_t aVal = 100;
      double aBuff[BUFF_MAX];
      while ((aSize < iNumEntries * sizeof( double )) && (aVal != 0))
	{
	  aVal = ::read( iFD, (void *)aBuff, iNumEntries * sizeof( double ) );
	  aSize += aVal;
	}
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  iCurrent[i] = aBuff[i];
	}
    }
  else
    {
      for ( unsigned int i = 0; i < iNumEntries; i++ )
	{
	  iFileStream >> iCurrent[i];
	}
    }
}

/// Output initialisation script for fileinput
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream&  FileInput::externalise( std::ostream& anOStream, 
				       std::string indent )
{
#ifdef DEBUG
  anOStream << "// Configuring a fileinput called " << iName;
#endif
  anOStream << "add input file " << iName << "( \"" 
	    << iFilename << "\" );" << std::endl << indent;
  for ( unsigned int i = 0; i < iNumEntries; i++ )
    {
      switch (iTypes[i])
	{
	case spikeMessage:
	  anOStream << "// spike source: " << iNodeNames[i]
		    << (iNumConns[i] > 0? " (used)": " (unused)") 
		    << std::endl << indent;
	  break;
	case analogueMessage:
	  if ((i + 1 == iNumEntries) || 
	      (iNodeNames[i] != iNodeNames[i + 1]) ||
	      ((iTypes[i + 1] != analogueMessage) &&
	       (iTypes[i + 1] != spikeMessage)))

	    {
	      anOStream << "// analogue source: " << iNodeNames[i]
			<< (iNumConns[i] > 0? " (used)": " (unused)") 
			<< std::endl << indent;
	    }
	  break;
	default:
	  break;
	}
    }

  return anOStream;
}

/// Output variable names for fileinput
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream&  FileInput::write( std::ostream& anOStream,
				 OutputType aType )
{
  std::string aName = getName( true );
  std::string myName = aName + '.';
  std::string totalName = getRoot()->getName() + '.';
  std::string fullName;
  std::string::size_type last;
  std::string::size_type anST;

  std::map< int, Message* >::iterator anIter;
  switch (aType)
    {
    case outTypeText:
      if (gOutputLevel.test( outputInput ))
	for ( anIter = iMessages.begin(); anIter != iMessages.end(); anIter++ )
	  anOStream << myName << iNames[ anIter->first ] << "\t";
      break;
    case outTypeMatlab:
      if (gOutputLevel.test( outputInput ))
	for ( anIter = iMessages.begin(); anIter != iMessages.end(); anIter++ )
	  {
	    fullName = myName + iNames[ anIter->first ];
	    aName = fullName;
	    last = fullName.rfind( '.' );
	    anST = aName.find( '.' );
	    while (anST != last)
	      {
		fullName.replace( anST, 1, "_" );
		anST = fullName.find( '.' );
	      }

	    switch (anIter->second->getType())
	      {
	      case spikeMessage:
		anOStream << aName.substr(0, last) 
			  << ".spikinginput = 1;" << std::endl;
		break;
	      case analogueMessage:
		anOStream << aName.substr(0, last) 
			  << ".analogueinput = 1;" << std::endl;
		break;
	      default:
		std::cerr << "bad message type in FileInput::write" << std::endl;
		throw bad_message( anIter->second->getType() );
		break;
	      }
	    //anOStream <<totalName << aName << " = " << fullName << std::endl;
	    anOStream << totalName << fullName.substr(0, last) 
		      << " = node_convert( " << aName.substr(0, last)
		      << " );" << std::endl;
	    anOStream << "clear " << aName.substr(0, last) << std::endl;
	  }
      break;
	default:
	  std::cerr << "FileInput::write: Cannot write this output type"
		    << std::endl;
	  break;
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
double*  FileInput::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;
  std::map< int, Message* >::iterator anIter;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputInput ))
	    {
	      for ( anIter = iMessages.begin(); anIter != iMessages.end();
		    anIter++ )
		{
		  *(thePtrPos++) = iSum[ anIter->first ]/iCount;
		}
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputInput ))
	    {
	      for ( anIter = iMessages.begin(); anIter != iMessages.end();
		    anIter++ )
		{
		  *(thePtrPos++) = iCurrent[ anIter->first ];
		}
	    }
	}
      iCount = 0;
      iSum = 0;
    }
  else
    if (gOutputLevel.test( outputInput ))
      {
	for ( anIter = iMessages.begin(); anIter != iMessages.end();
	      anIter++ )
	  {
	    *(thePtrPos++) = iCurrent[ anIter->first ];
	  }
      }
  return thePtrPos;
}
