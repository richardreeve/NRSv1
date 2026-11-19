// Peripheral model
// $Id: peripheral.h,v 1.2 2003/12/06 00:07:34 darrens Exp $
// Richard Reeve

#ifndef __PERIPHERAL_H
#define __PERIPHERAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"

// Used to avoid freeing on changes
#define PERIPHERALS_MAX 50

  // Do we want true poisson spike trains or will average response do
  //#define DO_RANDOM

typedef enum
{
  ROBOT_KOALA = 0,
  ROBOT_OLD_KOALA,//1
  ROBOT_KHEPERA,//2
  SENSOR_EARS,//3
  SENSOR_OPTOMOTOR,//4
  SENSOR_BUMP,//5
  SENSOR_HAIR,//6
  SENSOR_ANTENNA,//7
  SENSOR_TILT,//8
  SENSOR_TICK,//9
  MOTOR_SPEED,//10
  MOTOR_POSITION,//11
  MOTOR_SOUND,//12
  INPUT_FILE,//13
  INPUT_RANDOM,//14
  INPUT_CONSTANT,//15
  INPUT_INTERVAL,//16
  OUTPUT_SONG,//17
  TRACKER_CAMERA,//18
  TRACKER_TICK,//19
  TRACKER_REEL,//20
  SENSOR_COMPASS,//21
  SENSOR_DIGITAL,//22
  SENSOR_AMBIENT,//23
  SENSOR_INTERNAL,//24
  SENSOR_ANALOGUE,//25
  SENSOR_IR,//26
  TRACKER_COMPASS,//27
  MOTOR_SPIKE,//28
  INPUT_LATCH//29
} PeripheralType;

#define PERIPHERAL_TYPES 30

#define PERIPHERAL_DATA 3
#define PERIPHERAL_REF 0
#define PERIPHERAL_BITS 1
#define PERIPHERAL_NUM_ELTS 2

extern BOOL Peripheral_input[PERIPHERAL_TYPES];
extern BOOL Peripheral_robot[PERIPHERAL_TYPES];
extern BOOL Peripheral_lowfreq[PERIPHERAL_TYPES];

#ifndef __RTL__

// initialise above variables
void peripheral_initialise();

#endif //ndef __RTL__

#ifdef __cplusplus
  };
#endif

#endif//ndef __PERIPHERAL_H
