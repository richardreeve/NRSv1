// low interface header file
// defines communication interface for simulator
// $Id: low_interface.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __LOW_INTERFACE_H
#define __LOW_INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

#ifndef __RTL__

#include "peripheral.h"
#include "interface.h"

extern InterfaceType Low_interface_type;

#endif //ndef __RTL__

// Serial stuff
#define SERIAL_DEVICE "/dev/ttyS%d"
#define KOALA_BAUD B115200
#define KHEP_BAUD B38400
#define KHEP_BAUD2 B57600
#define KOALA_BAUD_NUM 115200
#define KHEP_BAUD_NUM 38400
#define KHEP_BAUD_NUM2 57600
#define SERIAL_PORT 0

typedef enum {
  LOW_ADD_MOTOR=0,
  LOW_SUB_MOTOR,
  LOW_ADD_SENSOR,
  LOW_SUB_SENSOR,
  LOW_RESET,
  LOW_QUIT,
  LOW_UPDATE_MOTORS,
  LOW_UPDATE_MOTORS_OLD, // for direct serial comms code
  LOW_MOTORS_STOP,
  LOW_ACQUIRE_SENSORS,
  LOW_TICK,
  LOW_PING,
  LOW_PREPARE,
  LOW_PREPARE_OLD // for direct serial comms code
} LowCommands;

#ifndef __RTL__

// SIM (HIGH) SIDE CALLS


BOOL low_send_uchar(UCHAR uc);
BOOL low_send_ints(INT length ,INT *data);
BOOL low_send_chars(INT length ,CHAR *data);
BOOL low_receive_chars(INT length, CHAR *data);
BOOL low_receive_ints(INT length, INT *data);
UCHAR low_receive_uchar();
// add motors/sensors
BOOL low_peripheral_add( BOOL input, PeripheralType type,
			 VarArg arg, INT* buff );
// remove motors/sensors
BOOL low_peripheral_remove( BOOL input, INT ref );
// set parameters for sensors
BOOL low_sensors_set_params(INT ref, VarArg arg);
// send reset
BOOL low_call_reset();
// check listening
BOOL low_ping();

// stop motors
void low_motors_stop();

// end game
BOOL low_call_quit();
// take new motor input - old direct serial code
BOOL low_motors_update_old(INT length, CHAR *data, INT num_inputs );
// send new sensor output
void low_sensors_acquire( CHAR *data);

// Prepare robot for start (set ticks to zero)
void low_prepare();
// Send tick count from motors
void low_send_ticks();

// FIFO specific commands

// Tell fifo the period of the clock in microseconds
void low_fifo_period( LONG period );
// Tell fifo to start
void low_fifo_start();
// Tell fifo to stop
void low_fifo_stop();
// Get ints from SENSOR_OUT
BOOL low_receive_fifo_chars(INT length, CHAR *data);
// take new motor input
BOOL low_motors_update_fifo(INT length, CHAR *data);

// ROBOT (LOW) SIDE CALLS

// start and run sensor side of interface
BOOL low_handle_run(InterfaceType type, void *params);

#endif //ndef __RTL__

#ifdef __cplusplus
  };
#endif

#endif //ndef __LOW_INTERFACE_H
