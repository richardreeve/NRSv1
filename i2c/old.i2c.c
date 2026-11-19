// $Id: old.i2c.c,v 1.1.1.1 2003/07/21 14:46:57 darrens Exp $

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <math.h>
#include <errno.h>

#define PROGRAM_NAME "tether.linux: "
// Highest i2c slave device
#define MAX_I2C 127
// Maximum number of tethers we may have
#define MAX_BOXES 128
// Number of bytes to request from slaves
#define NUM_BYTES 3
// Number of mm to pull away from tethers to start calibration
#define MIN_CALIBRATE -1
// Number of mm to accept as close enough to count as a calibration
#define MAX_ACCEPT 5

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

// Initialise the I2C connection
int init_i2c( int adapter_nr )
{
  char filename[20];
  int file;
  
  sprintf( filename, "/dev/i2c-%d", adapter_nr );
  if ((file = open( filename, O_RDWR )) < 0)
    {
      perrorf( "init_i2c: couldn't not open '%s'", filename );
      exit( 1 );
    }

  return file;
}

// Check whether slave exists, return value if found or -1 if not there
int check_slave( int file, int addr )
{
  char array[NUM_BYTES];
  unsigned short* num = (short *)array;
  ssize_t sz;

  if (ioctl( file, I2C_SLAVE, addr ) < 0)
    {
      perrorf( "check_slave: couldn't set slave to %d", addr );
      close( file );
      exit( 1 );
    }

  if ((sz = read( file, num, NUM_BYTES )) != NUM_BYTES)
    {
      if ((sz == -1) && (errno == EFAULT))
	return -1;
      else if (sz >= 0)
	{
	  printf( "check_slave: couldn't read %d bytes from %d, got %d\n",
		  NUM_BYTES, addr, sz );
	}
      else
	{
	  perrorf( "check_slave: couldn't read %d bytes from %d", 
		   NUM_BYTES, addr );
	}

      close( file );
      exit( 1 );
    }

  return (int) *num;
}

// Get value from slave, exit if fail
int get_slave( int file, int addr )
{
  int num = check_slave( file, addr );

  if (num == -1)
    {
      printf( "get_slave: no such slave address %d\n", addr );
      close( file );
      exit( 1 );
    }

  return num;
}

// Returns angle opposite side a of triangle of sides of length a, b and c
double get_angle( int a, int b, int c )
{
  return (180.0 / M_PI) * acos ( ((double)(b*b + c*c - a*a)) /
				 ((double)(2.0 * b * c)) );
}

int main( int argc, char** argv )
{
  int file;
  int adapter_nr = 0; /* probably right */
  int num = 0;
  int total = 0;
  int slaves[MAX_BOXES];
  int calibration[MAX_BOXES][MAX_BOXES];
  double position[MAX_BOXES][3];
  double xPos, yPos;
  int i, j;
  int check;
  int dist;

  file = init_i2c( adapter_nr );

  for ( i = 0; i <= MAX_I2C; i++ )
    {
      if ((num = check_slave( file, i )) >= 0)
	{
	  if (total == MAX_BOXES)
	    {
	      printf( "main: too many boxes found! "
		      "(number %d was tether %d).\n"
		      " if you have this many, "
		      "you must recompile the code with a bigger MAX_BOXES\n",
		      MAX_BOXES + 1, i );
	      close( file );
	      exit( 1 );
	    }

	  slaves[total++] = i;
	  if (num != 0)
	    {
	      printf( "main: tether %d is reading %d, reset?\n", i, num );
	    }
	  printf( "Found tether %d\n", i );
	}
    }

  printf( "Total: %d tethers\n", total );

  printf( "Calibration: now hook all of the leads together and\n"
	  "             pull them at least %dmm from the nearest tether\n",
	  MIN_CALIBRATE );

  check = 0;
  while ( check < total )
    {
      check = 0;
      for ( i = 0; i < total; i++ )
	{
	  if ((calibration[i][i] = get_slave( file, slaves[i] )) >
	      MIN_CALIBRATE)
	    check++;
	}
    }

  printf( "Calibration: now pull the leads within %d mm of all the tethers\n",
	  MAX_ACCEPT );

  check = 0;
  while ( check < total )
    {
      for ( i = 0; i < total; i++ )
	{
	  if ((dist = get_slave( file, slaves[i] )) <= MAX_ACCEPT)
	    {

	      if (calibration[i][i] > MAX_ACCEPT)
		{
		  check++;
		  printf( "Calibration: got tether %d\n", slaves[i] );
		}

	      if (dist < calibration[i][i])
		{
		  for (j = 0; j < total; j++)
		    {
		      calibration[j][i] = get_slave( file, slaves[j] );
		    }
		  if (calibration[i][i] > MAX_ACCEPT)
		    {
		      printf( "Oops - lost %d, try again\n", slaves[i] );
		    }
		}
	    }
	}
    }
  printf( "Calibration: okay, got all of the tethers, calibrating...\n" );

  for ( i = 0; i < total; i++ )
    {
      printf( "%d\t", get_slave( file, slaves[i] ) );
    }
  printf( "\n" );

  close( file );
  
  return 0;
}
