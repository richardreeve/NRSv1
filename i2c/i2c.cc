// $Id: i2c.cc,v 1.2 2003/07/22 09:16:48 darrens Exp $

#ifdef ALLEGRO_PRESENT
#include <allegro.h>
#endif

#include <cstdlib>
#include <cstdarg>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <cmath>
#include <cerrno>
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <queue>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>
#include <signal.h>

#include "base.h"
#include "interface.h"

#define PROGRAM_NAME "tether.linux: "
// Highest i2c slave device
#define MAX_I2C 127
// Number of bytes to request from slaves
#define NUM_BYTES 2
// Number of mm to pull away from tethers to start calibration
#define MIN_CALIBRATE 200
// Number of mm to accept as close enough to count as a calibration
#define MAX_ACCEPT 30
// Maximum approach distance defined as contact in mm
#define APPROACH 1000.0
// Distance from target to be accepted as contact in mm
#define TARGET_DIST 1000.0
// Length of tethers
#define TETHER_LENGTH 20000
// Maximum disagreement between tethers before recalibration is necessary
#define ERROR_WARN 250.0
#define ERROR_REJECT 750.0
// number of consecutive errors outside ERROR_REJECT before we kill the run.
#define NUM_ERRORS 10

#define SCREEN_X 640
#define SCREEN_Y 480
//#define OFFSET_X 200

// maximum movement between timesteps before confirmation required
#define MOVEMENT 20 

void (*G_old_handler)(int);

// perror with formatting!
static void perrorf( char *format, ... )
{
  va_list ap;
  fprintf( stderr, PROGRAM_NAME );
  va_start( ap, format );
  vfprintf( stderr, format, ap );
  va_end( ap );
  perror( " " );
}

void abort_handler( int signum )
{
#ifdef ALLEGRO_PRESENT
  allegro_exit();
#endif
  std::cout << "Aborting, closing allegro" << std::endl;
  signal( SIGINT, G_old_handler );
  exit( 1 );
}

// Returns angle opposite side a of triangle of sides of length a, b and c
double calculateAngle( double a, double b, double c )
{
  return /* (180.0 / M_PI) * */ acos ( ((double)(b*b + c*c - a*a)) /
				       ((double)(2.0 * b * c)) );
}

class Position
{
public:
  Position( double x, double y, double theta = 0.0 ): iX( x ), iY( y ),
						      iTheta( theta ) {}

  Position() : iX( 0.0 ), iY( 0.0 ), iTheta( 0.0 ) {}

  ~Position() {}

  void setPos( double x, double y, double theta )
  {
    iX = x;
    iY = y;
    iTheta = theta;
  }
  
  void setPos( double x, double y )
  {
    iX = x;
    iY = y;
  }

  void setAngle( double theta )
  {
    iTheta = theta;
  }

 void  getPos( double &x, double &y, double &theta )
  {
    x = iX;
    y = iY;
    theta = iTheta;
  }
  
  void getPos( double &x, double &y )
  {
    x = iX;
    y = iY;
  }

  double getX()
  {
    return iX;
  }

  double getY()
  {
    return iY;
  }

  double getAngle()
  {
    return iTheta;
  }

  double getBearing( Position aPos )
  {
    return atan2( aPos.iY - iY, aPos.iX - iX );
  }

  Position getVector( Position aPos )
  {
    return Position( aPos.iX - iX, aPos.iY - iY, getBearing( aPos ) );
  }

  void setDirection( Position aPos )
  {
    iTheta = getBearing( aPos );
  }

  double getLength()
  {
    return sqrt( iX * iX + iY * iY );
  }

  void clear()
  {
    iX = 0.0;
    iY = 0.0;
    iTheta = 0.0;
  }

  void transformVector( Position aVector )
  {
    double myLen = getLength();
    iTheta = iTheta + aVector.iTheta;
    iX = aVector.iX + myLen * cos( iTheta );
    iY = aVector.iY + myLen * sin( iTheta );
  }

  Position operator-( Position aPos )
  {
    return aPos.getVector( *this );
  }

  Position operator+( Position aPos )
  {
    return Position( iX + aPos.iX, iY + aPos.iY, iTheta + aPos.iTheta );
  }

  Position& operator+=( Position aPos )
  {
    *this = *this + aPos;
    return *this;
  }

  Position operator/( double aNum )
  {
    return Position( iX/aNum, iY/aNum, iTheta/aNum );
  }

  Position& operator/=( double aNum )
  {
    iX /= aNum;
    iY /= aNum;
    iTheta /= aNum;

    return *this;
  }

  static Position newCalculation( Position aPos, int aDistance,
				  Position bPos, int bDistance )
  {
    Position newPos;
    newPos.iTheta = calculateAngle( static_cast< double >( bDistance ),
				    static_cast< double >( aDistance ),
				    (bPos - aPos).getLength() );
    //    std::cout << aDistance << ", " << bDistance << std::endl;
    newPos.iX = static_cast< double >( aDistance ) * cos( newPos.iTheta );
    newPos.iY = static_cast< double >( aDistance ) * sin( newPos.iTheta );
    //    newPos.put( std::cout << "before: " );
    aPos.setDirection( bPos );
    newPos.transformVector( aPos );
    //    newPos.put( std::cout << "after: " );
    return newPos;
  }

  std::ostream& put( std::ostream& anOStream)
  {
    return anOStream << "( " << iX << ", " << iY << " ) @ " << iTheta
		     << std::endl;
  }

private:
  double iX, iY, iTheta;
};

class I2C
{
  
public:

  I2C( int adapter_number = 0 ) : iTotal( 0 )
  {
    char aFilename[20];
    
    sprintf( aFilename, "/dev/i2c-%d", adapter_number );
    if ((iFile = open( aFilename, O_RDWR )) < 0)
      {
	perrorf( "I2C::I2C: couldn't not open '%s'", aFilename );
	exit( 1 );
      }

    calibrate();
  }

  ~I2C()
  {
    close( iFile );
  }

  void calibrate()
  {
    int i, j, check, num, dist;

    iSlaves.clear();
    iTotal = 0;

    for ( i = 0; i <= MAX_I2C; i++ )
      {
	if ((num = checkSlave( i, false )) >= 0)
	  {
	    iSlaves.push_back( i );
	    iTotal++;
	    if (num != 0)
	      {
		std::cerr << "I2C::calibrate: tether " << i 
		     << " is reading " << num << ", reset?" << std::endl;
	      }
	    std::cout << "Found tether " << i << std::endl;
	  }
      }

    
    
    iCalibration.resize( iTotal );
    for (i = 0; i < iTotal; i++)
      iCalibration[i].resize( iTotal );

    std::cerr << "Total: " << iTotal << " tethers" << std::endl;
    
    std::cout << "Calibration: now hook all of the leads together and" << std::endl
	 << "             pull them at least " << MIN_CALIBRATE
	 << "mm from the nearest tether" << std::endl;
    
    check = 0;
    while ( check < iTotal )
      {
	check = 0;
	for ( i = 0; i < iTotal; i++ )
	  {
            dist = getSlave( i, true );
            //std::cerr << iSlaves[i] << " : " << dist << std::endl;
	    if (dist > MIN_CALIBRATE)
              {
                iCalibration[i][i] = dist;
                check++;
              }
	  }
      }
    
    std::cout << "Calibration: now pull the leads within " << MAX_ACCEPT
	 << "mm of all the tethers," << std::endl
	 << "             starting with the origin, and going anticlockwise."
	 << std::endl
	 << "             The line between the first 2 tethers is the x-axis."
	 << std::endl;
    
    check = 0;
    int far = 0;
    while (( check < iTotal ) || ( far < iTotal ))
      {
	far = 0;
	for ( i = 0; i < iTotal; i++ )
	  {
	    if ((dist = getSlave( i, true )) <= MAX_ACCEPT)
	      {
		if (iCalibration[i][i] > MAX_ACCEPT)
		  {
		    if (i != check)
		      {
			int store = iSlaves[i];
			iSlaves[i] = iSlaves[check];
			iSlaves[check] = store;
			store = iCalibration[i][i];
			iCalibration[i][i] = iCalibration[check][check];
			iCalibration[check][check] = store;
			// we don't need to swap iCalibration[...]([i]
			//                                         <->[check])
			// because by definition they haven't been set yet!
			//std::vector< int > calibrate = iCalibration[i];
			//iCalibration[i] = iCalibration[check];
			//iCalibration[check] = calibrate;
			i = check;
		      }
		    check++;
		    std::cerr << "Got tether " << iSlaves[i] << std::endl;
		  }
		
		if (dist < iCalibration[i][i])
		  {
		    for (j = 0; j < iTotal; j++)
		      {
                        if (i!=j)
                          iCalibration[j][i] = getSlave( j );
                        else // allow reset when at zero
                          {
                            iCalibration[i][i] = getSlave( i, true );
                            // but forces uncalibration
//                             if (iCalibration[i][i] == -1)
//                               {
//                                 check = 1;
//                                 std::cerr << iSlaves[i] 
//                                           << " was reset, please restart"
//                                           << " calibration at origin"
//                                           << std::endl;
//                                 iCalibration[i][i] = MAX_ACCEPT+1;
//                               }
                          }
		      }
		    if (iCalibration[i][i] > MAX_ACCEPT)
		      {
			std::cerr << "Oops - lost " << iSlaves[i]
			     << ", try again" << std::endl;;
			check--;
		      }
		  }
	      }
            else if (dist == -1)
              {
                check = 0;
                std::cerr << iSlaves[i] << " was reset, please restart"
                          << " calibration at origin" << std::endl;
              }
	    else if (dist > MIN_CALIBRATE)
	      far++;
	  }
      }
    std::cout << "Calibration: okay, got all of the tethers, calibrating..."
	 << std::endl;
    
    iPosition.clear();

    if (iTotal > 0)
      {
	iPosition.push_back( Position( 0.0, 0.0 ) );
      }

    if (iTotal > 1)
      {
	iPosition.push_back( Position( iCalibration[0][1], 0.0 ) );
      }

    for (i = 2; i < iTotal; i++)
      {
	iPosition.push_back( Position::newCalculation( iPosition[i - 2],
						       iCalibration[i - 2][i],
						       iPosition[i - 1],
						       iCalibration[i - 1][i] )
			     );
      }

    if (iTotal < 2)
      {
	std::cerr << "Now I look at the setup, it's not very useful" << std::endl
	     << "maybe you need to find some more tethers!" << std::endl;
	     
	exit( 1 );
      }
    
    for (i = 0; i < iTotal; i++)
      {
	iPosition[i].put( std::cout << "calibration " << i << " : " );
      }

    std::cout << "Calibration: Now pull the leads to the target" << std::endl
	 << "             and press return to mark the spot" << std::endl;
    getchar();

    iEstimates.resize( iTotal );
    iLength.resize( iTotal );

    iTarget = getPosition( true );

    std::cout << "Calibration: Now attach the leads to the robot" << std::endl
	 << "             and press return to start tracking" << std::endl;
    getchar();
  }

  void doUpdate()
  {
    int tmp;
    for (int i = 0; i < iTotal; i++)
      {
        tmp = getSlave( i );
        while ((tmp < iLength[i] - MOVEMENT) || (tmp > iLength[i] + MOVEMENT))
          {
            std::cerr << ".";
            iLength[i]=tmp;
            tmp = getSlave( i );
          }
        iLength[i] = tmp;
      }
  }

  Position getPosition( bool update = false )
  {
    int theMax = ( iTotal == 2 ? 1 : iTotal );
    if (update)
      doUpdate();
    
    iCurrent.clear();

    for (int i = 0; i < theMax; i++)
      {
	iEstimates[i] = Position::newCalculation( iPosition[i],
						  iLength[i],
						  iPosition[(i + 1) % iTotal],
						  iLength[(i + 1) % iTotal] );
	iCurrent += iEstimates[i];
      }

    iCurrent /= static_cast< double >( theMax );
    return iCurrent;
  }

  double getError( bool recalc = false, bool update = false )
  {
    double theError = 0.0;
    double thisError;

    if (recalc)
      getPosition( update );
    
    int theMax = ( iTotal == 2 ? 1 : iTotal );
    for (int i = 0; i < theMax; i++)
      {
        thisError = (iEstimates[i]-iCurrent).getLength();
        if (theError < thisError)
          theError = thisError;
      }
    return theError;
  }

  bool errorCorrection( double distance = ERROR_WARN,
			bool recalc = false, bool update = false )
  {
    bool excess = false;

    if (getError( recalc, update ) >= distance)
      excess = true;

    return excess;
  }

  bool atTarget( double distance = TARGET_DIST,
		 bool recalc = false, bool update = false )
  {
    if (recalc)
      getPosition( update );
    
    return ((iTarget - iCurrent).getLength() <= distance);
  }

  bool outofBounds( double distance = APPROACH,
		    bool recalc = false, bool update = false )
  {
    if (recalc)
      getPosition( update );
    
    for (int i = 0; i < iTotal; i++)
      {
	// getting dangerously close to convex hull
	if (iLength[i] + iLength[(i + 1) % iTotal] - 2 * MAX_ACCEPT <
	    (iPosition[(i + 1) % iTotal] - iPosition[i]).getLength())
	  return true;
	// getting dangerously close to tether
	if (distance > iLength[i])
	  return true;
	// reached target
	if (TETHER_LENGTH < iLength[i])
	  return true;
      }

    return false;
  }

  Position getTarget()
  {
    return iTarget;
  }

  int getNumTethers()
  {
    return iTotal;
  }

  Position getTether( int i )
  {
    return iPosition[i];
  }

private:

  int checkSlave( int addr, bool ignore = true )
  {
    char array[NUM_BYTES];
    short* num = (short *)array;
    ssize_t sz = 0;
    bool first = true;

    if (ioctl( iFile, I2C_SLAVE, addr ) < 0)
      {
	perrorf( "check_slave: couldn't set slave to %d", addr );
	close( iFile );
	exit( 1 );
      }
    
    while (first || ((sz != NUM_BYTES) && ignore))
      {
	first = false;
	if ((sz = read( iFile, num, NUM_BYTES )) != NUM_BYTES)
	  {
	    if ((sz == -1) && (errno == EREMOTEIO))
	      {
		if (ignore)
		  std::cout << "No such id " << addr << std::endl;
		else
		  return -1;
	      }
	    else if (sz >= 0)
	      {
		std::cout << "check_slave: couldn't read " << NUM_BYTES
		     << " bytes from " << addr << ", got " << sz << std::endl;
		if (!ignore)
		  { 
		    close( iFile );
		    exit( 1 );
		  }
	      }
	    else
	      {
		perrorf( "check_slave: couldn't read %d bytes from %d", 
			 NUM_BYTES, addr );
		if (!ignore)
		  { 
		    close( iFile );
		    exit( 1 );
		  }
	      }
	    
	  }
      }
    //    std::cout << addr << " - " << *num << std::endl;
    return (int) *num;
  }

  // Get value from slave, exit if fail
  int getSlave( int index, bool ignore = false )
  {
    if (index >= iTotal)
      {
	std::cerr << "getSlave: no such slave index " << index << std::endl;
	close( iFile );
	exit( 1 );
      }
    int num = checkSlave( iSlaves[index] );
    
    if ((num == -1) && (!ignore))
      {
	std::cerr << "getSlave: no such slave address " << iSlaves[index] << std::endl;
	close( iFile );
	exit( 1 );
      }
    
    return num;
  }

private:

  int iFile;
  int iTotal;
  std::vector< int > iSlaves;
  std::vector< std::vector< int > > iCalibration;
  std::vector< Position > iPosition;
  Position iTarget;
  std::vector< int > iLength;
  std::vector< Position > iEstimates;
  Position iCurrent;
};

int main( int argc, char** argv )
{
  int adapter_nr = 0; /* probably right */
  Position theTarget;
  Position aPos;
  bool finished = false;
  std::string input;
#ifdef ALLEGRO_PRESENT
  bool allegro = false;
  int highlightColour=0xffff00;
  int whiteColour=0xffffff;
  int greyColour=0x808080;
  double offsetX, offsetY, scale;
  int sizeX, sizeY;
  offsetX = 0;
  offsetY = 0;
  sizeX=SCREEN_X;
  sizeY=SCREEN_Y;
  scale = 1;
#endif
  bool network = false;
  bool debugging = false;
  fd_set read_fds;
  struct timeval tv;
  struct sockaddr_in address;
  int tmp_sock_fd, sock_fd=0;
  unsigned int addrlen;
  bool do_track=true;
  float data[3];
  INT answer;
  INT id=0;
  int msg_len = 3 * sizeof(float);
  int retval;
  INT int_signal;
  SimInterface my_signal;
  bool network_send=false;
  std::queue< Position > aQueue;
  int length = 0;
  bool inError = false;
  int numErrors = 0;
  double theErr = 0.0;

  while (argc > 1)
    {
      if ((!strcmp(argv[1], "--i2c_adaptor")) || (!strcmp(argv[1], "-i")))
	{
	  argc--;
	  argv++;
	  adapter_nr = atoi( argv[1] );
	  argc--;
	  argv++;
	}
      else if ((!strcmp(argv[1], "--debug")) || (!strcmp(argv[1], "-d")))
	{
	  argc--;
	  argv++;
	  debugging=true;
	}
      else if ((!strcmp(argv[1], "--network")) || (!strcmp(argv[1], "-n")))
	{
	  argc--;
	  argv++;
	  network=true;
	}
      else if ((!strcmp(argv[1], "++network")) || (!strcmp(argv[1], "+n")))
	{
	  argc--;
	  argv++;
	  network=false;
	}
#ifdef ALLEGRO_PRESENT
      else if ((!strcmp(argv[1], "--allegro")) || (!strcmp(argv[1], "-a")))
	{
	  argc--;
	  argv++;
	  allegro=true;
	}
      else if ((!strcmp(argv[1], "++allegro")) || (!strcmp(argv[1], "+a")))
	{
	  argc--;
	  argv++;
	  allegro=false;
	}
#endif
//        else if ((!strcmp(argv[1], "--output")) || (!strcmp(argv[1], "-o")))
//  	{
//  	  argc--;
//  	  argv++;
//  	  strcpy(defaultName,argv[1]);
//  	  argc--;
//  	  argv++;
//  	}
      else if ((!strcmp(argv[1], "--help")) || (!strcmp(argv[1], "-h")))
	{
	  std::cout << std::endl << "tether.linux: a tether tracking program" << std::endl
	       << std::endl << "-i (--i2c_adaptor) <n> to change the adaptor number"
	       << std::endl << "-d (--debug) to show more debugging"
	       << std::endl << "-n (--network) to run a network server"
	       << std::endl << "+n (++network) to not run a network server"
#ifdef ALLEGRO_PRESENT
	       << std::endl << "-a (--allegro) to run a minimal allegro tracker"
	       << std::endl << "+a (++allegro) to not run allegro"
#endif
//  	       << std::endl << "-o (--output) <ofile> to output track to <ofile>"
	       << std::endl;
	  exit(0);
	}
      else
	{
	  std::cerr << "Argument not recognised: " << argv[1] << std::endl;
	  exit(1);
	}
    }

  I2C aConnection( adapter_nr );

#ifdef ALLEGRO_PRESENT
  // Are we graphical?
  if (allegro)
    { // Initialise allegro
      allegro_init();
      install_keyboard(); // install keyboard handler
      install_mouse();    // and mouse handler
      install_timer();    // timer needed for mouse
      std::cout << "Desktop colour depth = " << desktop_color_depth()
                << std::endl;      
      set_color_depth( desktop_color_depth() );
      
      highlightColour = makecol(0x20, 0xb0, 0x20);
      whiteColour = makecol( 0xff, 0xff, 0xff);
      greyColour = makecol( 0x80, 0x80, 0x80);

      G_old_handler = signal( SIGINT, abort_handler );
    }
#endif //def ALLEGRO_PRESENT


  while (!finished)
    {
#ifdef ALLEGRO_PRESENT
      if (allegro)
        { // wait for Esc to quit
	  double minx = 0.0;
	  double maxx = 0.0;
	  double miny = 0.0;
	  double maxy = 0.0;
	  for (int i = 0; i < aConnection.getNumTethers(); i++)
	    {
	      Position aPos = aConnection.getTether(i);
	      if (minx > aPos.getX())
		minx = aPos.getX();
	      if (miny > aPos.getY())
		miny = aPos.getY();
	      if (maxx < aPos.getX())
		maxx = aPos.getX();
	      if (maxy < aPos.getY())
		maxy = aPos.getY();
	    }
	  offsetX = minx;
	  offsetY = miny;
	  scale = ((maxx-minx)*SCREEN_Y < (maxy-miny)*SCREEN_X ?
		   SCREEN_Y/(maxy-miny) : SCREEN_X/(maxx-minx));
	  sizeX=static_cast<int>(scale*(maxx-minx));
	  sizeY=static_cast<int>(scale*(maxy-miny));
	  
	  set_gfx_mode(GFX_XWINDOWS, sizeX, sizeY+9, 0, 0);
	  clear( screen );
	  
	  std::cout << "Scale is " << scale << std::endl;
	  std::cout << "Size is " << sizeX << " by " << sizeY << std::endl;
	  std::cout << "Offset is " << offsetX << " by " << offsetY
		    << std::endl;
	  
          while (length-- > 0)
            {
//               putpixel( screen, (int)((-offsetX +
//                                        aQueue.front().getX())*scale),
//                         sizeY -
//                         (int)((-offsetY+aQueue.front().getY())*scale),
//                         greyColour );
//               aQueue.pop();
            }
          Position aPos;
	  Position oldPos =
	    aConnection.getTether(aConnection.getNumTethers()-1);

	  for (int i = 0; i < aConnection.getNumTethers(); i++)
            {
              aPos = aConnection.getTether(i);
              circle( screen, (int)((aPos.getX()-offsetX)*scale),
                      sizeY-(int)((aPos.getY()-offsetY)*scale),
                      (int)(APPROACH*scale), highlightColour);
	      line( screen, (int)((aPos.getX()-offsetX)*scale),
		    sizeY-(int)((aPos.getY()-offsetY)*scale),
		    (int)((oldPos.getX()-offsetX)*scale),
		    sizeY-(int)((oldPos.getY()-offsetY)*scale),
		    highlightColour );
	      oldPos = aPos;
            }
          aPos = aConnection.getTarget();
          circle( screen, (int)((aPos.getX()-offsetX)*scale),
                  sizeY-(int)((aPos.getY()-offsetY)*scale),
                  (int)(TARGET_DIST*scale), whiteColour);
        }
#endif // def ALLEGRO_PRESENT
      length = 0;
      if (network)
	{ // Initialise networking
	  address.sin_family=AF_INET;
	  address.sin_port=htons(PORT_TRACK);
	  address.sin_addr.s_addr=htonl(INADDR_ANY);
	  
	  // create socket
	  retval=socket(PF_INET, SOCK_STREAM, 0);
	  if (retval<0)
	    {
	      std::cerr << "main: failed socket request: " << strerror(errno)
		   << std::endl;
	      exit(1);
	    }
	  else
	    {
	      tmp_sock_fd=retval;
	    }
	  
	  addrlen=sizeof(struct sockaddr_in);
	  retval=bind(tmp_sock_fd, (struct sockaddr *)&address, addrlen);
	  if (retval==-1)
	    {
	      std::cerr << "main: cannot bind: " << strerror(errno)
		   << std::endl;
	      exit(1);
	    }
	  
	  // listen on socket
	  retval=listen(tmp_sock_fd, 1);
	  if (retval==-1)
	    {
	      std::cerr << "main: cannot listen: " << strerror(errno)
		   << std::endl;
	      exit(1);
	    }
	  // accept connection to socket
	  retval=accept(tmp_sock_fd, (struct sockaddr *)&address, &addrlen);
	  if (retval==-1)
	    {
	      std::cerr << "main: cannot accept: " << strerror(errno)
		   << std::endl;
	      exit(1);
	    }
	  sock_fd=retval;
	  if (close(tmp_sock_fd)!=0)
	    {
	      std::cerr << "main: error shutting down original socket: " 
		   << strerror(errno) << std::endl;
	      exit(1);
	    }
	}

      theTarget = aConnection.getTarget();

      do_track = true;

      if (debugging)
	theTarget.put( std::cout << "Target at " );
  
      // now track...
      while (do_track)
	{
	  if (network)
	    {
	      // check for commands
	      FD_ZERO(&read_fds);
	      FD_SET(sock_fd, &read_fds);
	      tv.tv_sec = 0;
	      tv.tv_usec = 0;
	      retval = select(sock_fd+1, &read_fds, NULL, NULL, &tv);
	      if (retval>=0)
		{
		  // timeout interrupted, answer call
		  if (FD_ISSET(sock_fd,&read_fds))
		    {
		      retval=read(sock_fd, &int_signal, sizeof(INT));
		      while (retval<(INT)sizeof(INT))
			retval+=read(sock_fd, (&int_signal)+retval,
				     sizeof(INT)-retval);
		      my_signal=(SimInterface)int_signal;
		      if (my_signal==SIM_QUIT)
			{
			  do_track=false;
			  network_send=false;
			  std::cout << "Tracker signalled to stop"
				    << std::endl;
			}
		      else if (my_signal==SIM_START_SIM)
			{
			  network_send=true;
			  retval=read(sock_fd, (void *)&id, sizeof(INT));
			  while (retval<(INT)sizeof(INT))
			    retval += read(sock_fd, ((char *)&id)+retval,
					   sizeof(INT)-retval);
			  std::cout << "Signalled to start transmitting"
				    << std::endl << "id = " << id << std::endl;
			  numErrors=0;
			}
		      else if (my_signal==SIM_STOP)
			{
			  network_send=false;
			  std::cout << "Signalled to stop transmitting"
				    << std::endl;
			}
		      else
			{
			  std::cerr << "Received illegal signal: " 
			       << int_signal << std::endl;
			}
		    }
		}
	    }
          
	  aPos = aConnection.getPosition( true );
          inError = false;
          theErr = aConnection.getError();
          //	  if (aConnection.errorCorrection())
	  if (theErr > ERROR_WARN)
            {
              std::cerr << "May need to recalibrate to correct errors!"
                        << std::endl;
              if (theErr > ERROR_REJECT)
                {
                  inError = true;
                  std::cerr << "Error of " << theErr << ", rejecting sample."
                            << std::endl;
                }
              else
                std::cerr << "Error only " << theErr << ", accepting as real."
                          << std::endl;
            }

          if (inError)
            numErrors++;
          else
            numErrors=0;

#ifdef ALLEGRO_PRESENT
	  if (allegro)
	    { // wait for Esc to quit
	      if (key[KEY_ESC])
		{
		  do_track=false;
		  network_send=false;
		}
	      if (length > 100)
		{
		  putpixel( screen, (int)((-offsetX +
                                           aQueue.front().getX())*scale),
			    sizeY -
                            (int)((-offsetY+aQueue.front().getY())*scale),
			    greyColour );
		  aQueue.pop();
		}
	      else
		length++;
              if (inError)
                {
                  textprintf( screen, font, 0, sizeY + 1,
                              highlightColour, "Error %f, number %d", 
                              theErr, numErrors);
                }
              else
                {
                  aQueue.push( aPos );
                  putpixel( screen, (int)((-offsetX + aPos.getX())*scale),
                            sizeY - (int)((-offsetY+aPos.getY())*scale),
                            whiteColour );
                  textprintf_right( screen, font, sizeX, sizeY + 1,
                                    greyColour, "Error %f", theErr);
                }
	    }
#endif // def ALLEGRO_PRESENT
	  
          if ((!inError) || (numErrors >= NUM_ERRORS))
            {
              if (numErrors >= NUM_ERRORS)
                std::cerr << "Too many errors (" << numErrors
                          << "): abort and recalibrate!"
                          << std::endl;
              
              answer = ((aConnection.atTarget() || aConnection.outofBounds() || (numErrors >= NUM_ERRORS)) ?
                        0 : 1 );
              data[0] = aPos.getX()-theTarget.getX();
              data[1] = aPos.getY()-theTarget.getY();
              data[2] = theErr;
              if (network)
                {
                  INT val = (answer << 4) + id;
		  //		  std::cerr << "Sending " << val << " id is " << id
		  //			    << std::endl;
                  if (network_send
                      && ((send(sock_fd, (void *)&val, 
                                sizeof(INT), 0)!=sizeof(INT)) ||
                          (send(sock_fd, (void *)data, msg_len, 0)!=msg_len)))
                    {
                      std::cerr << "Failed to send" << std::endl;
                    }
                }
              
              if ((!network) && (aConnection.atTarget() ||
                                 aConnection.outofBounds()))
                {
                  do_track = false;
                }
              if (debugging || ((!network)
#ifdef ALLEGRO_PRESENT
                                && (!allegro)
#endif
                                ))
                std::cout << aPos.getX() << ", " << aPos.getY() << std::endl;
            }
        }
          
      if (aConnection.atTarget())
        std::cout << "Found target" << std::endl;
      else if (aConnection.outofBounds())
        std::cout << "Out of bounds!" << std::endl;
      else
        std::cout << "Must have been told to stop" << std::endl;
      
      if (network)
	{ // Shut down network
	  if (shutdown(sock_fd,SHUT_RDWR)==0)
	    {
	      if (!(close(sock_fd)==0))
		{
		  std::cerr << "main: error shutting down socket: " 
			    << strerror(errno) << std::endl;
		  exit(1);
		}
	    }
	  else
	    {	
	      std::cerr << "main: error closing socket: " 
			<< strerror(errno) << std::endl;
	      exit(1);
	    }
	}

      bool answered = false;
      while (!answered)
        {
          std::cout << "Do you want to continue (y/n)?";
          std::cin >> input;
          if ((input == "y") || (input == "yes") || (input == "Y"))
            answered = true;
          else if ((input == "n") || (input == "no") || (input == "N"))
            {
              answered = true;
              finished = true;
            }
        }
      if (!finished)
        {
          answered = false;
          while (!answered)
            {
              std::cout << "Do you want to recalibrate (y/n)?";
              std::cin >> input;
              if ((input == "y") || (input == "yes") || (input == "Y"))
                {
                  aConnection.calibrate();
                  answered = true;
                }
              else if ((input == "n") || (input == "no") || (input == "N"))
                answered = true;
            }
	}
    }
  


#ifdef ALLEGRO_PRESENT
  if (allegro)
    allegro_exit(); // byebye allegro
#endif

  return 0;
}

#ifdef ALLEGRO_PRESENT
  END_OF_MAIN();
#endif
