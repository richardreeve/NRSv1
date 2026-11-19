// $Id: helper.cc,v 1.2 2003/12/18 17:23:06 darrens Exp $

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <iterator>
#include <cstdio>
#include <iomanip>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "base.h"

int main( int argc, char** argv )
{
  std::list< char* > theInstructions;
  bool failed = false;
  bool first = true;

  theInstructions.push_back( "\n"
			     "nodeHelper.linux: "
			     "a decoder and general tool "
			     "for node outputs\n"
 			     "----------------\n" );

  while ((argc > 1) && (!failed))
    {
      failed = true;
      if (first)
	theInstructions.push_back( " [ ( -t | --text ) <root> [ - ] ]: "
				   "decode binary file <root>.bin\n"
				   "                                   "
				   "to text file <root>.txt or stdout\n" );
      if ((argc > 2) &&
	  ((!strcmp(argv[1], "--text")) || (!strcmp(argv[1], "-t"))))
	{ 
	  failed = false;
	  std::string theRoot( argv[2] );
	  std::string theBinary = theRoot + ".bin";
	  std::string theText = theRoot + ".txt";
	  std::string theNames = theRoot + ".names";
	  int numVars = 0;
	  double *theBuffer;
	  int theFD;
	  std::ostream *textStream;
	  std::ifstream nameStream;
	  std::string aName;

	  argc -= 2;
	  argv += 2;

	  nameStream.open( theNames.c_str() );
	  if (nameStream.fail())
	    {
	      std::cerr << "Failed to open name file: " << theNames << std::endl;
	      exit( 1 );
	    }

	  getline( nameStream, aName, '\t' );
	  while (!nameStream.eof())
	    {
	      numVars++;
	      getline( nameStream, aName, '\t' );
	    }
	  nameStream.close();
	  theBuffer = new double[numVars];

	  if ((argc > 1) && (!strcmp(argv[1], "-")))
	    {
	      textStream = &std::cout;
	      argc--;
	      argv++;
	    }
	  else
	    {
	      textStream = 
		dynamic_cast< std::ostream* >( new std::ofstream( theText.c_str() ) );
	    }

	  theFD = open( theBinary.c_str(), O_RDONLY );
	  if ( theFD == -1 )
	    {
	      std::cerr << "Bad file: " << theBinary.c_str() << std::endl;
	      exit( 1 );
	    }
	  
	  int retval = 
	    read( theFD, (void *)theBuffer, numVars * sizeof( double ) );
	  std::ostream_iterator< double > theOSIter( (*textStream), "\t" );

          /* Set output format */
          //          textStream->setf(std::ios::fixed);
          (*textStream) << std::setprecision(10);   // C++ default is 6
	  
	  while (retval == static_cast< int >( numVars * sizeof( double ) ))
	    {
	      for (int i = 0; i < numVars; i++)
		{
		  *(theOSIter++) = theBuffer[i];
		}
	      (*textStream) << "\n";
	      retval = read( theFD, (void *)theBuffer, 
			     numVars * sizeof( double ) );
	    }
	  
	  close( theFD );
	  
	  if (textStream != &std::cout)
	    {
	      (*textStream) << std::flush;
	      dynamic_cast< std::ofstream* >( textStream )->close();
	      delete textStream;
	    }
	  
	  delete[] theBuffer;
	}
      
      // use bzip2 to compress data file
      if (first)
	theInstructions.push_back( " [ ( -z | --zip ) <root> ]: "
				   "tar and bzip experiment <root>\n"
				   "                            "
				   "to file <root>.tbz2\n" );
      if ((argc > 2) &&
	  ((!strcmp(argv[1], "--zip")) || (!strcmp(argv[1], "-z"))))
	{
	  failed = false;
	  char aCommand[BUFF_MAX];

	  argc -= 2;
	  argv += 2;

	  sprintf( aCommand, "rm -f %s.tbz2; tar cf - %s.* | bzip2 > %s.tbz2",
		   argv[2], argv[2], argv[2] );
	  system( aCommand );
	}

      // create a matlab file
      if (first)
	theInstructions.push_back( " [ ( -m | --matlab ) <root> ]: "
				   "encode binary file <root>.bin\n"
				   "                               "
				   "as matlab file <root>.mat\n" );
      if ((argc > 2) &&
	  ((!strcmp(argv[1], "--matlab")) || (!strcmp(argv[1], "-m"))))
	{
	  failed = false;
	  std::string theRoot( argv[2] );
	  std::string theNames = theRoot + ".names";
	  std::string theBinary = theRoot + ".bin";
	  std::string theMatlab = theRoot + ".mat";
	  std::ifstream nameStream;
	  double *aDoubleBuffer = NULL;
	  int numVars = 0;
	  struct stat statBuf;
	  unsigned long totalLength;
	  long numSteps;
	  unsigned int aValue;
	  unsigned int anIntBuffer[BUFF_MAX];
	  char aCharBuffer[BUFF_MAX];
	  std::string aName;
	  std::list< std::string > aList;
	  std::map< std::string, bool > aStructMap;
	  std::map< std::string, std::list< std::string > > anElementMap;
	  std::map< std::string, int > aLengthMap;
	  std::map< std::string, int > anOrderMap;
	  int iFD;
	  int oFD;

	  argc -= 2;
	  argv += 2;

	  int retval = ::stat( theBinary.c_str(), &statBuf );
	  if (retval == -1)
	    {
	      std::cerr << "Failure stating: " << theBinary << std::endl;
	      perror( "" );
	      exit( 1 );
	    }
	  totalLength = static_cast< unsigned long >( statBuf.st_size );

	  nameStream.open( theNames.c_str() );
	  if (nameStream.fail())
	    {
	      std::cerr << "Failed to open name file: " << theNames << std::endl;
	      exit( 1 );
	    }

	  getline( nameStream, aName, '\t' );
	  while (!nameStream.eof())
	    {
	      std::string::size_type last = aName.rfind( '.' );
	      std::string::size_type anST = aName.find( '.' );
	      while (anST != last)
		{
		  aName.replace( anST, 1, "_" );
		  anST = aName.find( '.' );
		}
	      aList.push_back( aName );
	      anOrderMap[aName] = numVars;
	      numVars++;
	      getline( nameStream, aName, '\t' );
	    }

	  aDoubleBuffer = new double[numVars];
	  numSteps = totalLength / ( numVars * sizeof( double ) );
//  	  std::cerr << numVars << " x " << numSteps << " x sizeof( double ) = " 
//  	       << totalLength << std::endl;

	  if (numSteps * numVars * sizeof( double ) != totalLength)
	    {
	      std::cerr << numVars << " x " << numSteps << " x sizeof( double ) != "
		   << totalLength << " (actually " 
		   << numSteps * numVars * sizeof( double )
		   << ")" << std::endl;
	      exit( 1 );
	    }

	  for ( std::list< std::string >::iterator listIter = aList.begin();
		listIter != aList.end(); listIter++ )
	    {
	      aName = *listIter;
	      unsigned int aPos = aName.find( '.' );
	      if (aPos == std::string::npos)
		{
		  // no sub-elements (probably time)
		  aStructMap[aName] = false;
		  aLengthMap[aName] = 56 + numSteps * sizeof( double ) +
		    ( static_cast< int >( aName.length() ) > 4 ?
		      (((static_cast< int >( aName.length() ) - 1) 
			/ sizeof( double )) * sizeof( double )) :
		      -sizeof( double ));
//  		  std::cerr << "Length of " << aName << " is: " 
//  		       << aLengthMap[aName] << std::endl;
		}
	      else
		{
		  // has sub-elements
		  std::string aNode = aName.substr( 0, aPos );
		  std::string aMember = aName.substr( aPos + 1, std::string::npos );
		  if (aMember.length() > 31)
		    {
		      std::cerr << aNode << "'s member " << aMember
			   << " is too long (max 31 chars), please adapt it"
			   << std::endl;
		      exit( 1 );
		    }
//  		  std::cerr << aNode << " has member " << aMember << std::endl;
		  aStructMap[aNode] = true;
		  anElementMap[aNode].push_back( aMember );
		  if (aLengthMap.find( aNode ) == aLengthMap.end())
		    {
		      aLengthMap[aNode] = 64 +
			( static_cast< int >( aNode.length() ) > 4 ?
			  (((static_cast< int >( aNode.length() ) - 1) 
			    / sizeof( double )) * sizeof( double )) :
			  -sizeof( double ));
		    }
		  aLengthMap[aNode] += 88 + numSteps * sizeof( double );
//  		  std::cerr << "Length of " << aNode
//  		       << " is now: " << aLengthMap[aNode] << std::endl;
		}
	    }
	  nameStream.close();

	  // Create output file
	  oFD = ::creat( theMatlab.c_str(),
			 S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH );
	  if (oFD == -1)
	    {
	      std::cerr << "Failed to create: " << theMatlab << std::endl;
	      perror( "" );
	      exit( 1 );
	    }

	  sprintf( aCharBuffer, "MATLAB 5.0 MAT-file, Platform: LNX86,"
		   " Created by: nodeHelper.linux                     "
		   "                                     " );
	  ::write( oFD, aCharBuffer, sizeof( char ) * strlen( aCharBuffer ) );
	  aValue = 1296630016;
	  ::write( oFD, &aValue, sizeof( unsigned int ) );

	  for ( std::map< std::string, bool >::iterator varIter =
		  aStructMap.begin();
		varIter != aStructMap.end(); varIter++ )
	    {
	      std::string theNode = varIter->first;
	      int aCount = 0;
	      anIntBuffer[aCount++] = 14;
	      anIntBuffer[aCount++] = aLengthMap[theNode]; // space needed
	      anIntBuffer[aCount++] = 6;
	      anIntBuffer[aCount++] = 8;
	      if (varIter->second)
		anIntBuffer[aCount++] = 2; // struct
	      else
		anIntBuffer[aCount++] = 6; // double (also 4 = string)
	      anIntBuffer[aCount++] = 0;
	      anIntBuffer[aCount++] = 5;
	      anIntBuffer[aCount++] = 8;
	      anIntBuffer[aCount++] = 1; // rows
	      if (varIter->second)
		anIntBuffer[aCount++] = 1; // struct => 1 column
	      else
		anIntBuffer[aCount++] = numSteps; // variable => columns
	      if (theNode.length() <= 4)
		{
		  anIntBuffer[aCount++] = 1 + theNode.length()*256*256;
		  aValue = 4 - theNode.length();
		}
	      else
		{
		  anIntBuffer[aCount++] = 1;
		  anIntBuffer[aCount++] = theNode.length();
		  aValue = 7 - ((theNode.length() - 1) % 8);
		}
	      ::write( oFD, anIntBuffer, sizeof( unsigned int ) * aCount );
	      ::write( oFD, theNode.c_str(), 
		       sizeof( char ) * strlen( theNode.c_str() ) );
	      
	      for (unsigned int i = 0; i < aValue; i++)
		aCharBuffer[i] = '\0';
	      ::write( oFD, aCharBuffer, aValue );
	      if (varIter->second)
		{ // Go though elements of struct
		  std::list< std::string >& elementList = anElementMap[theNode];
		  aCount = 0;
		  // Set up for the list of names
		  anIntBuffer[aCount++] = 262149;
		  anIntBuffer[aCount++] = 32;
		  anIntBuffer[aCount++] = 1;
		  anIntBuffer[aCount++] = 32 * elementList.size();
		  ::write( oFD, anIntBuffer, sizeof( unsigned int ) * aCount );
		  // Now print the names
		  for ( std::list< std::string >::iterator listIter =
			  elementList.begin();
			listIter != elementList.end(); listIter++ )
		    {
		      ::write( oFD, listIter->c_str(), 
			       sizeof( char ) * strlen( listIter->c_str() ) );
		      // Pad to 32 bytes
		      aValue = 32 - strlen( listIter->c_str() );
		      for (unsigned int i = 0; i < aValue; i++)
			aCharBuffer[i] = '\0';
		      ::write( oFD, aCharBuffer, aValue );
		    }
		  // Now put in the values themselves
		  for ( std::list< std::string >::iterator listIter =
			  elementList.begin();
			listIter != elementList.end(); listIter++ )
		    {
		      std::string fullName = theNode + "." + *listIter;
		      aCount = 0;
		      anIntBuffer[aCount++] = 14;
		      anIntBuffer[aCount++] = numSteps * sizeof( double )
			+ 48; // space needed
		      anIntBuffer[aCount++] = 6;
		      anIntBuffer[aCount++] = 8;
		      anIntBuffer[aCount++] = 6; // double
		      anIntBuffer[aCount++] = 0;
		      anIntBuffer[aCount++] = 5;
		      anIntBuffer[aCount++] = 8;
		      anIntBuffer[aCount++] = 1; // rows
		      anIntBuffer[aCount++] = numSteps; // columns
		      anIntBuffer[aCount++] = 1;
		      anIntBuffer[aCount++] = 0; // No name
		      anIntBuffer[aCount++] = 9;
		      anIntBuffer[aCount++] = numSteps
			* sizeof( double ); // space needed
		      ::write( oFD, anIntBuffer,
			       sizeof( unsigned int ) * aCount );
		      
		      // Now open input file
		      iFD = ::open( theBinary.c_str(), O_RDONLY );
		      if (iFD == -1)
			{
			  std::cerr << "Failed to open: " << theBinary << std::endl;
			  perror( "" );
			  exit( 1 );
			}
		      int order = anOrderMap[ fullName ];
		      for (int i=0; i < numSteps; i++)
			{
			  ::read( iFD, (void *)aDoubleBuffer,
				  numVars * sizeof( double ) );
			  ::write( oFD, aDoubleBuffer + order,
				   sizeof( double ));
			}
		      ::close( iFD );
		    }
		}
	      else
		{ // Just do variable
		  aCount = 0;
		  anIntBuffer[aCount++] = 9; // for doubles
		  // also 2 for bytes, 4 for shorts and chars, 5 for longs?
		  anIntBuffer[aCount++] = numSteps * sizeof( double );
		  ::write( oFD, anIntBuffer, sizeof( unsigned int ) * aCount );
		  // Now open input file
		  iFD = ::open( theBinary.c_str(), O_RDONLY );
		  if (iFD == -1)
		    {
		      std::cerr << "Failed to open: " << theBinary << std::endl;
		      perror( "" );
		      exit( 1 );
		    }
		  int order = anOrderMap[ theNode ];
		  for (int i=0; i < numSteps; i++)
		    {
		      ::read( iFD, (void *)aDoubleBuffer, 
			      numVars * sizeof( double ) );
		      ::write( oFD, aDoubleBuffer + order,
			       sizeof( double ));
		    }
		  ::close( iFD );
		}
	    }

	  delete aDoubleBuffer;
	  ::close( oFD );
	}

      // Print out instructions
      if (first)
	theInstructions.push_back( " [ ( -h | --help ) ]: "
				   "print out this message\n" );
      if ((argc > 1) && 
	  ((!strcmp(argv[1], "--help")) || (!strcmp(argv[1], "-h"))))
	{
	  failed = false;
	  for ( std::list< char* >::iterator anIter =
		  theInstructions.begin(); anIter != theInstructions.end();
		anIter++ )
	    {
	      std::cerr << *anIter;
	    }
	  std::cerr << std::endl;
	  argc = 1;
	}
      
      if (failed)
	{
	  if (argc > 1)
	    std::cerr << std::endl << "Unrecognized argument: " << argv[1] << std::endl;
	  for ( std::list< char* >::iterator anIter =
		  theInstructions.begin(); anIter != theInstructions.end();
		anIter++ )
	    {
	      std::cerr << *anIter;
	    }
	  std::cerr << std::endl;
	  exit( 1 );
	}
      first = false;
    }

  return 0;
}
