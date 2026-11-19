// fifo interface file
// handles communication between robot and simulator
// $Id: fifo_serial.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#include <rtl.h>
#include <rtl_fifo.h>
#include <rt_com.h>
#include <semaphore.h>
#include <rtl_printf.h>
#include <rtl_sched.h>
#include <rtl_time.h>

#include <time.h>
#include <pthread.h>
#include <asm/io.h>
#include <unistd.h>
#include <string.h>
#include "base.h"
#include "peripheral.h"
#include "low_interface.h"
#include "fifo_interface.h"

long period=1000000;
#define FIFO_SIZE 4000
int num_sensor_bytes=3;
int serial_speed=KOALA_BAUD_NUM;
int serial_port=SERIAL_PORT;
int stop_bits=2;

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Richard Reeve <r.e.reeve@stir.ac.uk>");
MODULE_DESCRIPTION("A fifo to serial port realtime module to talk to k-team robots");
MODULE_PARM(serial_speed,"i");
MODULE_PARM_DESC(serial_speed, "Serial port speed: "
		 "Koala speed is 115200, Khepera is 38400");
MODULE_PARM(serial_port,"0-1i");
MODULE_PARM_DESC(serial_port,"serial port number (0 or 1)");

pthread_t tasks[4];
#define GET_SENSOR 0
#define FIFO_COMMAND 1
#define NORMAL_COMMAND 2
#define ACTIVE_TIMER 3

// was 10, 30, 1, 20, 1000000?
#define GET_SENSOR_PRIO 10
#define FIFO_COMMAND_PRIO 30
#define NORMAL_COMMAND_PRIO 20
#define ACTIVE_TIMER_PRIO 40
#define MUTEX_OVERRIDE_PRIO 1000

sem_t semaphores[5];
#define SEM_FIFO 0
#define SEM_NORMAL_COMMAND 1
#define SEM_TIMER 2
#define SEM_SERIAL 3
#define SEM_OUT_SERIAL 4

#define LONG_TIME 1000
#define SHORT_TIME 100
#define V_SHORT_TIME 10


pthread_mutex_t mutex_have_serial;
pthread_mutex_t mutex_start_timer;

void *get_sensor( void *p ) 
{
  char get_sensor_command = (char) LOW_ACQUIRE_SENSORS;
  int i = 0;
  char buff[PERIPHERALS_MAX];
  char *buff_ptr = buff;
  int num;

  while (TRUE)
    {
      sem_wait( &semaphores[SEM_TIMER] );
      pthread_mutex_lock( &mutex_have_serial );
      rt_com_write( serial_port, &get_sensor_command, 1 );

      while (rt_com_read( serial_port, buff + 1, 1 ) < 1)
	{
	  sem_wait( &semaphores[SEM_SERIAL] );
	}

      i = 1;

      if ((buff[1] & 1) == 1)
	{
	  num_sensor_bytes = (INT) (((UCHAR) buff[1] - 1) >> 1);
	  num = num_sensor_bytes + 1;
	  buff_ptr = buff + 1;
	}
      else
	{
	  buff[0] = (CHAR) (num_sensor_bytes << 1);
	  num = num_sensor_bytes;
	  buff_ptr = buff;
	}

      while (i < num)
	{
	  i+=rt_com_read( serial_port, buff + i + 1, num - i );
	  if (i < num)
	    sem_wait( &semaphores[SEM_SERIAL] );
	}

      pthread_mutex_unlock( &mutex_have_serial );
      sem_post( &semaphores[SEM_SERIAL] );
      rtf_put( SENSOR_OUT, buff_ptr, num_sensor_bytes + 1 );
    }
}

void *fifo_command(void *p)
{
  int i=0;
  int num;
  unsigned char buff[BUFF_MAX];
  unsigned char val;
  int started=FALSE;
  FifoCommands fc;
  char msg[BUFF_MAX];
  char boot[BUFF_MAX];
  char boot2[BUFF_MAX];
  char reply[BUFF_MAX];
  int pos=0;
  int pos2=0;
  int pos3=0;
  int length;
  int length2;
  int length3;
  LONG myLONG;

  // Stop timer...
  pthread_mutex_lock(&mutex_start_timer);
  sprintf(boot,ROBOT_BOOT);
  length=strlen(boot);
  sprintf(boot2,ROBOT_BOOT_SCP);
  length2=strlen(boot2);

  while (TRUE)
    {
      sem_wait(&semaphores[SEM_FIFO]); // Hang until signal
      if (rtf_get(FIFO_COMMAND, &val, 1)>0) // There is something waiting
	{
	  fc=(FifoCommands)val;
	  switch (fc)
	    {
	    case FIFO_INIT:
	      while (rtf_get(FIFO_COMMAND, &val, 1)==0)
		sem_wait(&semaphores[SEM_FIFO]);
	      if (val==CON_KOALA)
		{
		  sprintf(reply,ROBOT_MESSAGE_L,"koala");
		}
	      else if (val==CON_KHEP)
		{
		  sprintf(reply,ROBOT_MESSAGE_L,"khepera");
		}
	      else
		{
		  rtl_printf("fifo_command: error FIFO_INIT type set to %d\n",
			     (int)val);
		  if (serial_speed==KOALA_BAUD_NUM)
		    sprintf(reply,ROBOT_MESSAGE_L,"koala");
		  else
		    sprintf(reply,ROBOT_MESSAGE_L,"khepera");
		}
	      length3 = strlen( reply );
	      while (rtf_get(FIFO_COMMAND, &val, 1)==0)
		sem_wait(&semaphores[SEM_FIFO]);
	      if (val==CON_KOALA)
		{
		  serial_speed=KOALA_BAUD_NUM;
		  stop_bits=2;
		}
	      else if (val==CON_KHEP)
		{
		  serial_speed=KHEP_BAUD_NUM;
		  stop_bits=1;
		}
	      else
		{
		  rtl_printf("fifo_command: error FIFO_INIT speed set to %d\n",
			     (int)val);
		}
	      pthread_mutex_lock(&mutex_have_serial);
	      // Set up serial port
	      rt_com_setup(serial_port, -1, 0, 0, 0);
	      rt_com_setup(serial_port, serial_speed, RT_COM_PARITY_NONE,
			   stop_bits, 8);
	      // Now wait for the reply
	      pos=0;
	      pos2=0;
	      pos3=0;
	      rtl_printf("fifo_serial: FIFO_INIT ready for robot boot\n");
	      while ((pos<length) && (pos2<length2) && (pos3<length3))
		{
		  sem_wait(&semaphores[SEM_SERIAL]);
		  if (rt_com_read(serial_port, &val, 1)==1)
		    {
		      if (val==boot[pos])
			  pos++;
		      else
			{
			  pos=0;
			  if (val==boot[pos])
			    pos++;
			}
		      if (val==boot2[pos2])
			  pos2++;
		      else
			{
			  pos2=0;
			  if (val==boot2[pos2])
			    pos2++;
			}
		      if (val==reply[pos3])
			  pos3++;
		      else
			{
			  pos3=0;
			  if (val==reply[pos3])
			    pos3++;
			}
		      rtf_put(SERIAL_REPLY, &val, 1);
		      sem_post(&semaphores[SEM_SERIAL]);
		    }
		  //		  rtl_printf("fifo_serial: waiting for serial post %d/%d\n", pos, length);
		}

	      if (pos2==length2) // flash on koala, as before on khepera
		{
		  if (serial_speed==KOALA_BAUD_NUM)
		    sprintf(msg, SCP_MSG_KOA);
		  else
		    sprintf(msg, SCP_MSG_KHE);
		  rtf_put(SERIAL_REPLY, msg, strlen(msg));
		  if (serial_speed==KOALA_BAUD_NUM)
		    sprintf(msg, SCP_PRE_KOA);
		  else
		    sprintf(msg, SCP_PRE_KHE);
		  rt_com_write( serial_port, msg, strlen(msg) );
		  // Now wait for the reply
		  pos=0;
		  rtl_printf("fifo_serial: FIFO_INIT ready for robot boot\n");
		  while (pos<length)
		    {
		      sem_wait(&semaphores[SEM_SERIAL]);
		      if (rt_com_read(serial_port, &val, 1)==1)
			{
			  if (val==boot[pos])
			    pos++;
			  else
			    {
			      pos=0;
			      if (val==boot[pos])
				pos++;
			    }
			  rtf_put(SERIAL_REPLY, &val, 1);
			  sem_post(&semaphores[SEM_SERIAL]);
			}
		    }
		}

	      // And add confirmation message
	      if ((pos==length) || (pos2==length2))
		{
		  sprintf(msg,READY_FOR_CODE);
		  rtf_put(SERIAL_REPLY, msg, strlen(msg));
		  
		  rtl_printf("fifo_serial: FIFO_INIT ready for transmit\n");
		  // Now transmit program
		  // comes in chunks of up to BUFF_MAX bytes
		  // preceded by size
		  // final chunk is size 0
		  // ie BUFF_MAX xx...xx BUFF_MAX xx...xx 2 xx 0
		  pos = rtf_get( FIFO_COMMAND, &length, sizeof( int ) );
		  while (pos < sizeof( int ))
		    {
		      sem_wait( &semaphores[SEM_FIFO] );
		      pos += rtf_get( FIFO_COMMAND, &length + pos,
				      sizeof( int ) - pos );
		    }
		  rtl_printf( "fifo_serial: Printing %d bytes\n", length );
		  while (length != 0)
		    {
		      i=0;
		      while (i < length)
			{
			  i += rtf_get( FIFO_COMMAND, buff + i, length - i );
			  if (i < length)
			    sem_wait( &semaphores[SEM_FIFO] );
			}
		      pos = 0;
		      while (pos < length)
			{
			  while (!(rt_com_out_empty( serial_port )))
			    sem_wait( &semaphores[SEM_OUT_SERIAL] );
			  i = ( length > pos + 255 ? 255 : length - pos );
			  rt_com_write( serial_port, buff + pos, i );
			  pos += i;
			}
		      pos = rtf_get( FIFO_COMMAND, &length, sizeof( int ) );
		      while (pos < sizeof( int ))
			{
			  sem_wait( &semaphores[SEM_FIFO] );
			  pos += rtf_get( FIFO_COMMAND, (void*)(&length) + pos,
					  sizeof( int ) - pos );
			}
		      
		      if (length != BUFF_MAX)
			rtl_printf( "fifo_serial: Printing %d bytes\n",
				    length );
		    }

		  if (pos2==length2) // flash on koala, as before on khepera
		    {
		      rtl_printf("Sending user-flash commands\n");
		      if (serial_speed==KOALA_BAUD_NUM)
			sprintf(msg, SCP_POST_KOA);
		      else
			sprintf(msg, SCP_POST_KHE);
		      rt_com_write( serial_port, msg, strlen(msg) );
		      rtl_printf(msg);
		    }

		  while (!(rt_com_out_empty( serial_port )))
		    sem_wait( &semaphores[SEM_OUT_SERIAL] );
		  // Now wait for confirmation of download
		  rtl_printf( "fifo_serial: "
			      "FIFO_INIT waiting for confirmation "
			      "of download\n" );
		  // If a khepera we need to change speed!
		  if (serial_speed==KHEP_BAUD_NUM)
		    {
                      usleep(100000);
		      serial_speed=KHEP_BAUD_NUM2;
		      rt_com_setup( serial_port, -1, 0, 0, 0 );
		      rt_com_setup( serial_port, serial_speed,
				    RT_COM_PARITY_NONE, stop_bits, 8 );
		    }
		  pos = 0;
		  while (pos < length3)
		    {
		      sem_wait( &semaphores[SEM_SERIAL] );
		      if (rt_com_read( serial_port, &val, 1 )==1)
			{
			  if (val==reply[pos])
			    pos++;
			  else
			    pos=0;
			  rtf_put(SERIAL_REPLY, &val, 1);
			  sem_post(&semaphores[SEM_SERIAL]);
			}
		    }
		  
		  pthread_mutex_unlock(&mutex_have_serial);
		  // And send confirmation message
		  rtl_printf("fifo_serial: FIFO_INIT got confirmation\n");
		}
	      sprintf(msg,DOWNLOAD_COMPLETE);
	      rtf_put(SERIAL_REPLY, msg, strlen(msg));
	      break;
	    case FIFO_START:
	      // Set timer going
	      if (started==FALSE)
		{
		  val=(unsigned char)LOW_PREPARE;
		  //rtl_printf("fifo_serial: "
		  //"Got FIFO_START, waiting for mutex\n");
		  pthread_mutex_lock(&mutex_have_serial);
		  //rtl_printf("fifo_serial: Got mutex, writing\n");
		  rt_com_write(serial_port, &val, 1);
		  rtl_printf("fifo_serial: "
			     "Waiting for acknowledge (num sensors)\n");
		  while (rt_com_read(serial_port, &val, 1)==0)
		    sem_wait(&semaphores[SEM_SERIAL]);
		  sem_post(&semaphores[SEM_SERIAL]);

		  num_sensor_bytes=(int)val;
		  rtl_printf("fifo_serial: %d bytes of sensors\n",
			     num_sensor_bytes);
		  pthread_mutex_unlock(&mutex_have_serial);
		  pthread_make_periodic_np(tasks[ACTIVE_TIMER], gethrtime(),
					   period );
		  rtl_printf("fifo_serial: made timer periodic\n");
		  started=TRUE;
		}
	      else
		{
		  rtl_printf("fifo_command: FIFO_START - already started\n");
		}
	      break;
	    case FIFO_STOP:
	      // Stop timer
	      rtl_printf("fifo_command: Received stop...\n");
	      if (started==TRUE)
		{
		  pthread_make_periodic_np(tasks[ACTIVE_TIMER], 
					   HRTIME_INFINITY, period);
		  // Remove all remaining timer signals
		  i=0;
		  while (sem_trywait(&semaphores[SEM_TIMER])==0)
		    i++;
		  if (i>0)
		    rtl_printf("fifo_command: FIFO_STOP - %d unused timers\n",
			       i);
		  // And get rid of unused data
		  rtf_flush(SENSOR_OUT);
		  started=FALSE;
		}
	      else
		rtl_printf("fifo_command: repeated stop\n");
	      break;
	    case FIFO_PERIOD:
	      // Change period of sampling
	      rtl_printf("fifo_serial: Received period...\n");
	      length = rtf_get(FIFO_COMMAND, &myLONG, sizeof(LONG));
	      while (length < sizeof(LONG))
		{
		  sem_wait(&semaphores[SEM_FIFO]);
		  length += rtf_get(FIFO_COMMAND, &myLONG + length,
				    sizeof(LONG) - length);
		}
	      myLONG *= PERIOD_SCALE;
	      if (period != myLONG)
		{
		  period = myLONG;
		  if (started)
		    {
		      // Why do we want to do this?
		      // Anyway, we'll have to change frequency on the run
		      pthread_make_periodic_np(tasks[ACTIVE_TIMER], 
					       gethrtime(), period);
		      rtl_printf("fifo_command: FIFO_PERIOD - changed period "
				 "while active to %ld! Why?\n", period);
		    }
		}
	      break;
	    case FIFO_MOTOR:
	      // send number of motors
	      while (rtf_get( FIFO_COMMAND, &val, 1 ) == 0)
		sem_wait( &semaphores[SEM_FIFO] );
	      sem_post( &semaphores[SEM_FIFO] );
	      num = 2 + (int) val;
	      i = 2;
	      buff[0] = (unsigned char) LOW_UPDATE_MOTORS;
	      buff[1] = (unsigned char) val;
	      while (i < num)
		{
		  i+=rtf_get( FIFO_COMMAND, buff + i, num - i );
		  if (i < num)
		    sem_wait( &semaphores[SEM_FIFO] );
		}
	      pthread_mutex_lock( &mutex_have_serial );
	      rt_com_write( serial_port, buff, num );
#ifdef DEBUG
	      i=1;
	      while (i<num)
		{
		  rtl_printf( "(%d=%d) ", i, (int) buff[i] );
		  i++;
		}
	      rtl_printf( "\n" );
#endif //def DEBUG
	      pthread_mutex_unlock(&mutex_have_serial);
	      break;
	    }
	  // Is this ever necessary?
	  sem_post( &semaphores[SEM_FIFO] );
	}
      else
	{
	  // Too many fifo calls - ignore
	}
    }
}


void *normal_command(void *p) 
{
  int i=0;
  unsigned char buff[255];
  int in=0;
  int out=0;
  int have_lock=FALSE;
  int nums;

  while (TRUE)
    {
      sem_wait(&semaphores[SEM_NORMAL_COMMAND]); // Hang until signal
      nums = 2;
      i = rtf_get( SERIAL_COMMAND, buff, nums );
      if (i > 0) // There is something waiting
	{
	  //rtl_printf( "normal_command: Got something %d bytes - first %d\n",
	  //		      i, (int)buff[0]);
	  while (i < nums)
	    {
	      i += rtf_get( SERIAL_COMMAND, buff + i, nums - i );
	      if (i < nums)
		sem_wait( &semaphores[SEM_NORMAL_COMMAND] );
	    }
	  in = (int) buff[0];
	  out = (int) buff[1];
	  //	  rtl_printf("normal_command: Waiting for %d in and %d back\n",
	  //                 in, out);
	  if ((in==0)&&(out==0)) // End transmission signal
	    {
	      if (have_lock)
		{
		  have_lock=FALSE;
		  pthread_mutex_unlock(&mutex_have_serial);
		}
	    }
	  else
	    {
	      if (!have_lock)
		{
		  pthread_mutex_lock( &mutex_have_serial );
		  have_lock=TRUE;
		}
	      if (in > 0)
		{
		  nums = in;
		  i = 0;
		  //rtl_printf( "normal_command: Getting %d\n", in );
		  while (i < nums)
		    {
		      i += rtf_get( SERIAL_COMMAND, buff + i, nums - i );
		      //rtl_printf( "normal_command: Got %d\n", i );
		      if (i < nums)
			sem_wait( &semaphores[SEM_NORMAL_COMMAND] );
		    }
		  rt_com_write( serial_port, buff, in );
		  //		  rtl_printf("normal_command: Done in\n");
		}
	      if (out > 0)
		{
		  nums = out;
		  i = 0;
		  //rtl_printf( "normal_command: Doing out\n" );
		  while (i < nums)
		    {
		      i += rt_com_read( serial_port, buff + i, nums - i );
		      //rtl_printf( "normal_command: tried to read, got %d\n",
		      //		  i );
		      if (i < nums)
			sem_wait( &semaphores[SEM_SERIAL] );
		    }
		  sem_post( &semaphores[SEM_SERIAL] );
		  //rtl_printf("normal_command: Received out\n");

		  rtf_put( SERIAL_REPLY, buff, out );
		  //rtl_printf( "normal_command: Received %d %d\n",
		  //	     (int) buff[0], (int) buff[1] );
		}
	      sem_post( &semaphores[SEM_NORMAL_COMMAND] );
	    }
	}
      else
	{
	  // Blank - ignore, as we generate too many posts
	}
    }
}


void *active_timer(void *p)
{
  pthread_make_periodic_np(tasks[ACTIVE_TIMER], HRTIME_INFINITY, period);
  while (TRUE)
    {
      pthread_wait_np();
      //rtl_printf("Active_timer: activated\n");
      sem_post(&semaphores[SEM_TIMER]);
      //      pthread_wakeup_np(tasks[GET_SENSOR]);
    }
  return 0;
}

int fifo_command_handler(unsigned int f)
{
  sem_post(&semaphores[SEM_FIFO]);
  pthread_wakeup_np(tasks[SEM_FIFO]);
  return 0;
}

int serial_command_handler(unsigned int f)
{
  sem_post(&semaphores[SEM_NORMAL_COMMAND]);
  return 0;
}

int init_module(void)
{
  pthread_attr_t attr;
  struct sched_param sched_param;
  int status, i;

  rt_com_setup(serial_port, serial_speed, RT_COM_PARITY_NONE, stop_bits, 8);

  if (rtf_create(FIFO_COMMAND, FIFO_SIZE)<0) // Start/stop from linux to rtl
    {
      rtl_printf("fifo_serial: failed to create FIFO_COMMAND RT-fifo\n");
      return -1;
    }

  if (rtf_create(SENSOR_OUT, 1000000)<0) // Sensors from robot to linux
    {
      rtl_printf("fifo_serial: failed to create SENSOR_OUT RT-fifo\n");
      return -1;
    }

  if (rtf_create(SERIAL_COMMAND, FIFO_SIZE)<0) // Commands from linux to robot
    {
      rtl_printf("fifo_serial: failed to create SERIAL_COMMAND RT-fifo\n");
      return -1;
    }

  if (rtf_create(SERIAL_REPLY, FIFO_SIZE)<0) // Replies from robot to linux
    {
      rtl_printf("fifo_serial: failed to create SERIAL_REPLY RT-fifo\n");
      return -1;
    }

  sem_init(&semaphores[SEM_FIFO], 0, 0);
  sem_init(&semaphores[SEM_NORMAL_COMMAND], 0, 0);
  sem_init(&semaphores[SEM_TIMER], 0, 0);
  sem_init(&semaphores[SEM_SERIAL], 0, 0);
  sem_init(&semaphores[SEM_OUT_SERIAL], 0, 0);
  rt_com_set_input_semaphore(serial_port, &semaphores[SEM_SERIAL]);
  rt_com_set_output_semaphore(serial_port, &semaphores[SEM_OUT_SERIAL]);

  pthread_mutex_init(&mutex_have_serial, NULL);
  status=pthread_mutex_setprioceiling(&mutex_have_serial, 
				      MUTEX_OVERRIDE_PRIO, &i);
  if (status != 0)
    {
      rtl_printf("fifo_serial: falied to set priority ceiling\n");
      return -1;
    }

  pthread_attr_init (&attr);
  sched_param.sched_priority = GET_SENSOR_PRIO;
  pthread_attr_setschedparam (&attr, &sched_param);
  status = pthread_create (&tasks[GET_SENSOR],
			   &attr, get_sensor, NULL);
  if (status != 0) 
    {
      rtl_printf("fifo_serial: failed to create GET_SENSOR RT-thread\n");
      return -1;
    }
  else 
    {
      rtl_printf("fifo_serial: created GET_SENSOR RT-thread\n");
    }
  
  pthread_attr_init(&attr);
  sched_param.sched_priority = FIFO_COMMAND_PRIO;
  pthread_attr_setschedparam(&attr, &sched_param);
  status = pthread_create(&tasks[FIFO_COMMAND],
			  &attr, fifo_command, NULL);
  if (status != 0) 
    {
      rtl_printf("fifo_serial: failed to create FIFO_COMMAND RT-thread\n");
      return -1;
    }
  else 
    {
      rtl_printf("fifo_serial: created FIFO_COMMAND RT-thread\n");
    }

  pthread_attr_init(&attr);
  sched_param.sched_priority = NORMAL_COMMAND_PRIO;
  pthread_attr_setschedparam(&attr, &sched_param);
  status = pthread_create(&tasks[NORMAL_COMMAND],
			  &attr, normal_command, NULL);
  if (status != 0) 
    {
      rtl_printf("fifo_serial: failed to create NORMAL_COMMAND RT-thread\n");
      return -1;
    }
  else 
    {
      rtl_printf("fifo_serial: created NORMAL_COMMAND RT-thread\n");
    }
  
  pthread_attr_init(&attr);
  sched_param.sched_priority = ACTIVE_TIMER_PRIO;
  pthread_attr_setschedparam(&attr, &sched_param);
  status = pthread_create(&tasks[ACTIVE_TIMER],
			  &attr, active_timer, NULL);
  if (status != 0) 
    {
      rtl_printf("fifo_serial: failed to create ACTIVE_TIMER RT-thread\n");
      return -1;
    }
  else 
    {
      rtl_printf("fifo_serial: created ACTIVE_TIMER RT-thread\n");
    }

  rtf_create_handler(FIFO_COMMAND, &fifo_command_handler);
  rtf_create_handler(SERIAL_COMMAND, &serial_command_handler);

  return 0;
}

void cleanup_module(void)
{
  rtl_printf ("Removing serial->fifo interface module\n");

  pthread_delete_np(tasks[GET_SENSOR]);
  pthread_delete_np(tasks[FIFO_COMMAND]);
  pthread_delete_np(tasks[NORMAL_COMMAND]);
  pthread_delete_np(tasks[ACTIVE_TIMER]);

  sem_destroy(&semaphores[SEM_FIFO]);
  sem_destroy(&semaphores[SEM_NORMAL_COMMAND]);
  sem_destroy(&semaphores[SEM_TIMER]);

  rt_com_clear_semaphores(serial_port);
  sem_destroy(&semaphores[SEM_SERIAL]);
  sem_destroy(&semaphores[SEM_OUT_SERIAL]);

  pthread_mutex_destroy(&mutex_have_serial);

  rtf_destroy(FIFO_COMMAND);
  rtf_destroy(SENSOR_OUT);
  rtf_destroy(SERIAL_COMMAND);
  rtf_destroy(SERIAL_REPLY);

  rt_com_setup(serial_port, -1, 0, 0, 0);
}

