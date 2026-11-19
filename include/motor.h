// Motor model
// $Id: motor.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __MOTOR_H
#define __MOTOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "peripheral.h"

#define MOTOR_TYPES 3

// default number of motor updates per second
#define MOTOR_UPDATE_RATE 10
// Max motor signal
#define MAX_MOTOR_AMP 50

typedef enum {
  SIDE_LEFT=0,
  SIDE_RIGHT
} MotorSide;

typedef struct Motor {
  PeripheralType type;
  MotorSide side;
  INT ref;

  struct Motor *prev;
  struct Motor *next;
} Motor;

extern Motor *Motors;
extern INT Num_motors;

// Do we have this kind of sensor?
extern BOOL Has_motor[PERIPHERAL_TYPES];
// How many of them?
extern INT Num_motor[PERIPHERAL_TYPES];

// find motor from ref
Motor *find_motor( INT ref );
BOOL motors_change( PeripheralType type, INT change, INT* ref );
// Parameterise motors as appropriate
BOOL motors_set_params( INT ref, VarArg param, BOOL *bits, INT *num );
void motors_free();

// platform dependent
INT motors_activate( CHAR *motors );

  // Sets Has_motor
  void motor_initialise();

  // Doing per-timestep motor commands if necessary
  void motor_run_start();
  void motor_run_end();

#ifdef __cplusplus
  };
#endif

#endif//ndef __MOTOR_H
