// Sensor model for robot
// $Id: Sensor.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __SENSOR_HH
#define __SENSOR_HH

#include <string>

#include "base.h"
#include "sensor.h"


class Sensor: public Peripheral
{
public:
private:
  std::string iName;
  
};


typedef struct Sensor {
  SensorType type;
  BOOL spiking;
  INT output[MAX_DELAY2];
  INT spike_output;

  INT ref; // general reference
  INT number; // number of specific type
  INT num_conn;
  INT index; // reference in sensor arrays

  struct Sensor *prev;
  struct Sensor *next;
} Sensor;

#endif//ndef __SENSOR_HH
