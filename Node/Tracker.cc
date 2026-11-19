// $Id: Tracker.cc,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#include <iostream>
#include <cstdio>

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#pragma implementation

#include "Tracker.hh"
#include "Params.hh"
#include "interface.h"

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
	aParams.addParam( "Tracker", "enum" );
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
	aParams.addParam( "Camera", "enum" );
	aParams.addParam( "Reel", "enum" );
	  }
	catch ( bad_param& bp )
	  {
	    std::cerr << bp.what() << " already defined!" << std::endl;
	    throw;
	  }
      }
  }
}

/// Destructor for Tracker
Tracker::~Tracker()
{
  int retval;
  INT quit=(INT)SIM_QUIT;
  if (iFD >= 0)
    {
      send(iFD, &quit, sizeof(INT), 0);
      usleep(100000);
      retval = shutdown(iFD,SHUT_RDWR);
      if ((retval==0) || (retval==ENOTCONN)) // already shut
	{
	  if (close(iFD)!=0)
	    {
	      perror("~Tracker: error shutting down socket");
	      throw(bad_peripheral( iType ));
	    }
	}
      else
	{	
	  perror("~Tracker: error closing socket");
	  throw(bad_peripheral( iType ));
	}
    }
  iFD = -1;
}

/// Initialise parameter names for this node type
/**
 * @exception bad_param thrown if the parameter already exists
 */
void Tracker::initParams() throw ( bad_param, bad_file )
{
  ginput_init( gRoot );
}

/// Initialises and configures the node
/**
 * This is called by constructors, but can be called at any point (if
 * parameters have been changed, for instance).
 */
void Tracker::initialise() throw ( bad_peripheral )
{
#ifdef DEBUG
  std::cout << "Initialising a tracker called " << iName << std::endl;
#endif
  bool success=false;
  int retval, tries=0;
  
  // for socket comms
  struct sockaddr_in address;
  struct hostent *ip_addr;
  
  iX=0.0;
  iY=0.0;
  iTheta=0.0;

  if (iFD < 0)
    {
      address.sin_family=AF_INET;
      address.sin_port=htons(PORT_TRACK);
      ip_addr=gethostbyname(iHost.c_str());
      if (!ip_addr)
	{
	  std::cerr << "Tracker::initialise: failed to find host " 
	       << iHost << " with error: "
	       << hstrerror(h_errno)
	       << std::endl;
	  throw bad_peripheral( iType );
	}
      memcpy(&(address.sin_addr.s_addr), &(ip_addr->h_addr_list[0][0]), 4);
      
      // create socket
      retval=socket(PF_INET, SOCK_STREAM, 0);
      if (retval==-1)
	{
	  perror("tracker_start: failed socket request");
	  throw bad_peripheral( iType );
	}
      else
	{
	  iFD=retval;
	}

      retval=1;
      while (retval>0)
	{
	  retval=connect(iFD, reinterpret_cast<struct sockaddr *>(&address),
			 sizeof(struct sockaddr_in));
	  if (retval==-1)
	    {
	      if ((errno!=ECONNREFUSED) || (tries>=5))
		{
		  perror("Tracker::initialise: cannot connect");
		  throw bad_peripheral( iType );
		}
	      else
		{ // perhaps other process hasn't started yet
		  std::cout
		    << "Connection refused, waiting for server to start"
		    << std::endl;
		  tries++;
		  retval=1;
		  sleep(5);
		}
	    }
	  else
	    success=true;
	}
      
      if (!success)
	throw bad_peripheral( iType );
    }

  if (++iID > 15)
    iID = 0;
  
  INT go[2];
  go[0] = (INT) SIM_START_SIM;
  go[1] = iID;
  //      std::cout << "Setting tracker running" << std::endl;
  send(iFD, go, sizeof(INT)*2, 0);
  iRunning = true;      
  
  fd_set read_fds;
  INT answer;
  float data[3];
  int length = 3*sizeof(float);
  retval = 0;
  
  FD_ZERO( &read_fds );
  FD_SET( iFD, &read_fds );
  bool got_current = false;
  // Force get of current position
  while (!got_current)
    {
      retval = select( iFD+1, &read_fds, NULL, NULL, NULL ); // hang
      if (retval>=0)
	{
	  if (FD_ISSET( iFD, &read_fds ))
	    {
	      retval = read( iFD, (void *)&answer, sizeof(INT));
	      while (retval < (int)sizeof(INT))
		retval += read( iFD, retval + ((char *)&answer),
				sizeof(INT) - retval);
	      retval = read( iFD, data, length );
	      while (retval < length)
		retval += read( iFD, retval + ((char *)data),
				length - retval );
	      if ((answer&15)==iID)
		{
		  iX = data[0];
		  iY = data[1];
		  iTheta = data[2];
		  got_current=true;
		}
#ifdef DEBUG
	      printf("Using up track to: %f %f @ %f\n", iX, iY, iTheta );
#endif
	    }
	}
    }
  // Then shut down tracker
  INT quit = (INT) SIM_STOP;
  
  send(iFD, &quit, sizeof(INT), 0);
  iRunning = false;
  
  // Set name for spike message
  iMessage.newSource( iName );
  
}

/// Do one timestep for self only
bool Tracker::nodeTimestep()
{
  fd_set read_fds;
  static INT answer=16;
  float data[3] = {0.0, 0.0, 0.0};
  int length = 3*sizeof(float);
  int retval = 0;
  static int size = 0;
  static bool done_answer=false;

  FD_ZERO( &read_fds );
  FD_SET( iFD, &read_fds );
  struct timeval tv;
  tv.tv_usec=0;
  tv.tv_sec=0;

  if ((!iRunning) && (iFD >= 0))
    {
      if (++iID > 15)
	iID = 0;

      INT go[2];
      go[0] = (INT) SIM_START_SIM;
      go[1] = iID;
      send(iFD, go, sizeof(INT)*2, 0);
      iRunning = true;      
    }

  retval = select( iFD+1, &read_fds, NULL, NULL, &tv );
  if (retval>=0)
    {
      if (FD_ISSET( iFD, &read_fds ))
	{
          if (!done_answer)
            {
//               size = read( iFD, (void *)&answer, sizeof(INT));
//               while (size < (int)sizeof(INT))
              size += read( iFD, size + ((char *)&answer),
                            sizeof(INT) - size);
              if (size == sizeof(INT))
                {
                  done_answer = true;
                  size = 0;
		  //		  std::cerr << "Received " << answer << " id should be " << iID
		  //			    << std::endl;
		  if ((answer & 15) != iID)
		    answer = (answer & 15) + 16; // set to okay
                }
            }
          else
            {
// 	  size = read( iFD, data, length );
// 	  while (size < length)
              size += read( iFD, size + ((char *)data),
			    length - size );
              if (size == length)
                {
                  done_answer = false;
                  size = 0;
                  // use position only if from current run
                  if ((answer&15)==iID)
                    {
                      iX = data[0];
                      iY = data[1];
                      iTheta = data[2];
                      //    printf("found: %d (%f, %f)\n", answer, iX, iY);
                    }
                  else
                    {
		      std::cerr << "Tracker::nodeTimestep received false ID " 
				<< (answer&15) << " should be " << iID
				<< std::endl;
		      answer=16;
		    }
#ifdef DEBUG
                  printf("Tracked to: %f %f @ %f\n", iX, iY, iTheta );
#endif
                }
            }
        }
    }
  iSumX += iX;
  iSumY += iY;
  iSumTheta += iTheta;
  iCount++;
  
  if (done_answer && ((answer>>4) == 0))
    { // if we have got to stop, then finish reception first
      while (size < length)
	size += read( iFD, size + ((char *)data),
		      length - size );
      // use position only if from current run
      if ((answer&15)==iID)
	{
	  iX = data[0];
	  iY = data[1];
	  iTheta = data[2];
	  //    printf("found: %d (%f, %f)\n", answer, iX, iY);
	}
      else
	{
	  std::cerr << "Tracker::nodeTimestep received false ID " 
		    << (answer&15) << " should be " << iID
		    << std::endl;
	  answer=16;
	}
#ifdef DEBUG
      printf("Tracked to: %f %f @ %f\n", iX, iY, iTheta );
#endif
    }

  return ((!done_answer) || ((answer>>4) != 0));
}

/// Output configuration
/**
 * @param anOStream desired output stream
 * @return output stream after output
 */
std::ostream& Tracker::externalise( std::ostream& anOStream, 
				    std::string indent )
{
#ifdef DEBUG
  anOStream << "// Configuring a tracker called " << iName 
	    << std::endl << indent;
  anOStream << std::endl << indent;
#endif

  anOStream << "add Input ";

  switch (iType)
    {
    case TRACKER_CAMERA:
      anOStream << "camera " << iName << "( \"" << iHost << "\" );";
      break;
    case TRACKER_REEL:
      anOStream << "reel " << iName << "( \"" << iHost << "\" );";
      break;
    default:
      std::cerr << "Tracker::externalise: non-tracker peripheral type"
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
std::ostream& Tracker::write( std::ostream& anOStream,
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
      if (gOutputLevel.test( outputTrack ))
	{
	  anOStream << myName << ".iX\t";
	  anOStream << myName << ".iY\t";
	  anOStream << myName << ".iTheta\t";
	}
      break;
    case outTypeMatlab:
      if (gOutputLevel.test( outputTrack ))
	{
	  anOStream << aName << ".tracker = 1;" << std::endl;
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
double* Tracker::put( double* anArray, bool average )
{
  double* thePtrPos = anArray;

  if (average)
    {
      if (iCount > 1.0)
	{
	  if (gOutputLevel.test( outputTrack ))
	    {
	      *(thePtrPos++) = iSumX/iCount;
	      *(thePtrPos++) = iSumY/iCount;
	      *(thePtrPos++) = iSumTheta/iCount;
	    }
	}
      else
	{
	  if (gOutputLevel.test( outputTrack ))
	    {
	      *(thePtrPos++) = iX;
	      *(thePtrPos++) = iY;
	      *(thePtrPos++) = iTheta;
	    }
	}
      
      iCount = 0;
      iSumX = 0;
      iSumY = 0;
      iSumTheta = 0;
    }
  else
    {
      if (gOutputLevel.test( outputInput ))
	{
	  *(thePtrPos++) = iX;
	  *(thePtrPos++) = iY;
	  *(thePtrPos++) = iTheta;
	}
    }      
  return thePtrPos;
}
