// fifo interface file
// handles communication between robot and simulator
// $Id: mutex_fifo_serial.c,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#include <rtl.h>
#include <rtl_fifo.h>
#include <rt_com.h>
#include <semaphore.h>
#include <rtl_printf.h>
#include <pthread.h>
#include <rtl_sched.h>
#include <rtl_time.h>

#include <time.h>
#include <pthread.h>
#include <asm/io.h>
#include <unistd.h>
#include <string.h>
#include "base.h"
#include "sensor.h"
#include "low_interface.h"
#include "fifo_interface.h"

int period=1000000;
#define FIFO_SIZE 4000
int num_sensors=3;
int serial_speed=KOALA_BAUD_NUM;
int serial_port=SERIAL_PORT;
int stop_bits=2;

MODULE_PARM(serial_speed,"i");
MODULE_PARM_DESC(serial_speed,"Koala speed is 115200, Khepera is 38400");
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

sem_t semaphores[4];
#define SEM_FIFO 0
#define SEM_NORMAL_COMMAND 1
#define SEM_TIMER 2
#define SEM_SERIAL 3

#define LONG_TIME 1000
#define SHORT_TIME 100
#define V_SHORT_TIME 10


pthread_mutex_t mutex_have_serial;
pthread_mutex_t mutex_start_timer;

void *get_sensor(void *p) 
{
  char get_sensor_command=(char)LOW_ACQUIRE_SENSORS;
  int i=0;
  char buff[PERIPHERALS_MAX];
  int num;

  while (TRUE)
    {
      sem_wait(&semaphores[SEM_TIMER]);
      rtl_printf("get_sensor: waiting for mutex\n");
      pthread_mutex_lock(&mutex_have_serial);
      rtl_printf("get_sensor: got mutex\n");
      rt_com_write(serial_port, &get_sensor_command, 1);
      i=0;
      num=num_sensors*sizeof(INT);
      while (i<num)
	{
	  i+=rt_com_read(serial_port, buff+i, num-i);
	  if (i<num)
	    sem_wait(&semaphores[SEM_SERIAL]);
	}
      pthread_mutex_unlock(&mutex_have_serial);
      sem_post(&semaphores[SEM_SERIAL]);
      rtf_put(SENSOR_OUT, buff, num);
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
  char boot[BUFF_MAX];
  char reply[BUFF_MAX];
  int pos=0;
  int length;

  // Stop timer...
  pthread_mutex_lock(&mutex_start_timer);

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
		  serial_speed=KOALA_BAUD_NUM;
		  stop_bits=2;
		  sprintf(reply,ROBOT_MESSAGE_L,"koala");
		}
	      else if (val==CON_KHEP)
		{
		  serial_speed=KHEP_BAUD_NUM;
		  stop_bits=1;
		  sprintf(reply,ROBOT_MESSAGE_L,"khepera");
		}
	      else
		{
		  rtl_printf("fifo_command: error FIFO_INIT set to %d\n",
			     (int)val);
		  if (serial_speed==KOALA_BAUD_NUM)
		    sprintf(reply,ROBOT_MESSAGE_L,"koala");
		  else
		    sprintf(reply,ROBOT_MESSAGE_L,"khepera");
		}
	      sprintf(boot,ROBOT_BOOT);
	      pthread_mutex_lock(&mutex_have_serial);
	      // Set up serial port
	      rt_com_setup(serial_port, -1, 0, 0, 0);
	      rt_com_setup(serial_port, serial_speed, RT_COM_PARITY_NONE,
			   stop_bits, 8);
	      // Now wait for the reply
	      pos=0;
	      length=strlen(boot);
	      rtl_printf("fifo_serial: FIFO_INIT ready for robot boot\n");
	      while (pos<length)
		{
		  sem_wait(&semaphores[SEM_SERIAL]);
		  if (rt_com_read(serial_port, &val, 1)==1)
		    {
		      if (val==boot[pos])
			  pos++;
		      else
			pos=0;
		      rtf_put(SERIAL_REPLY, &val, 1);
		      sem_post(&semaphores[SEM_SERIAL]);
		    }
		  //		  rtl_printf("fifo_serial: waiting for serial post %d/%d\n", pos, length);
		}

	      // And add confirmation message
	      sprintf(boot,READY_FOR_CODE);
	      rtf_put(SERIAL_REPLY, boot, strlen(boot));

	      rtl_printf("fifo_serial: FIFO_INIT ready for transmit\n");
	      // Now transmit program
	      // comes in chunks of up to 255 bytes
	      // preceded by size
	      // final chunk is size 0
	      // ie 255 xxxxx 255 xxxxx 2 xx 0
	      while((length=rtf_get(FIFO_COMMAND, &val, 1))<1)
		sem_wait(&semaphores[SEM_FIFO]);
	      rtl_printf("fifo_serial: Printing %d bytes\n",(int)val);
	      while(val!=0)
		{
		  i=0;
		  while (i<(int)val)
		    {
		      i+=rtf_get(FIFO_COMMAND, buff+i, ((int)val)-i);
		      if (i<(int)val)
			sem_wait(&semaphores[SEM_FIFO]);
		    }
		  rt_com_write(serial_port, buff, (int)val);
		  // Massive delay to let serial port catch up!
		  if (serial_speed==KHEP_BAUD_NUM)
		    usleep(150000);
		  else
		    usleep(50000);
		  while((length=rtf_get(FIFO_COMMAND, &val, 1))<1)
		    sem_wait(&semaphores[SEM_FIFO]);
		  if (val!=255)
		    rtl_printf("fifo_serial: Printing %d bytes\n",(int)val);
		}
	      // Now wait for confirmation of download
	      rtl_printf("fifo_serial: FIFO_INIT waiting for confirmation\
of download\n");
	      // If a khepera we need to change speed!
	      if (serial_speed==KHEP_BAUD_NUM)
		{
		  serial_speed=KHEP_BAUD_NUM2;
		  rt_com_setup(serial_port, -1, 0, 0, 0);
		  rt_com_setup(serial_port, serial_speed, RT_COM_PARITY_NONE,
			       stop_bits, 8);
		}
	      pos=0;
	      length=strlen(reply);
	      while (pos<length)
		{
		  sem_wait(&semaphores[SEM_SERIAL]);
		  if (rt_com_read(serial_port, &val, 1)==1)
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
	      sprintf(reply,DOWNLOAD_COMPLETE);
	      rtf_put(SERIAL_REPLY, reply, strlen(reply));
	      break;
	    case FIFO_START:
	      // Set timer going
	      if (started==FALSE)
		{
		  val=(unsigned char)LOW_PREPARE;
		  rtl_printf("fifo_serial: Got FIFO_START, waiting for mutex\n");
		  pthread_mutex_lock(&mutex_have_serial);
		  rtl_printf("fifo_serial: Got mutex, writing\n");
		  rt_com_write(serial_port, &val, 1);
		  rtl_printf("fifo_serial: Waiting for acknowledge (num sensors)\n");
		  while (rt_com_read(serial_port, &val, 1)==0)
		    sem_wait(&semaphores[SEM_SERIAL]);
		  sem_post(&semaphores[SEM_SERIAL]);

		  num_sensors=(int)val;
		  rtl_printf("fifo_serial: %d sensors\n", num_sensors);
		  pthread_mutex_unlock(&mutex_have_serial);
		  pthread_make_periodic_np(tasks[ACTIVE_TIMER], gethrtime(),
					   period);
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
	      rtl_printf("fifo_serial: Received stop...\n");
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
		rtl_printf("fifo_command: FIFO_STOP - already stopped\n");
	      break;
	    case FIFO_PERIOD:
	      // Change period of sampling
	      rtl_printf("fifo_serial: Received period...\n");
	      while (rtf_get(FIFO_COMMAND,&val,1)==0)
		sem_wait(&semaphores[SEM_FIFO]);
	      num=PERIOD_SCALE*(int)val;
	      if (period!=num)
		{
		  period=num;
		  if (started)
		    {
		      // Why do we want to do this?
		      // Anyway, we'll have to change frequency on the run
		      pthread_make_periodic_np(tasks[ACTIVE_TIMER], 
					       gethrtime(), period);
		      rtl_printf("fifo_command: FIFO_PERIOD - changed period \
while active to %d! Why?\n", period);
		    }
		}
	      break;
	    case FIFO_MOTOR:
	      // send number of motors
	      while (rtf_get(FIFO_COMMAND,&val,1)==0)
		sem_wait(&semaphores[SEM_FIFO]);
	      sem_post(&semaphores[SEM_FIFO]);
	      num=1+(int)val;
	      i=1;
	      buff[0]=(unsigned char)LOW_UPDATE_MOTORS;
	      while (i<num)
		{
		  i+=rtf_get(FIFO_COMMAND, buff+i, num-i);
		  if (i<num)
		    sem_wait(&semaphores[SEM_FIFO]);
		}
	      pthread_mutex_lock(&mutex_have_serial);
	      rt_com_write(serial_port, buff, num);
	      rtl_printf(".");
	      i=1;
	      while (i<num)
	      {
		rtl_printf("(%d=%d) ",i,(int)buff[i]);
		i++;
	      }
	      rtl_printf("\n");
	      pthread_mutex_unlock(&mutex_have_serial);
	      break;
	    }
	  // Is this ever necessary?
	  sem_post(&semaphores[SEM_FIFO]);
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
      nums=2;
      i=rtf_get(SERIAL_COMMAND, buff, nums);
      if (i>0) // There is something waiting
	{
	  //rtl_printf("fifo_serial: Got something %d bytes - first %d\n", i, (int)buff[0]);
	  while (i<nums)
	    {
	      i+=rtf_get(SERIAL_COMMAND, buff+i, nums-i);
	      if (i<nums)
		sem_wait(&semaphores[SEM_NORMAL_COMMAND]);
	    }
	  in=(int)buff[0];
	  out=(int)buff[1];
	  rtl_printf("fifo_serial: Waiting for %d in and %d back\n", in, out);
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
		  pthread_mutex_lock(&mutex_have_serial);
		  have_lock=TRUE;
		}
	      if (in>0)
		{
		  nums=in;
		  i=0;
		  rtl_printf("fifo_serial: Getting %d\n", in);
		  while (i<nums)
		    {
		      i+=rtf_get(SERIAL_COMMAND, buff+i, nums-i);
		      rtl_printf("fifo_serial: Got %d\n", i);
		      if (i<nums)
			sem_wait(&semaphores[SEM_NORMAL_COMMAND]);
		    }
		  rt_com_write(serial_port, buff, in);
		  rtl_printf("fifo_serial: Done in\n");
		}
	      if (out>0)
		{
		  nums=out;
		  i=0;
		  rtl_printf("fifo_serial: Doing out\n");
		  while (i<nums)
		    {
		      i+=rt_com_read(serial_port, buff+i, nums-i);
		      if (i<nums)
			sem_wait(&semaphores[SEM_SERIAL]);
		    }
		  sem_post(&semaphores[SEM_SERIAL]);
		  rtl_printf("fifo_serial: Received out\n");

		  rtf_put(SERIAL_REPLY, buff, out);
		  rtl_printf("fifo_serial: Received %d %d\n", (int)buff[0], (int)buff[1]);
		}
	      sem_post(&semaphores[SEM_NORMAL_COMMAND]);
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
      rtl_printf("Active_timer: activated\n");
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
  rt_com_set_semaphore(serial_port, &semaphores[SEM_SERIAL]);

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

  rt_com_clear_semaphore(serial_port);
  sem_destroy(&semaphores[SEM_SERIAL]);

  pthread_mutex_destroy(&mutex_have_serial);

  rtf_destroy(FIFO_COMMAND);
  rtf_destroy(SENSOR_OUT);
  rtf_destroy(SERIAL_COMMAND);
  rtf_destroy(SERIAL_REPLY);

  rt_com_setup(serial_port, -1, 0, 0, 0);
}

