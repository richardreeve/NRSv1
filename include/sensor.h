// Sensor model for robot
// $Id: sensor.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __LOW_SENSOR_H
#define __LOW_SENSOR_H

#ifdef __cplusplus
extern "C" {
#endif

#include "base.h"
#include "peripheral.h"

typedef struct Sensor {
  PeripheralType type;
  INT ref; // general reference
  INT val; // current value

  struct Sensor *prev;
  struct Sensor *next;
} Sensor;

extern Sensor *Sensors;
extern INT Sensor_offset;
extern LONG Motor_pos[2];

// number of parameters for sensors
#define PARAMS_EARS      2
#define PARAMS_OPTOMOTOR 2
#define PARAMS_BUMP      1
#define PARAMS_HAIR      1
#define PARAMS_ANTENNA   1
#define PARAMS_RANDOM    2
#define PARAMS_CONSTANT  1
#define PARAMS_PLAYBACK  1
#define PARAMS_INTERVAL  4
#define PARAMS_TICK      1
#define PARAMS_SPIKING_EARS 0

// default parameter values for sensors
#define DEFAULT_EARS_OFF        0
#define DEFAULT_EARS_SCALE      1
#define DEFAULT_OPTOMOTOR_OFF   0
#define DEFAULT_OPTOMOTOR_SCALE 1
#define DEFAULT_BUMP            1
#define DEFAULT_HAIR            1
#define DEFAULT_ANTENNA         1
#define DEFAULT_RANDOM_MIN      0
#define DEFAULT_RANDOM_NUM      101
#define DEFAULT_CONSTANT        100
#define DEFAULT_INTERVAL_H      90
#define DEFAULT_INTERVAL_L      10
#define DEFAULT_INTERVAL_TH     20
#define DEFAULT_INTERVAL_TL     20

// Spikes for SENSOR_SPIKING_EARS sensors
#define SPIKE_LEFT_32 1
#define SPIKE_LEFT_64 2
#define SPIKE_LEFT_128 4
#define SPIKE_LEFT_256 8
#define SPIKE_RIGHT_32 256
#define SPIKE_RIGHT_64 512
#define SPIKE_RIGHT_128 1024
#define SPIKE_RIGHT_256 2048

// Nmber of sensors seen by this system
// (for linux, all, but for robot, just real sensors, not playback, etc.)
extern INT Num_sensors;
extern INT Num_sensor_bytes;

// Do we have this kind of sensor?
extern BOOL Has_sensor[PERIPHERAL_TYPES];
// How many of them?
extern INT Num_sensor[PERIPHERAL_TYPES];

// Platform dependent functions
// ----------------------------
// Parameterise sensors as appropriate
BOOL sensors_param( INT ref, VarArg param, BOOL *bits, INT *num );

// get sensor readings, return number of bytes
INT sensors_acquire( INT num_bytes, CHAR **sense_vals );

// Platform independent functions
// ------------------------------
// find sensor from ref
Sensor *find_sensor( INT ref );

// sets up sensor array for use by neurons
BOOL sensors_change( PeripheralType type, INT change, INT* ref );

// frees spare memory
void sensors_free();

  // Sets up log function table for later access
  void setup_log2B();

  // Sets Has_sensor
  void sensor_initialise();

#ifdef __cplusplus
  };
#endif

#endif//ndef __LOW_SENSOR_H
