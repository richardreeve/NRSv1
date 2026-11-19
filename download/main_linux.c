// basic test bed for model.[ch]
// $Id: main_linux.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "base.h"
#include "peripheral.h"
#include "sensor.h"
#include "NodeConversion.h"
#include "user_interface.h"

BOOL Repeat_main;
BOOL Has_tracker;     // Are we using an external tracker
BOOL Ticker;      // Are we using the internal tracker
LONG Ticker_gap;  // Gap between tick readings
INT  Display_level;
INT Connection=CON_KOALA;
INT ConnectionSpeed=0;
char DownloadName[NAME_LEN];
InterfaceType Default_user_conn;
InterfaceType Default_sensor_conn;

BOOL check_fifo();

int main(int argc, char** argv)
{
  node_initialise( "" );
  Repeat_main=TRUE;
  Display_level=0;
  Default_user_conn=INTERFACE_PORT;
  //Default_sensor_conn=INTERFACE_SERIAL;
  if (check_fifo())
    {
      printf("Using real-time serial link\n");
      Default_sensor_conn=INTERFACE_FIFO;
    }
  else
    {
      printf("No real-time serial link present, using default serial\n");
      Default_sensor_conn=INTERFACE_SERIAL;
    }
  /*#ifdef RTL_PRESENT
    Default_sensor_conn=INTERFACE_FIFO;
    #else
    printf("No real-time serial link present, using default serial\n");
    Default_sensor_conn=INTERFACE_SERIAL;
    #endif
  */
  Has_tracker=FALSE;
  Ticker=FALSE;
  Ticker_gap=100000;

  peripheral_initialise();
  sensor_initialise();

  sprintf(DownloadName, "tell");

  while (argc>1)
    {
      if ((!strcmp(argv[1],"-r")) || (!strcmp(argv[1],"--robot")))
	{
	  if (!strcmp(argv[2],"koala"))
	    Connection=CON_KOALA;
	  else if (!strcmp(argv[2],"khepera"))
	    Connection=CON_KHEP;
	  else if (!strcmp(argv[2],"sim"))
	    Default_sensor_conn=INTERFACE_DIRECT;
	  else
	    {
	      printf("main: robot not recognised\n");
	      exit(1);
	    }
	  argc-=2;
	  argv+=2;
	}
      else if ((!strcmp(argv[1],"-f")) || (!strcmp(argv[1],"--filename")))
	{
	  strcpy(DownloadName,argv[2]);
	  argc-=2;
	  argv+=2;
	}
      else if ((!strcmp(argv[1],"-s")) || (!strcmp(argv[1],"--speed")))
	{
	  if (!strcmp(argv[2],"koala"))
	    ConnectionSpeed=CON_KOALA;
	  else if (!strcmp(argv[2],"khepera"))
	    ConnectionSpeed=CON_KHEP;
	  else
	    {
	      printf("main: robot speed not recognised\n");
	      exit(1);
	    }
	  argc-=2;
	  argv+=2;
	}
/*        else if ((!strcmp(argv[1],"-u")) || (!strcmp(argv[1],"--uiname"))) */
/*  	{ */
/*  	  strcpy(Port_hostname,argv[2]); */
/*  	  argc-=2; */
/*  	  argv+=2; */
/*  	} */
/*        else if ((!strcmp(argv[1],"-t")) || (!strcmp(argv[1],"--tracker"))) */
/*  	{ */
/*  	  Has_tracker=TRUE; */
/*  	  if ((argc>2) && (argv[2][0]!='-')) // computer name for tracking */
/*  	    { */
/*  	      strcpy(Track_hostname,argv[2]); */
/*  	      argc-=2; */
/*  	      argv+=2; */
/*  	    } */
/*  	  else // use default */
/*  	    { */
/*  	      argc--; */
/*  	      argv++; */
/*  	    }	       */
/*  	} */
/*        else if ((!strcmp(argv[1],"-T")) || (!strcmp(argv[1],"--Ticker"))) */
/*  	{ */
/*  	  Ticker=TRUE; */
/*  	  if ((argc>2) && (argv[2][0]!='-')) // computer name for tracking */
/*  	    { */
/*  	      Ticker_gap=atoi(argv[2])*1000; */
/*  	      argc-=2; */
/*  	      argv+=2; */
/*  	    } */
/*  	  else // use default */
/*  	    { */
/*  	      argc--; */
/*  	      argv++; */
/*  	    }	       */
/*  	} */
      else
	{
	  if (!((!strcmp(argv[1],"-h")) || (!strcmp(argv[1],"--help"))))
	    printf("main: error in arguments to neuron.linux\n");
	  printf("download.linux: downloads software to robot\n");
	  printf("[ -r <robot_type> ] koala, khepera\n");
	  printf("[ -f <filename> ] filename of program (default %s)\n",
                 DownloadName);
	  printf("[ -s <robot_type> ] serial speed - koala or khepera\n");
/*  	  printf("[ -u <ui_name> ] machine user interface is running on\n"); */
/*  	  printf("[ -t [<tracker_name>] ] use tracker,  */
/*  with machine tracker is running on\n"); */
/*  	  printf("[ -T [ <time_gap> ] use tick counter on robot, */
/*  with inter tick gap in ms\n"); */
	  exit(1);
	}
    }

  if (ConnectionSpeed==0)
    ConnectionSpeed=Connection;

  call_start(LEVEL_SENSOR, Default_sensor_conn );
#ifdef DEBUG
  printf("download.linux ready to run\n");
#endif //def DEBUG
  //  user_handle_run(Default_user_conn, NULL);
  return 0;
}
