// $Id: analogue.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __ANALOGUE_H
#define __ANALOGUE_H

#include "base.h"

// in 20mV units
#define BATTERY_VOLTAGE 0
// in 8mA units
#define BATTERY_CURRENT 1
// in 0.1 degree C units
#define BATTERY_TEMPERATURE 5
// in 0.1 degree C units
#define TEMPERATURE 2
// in 4mA units
#define LEFT_MOTOR_CURRENT 3
// in 4mA units
#define RIGHT_MOTOR_CURRENT 4

#define NUM_INTERNAL_ANALOGUE 6

INT get_internal_analogue( INT number );
INT get_analogue( INT number );

#endif //ndef __ANALOGUE_H
