// fifo interface header file
// defines communication interface for simulator
// $Id: fifo_interface.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __FIFO_INTERFACE_H
#define __FIFO_INTERFACE_H

#include "low_interface.h"

#define FIFO_DEVICE "/dev/rtf%d"
// The four fifo pipes which communicate betweeen the rtl and the linux code
#define FIFO_COMMAND 1
#define SENSOR_OUT 2
#define SERIAL_COMMAND 3
#define SERIAL_REPLY 4

typedef enum {
  FIFO_INIT=0, // Download program to robot
  FIFO_START,  // Start timer
  FIFO_STOP,   // Stop timer
  FIFO_PERIOD, // Set period of timer
  // FIFO_TICK=LOW_TICK, // Get tick count
  FIFO_MOTOR=LOW_UPDATE_MOTORS // Update motors
} FifoCommands;

#define PERIOD_SCALE 1000 // Allows 1 us units as rest of code

#endif //ndef __FIFO_INTERFACE_H




