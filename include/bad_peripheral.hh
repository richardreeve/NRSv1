// Peripheral exceptions for robot
// $Id: bad_peripheral.hh,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __BAD_PERIPHERAL_HH
#define __BAD_PERIPHERAL_HH

#include <string>

#include "peripheral.h"
#include "Exception.hh"

namespace Runtime
{

  /// Exception class indicating peripheral problem
  class bad_peripheral : public runtime_exception
  {
  public:
    bad_peripheral() : runtime_exception() { }
    bad_peripheral( std::string aName ) : runtime_exception( aName ) { }
    bad_peripheral( PeripheralType aType ) : runtime_exception()
    {
      switch (aType)
	{
	case ROBOT_KOALA:
	  iName = "koala";
	  break;
	case ROBOT_OLD_KOALA:
	  iName = "old koala";
	  break;
	case ROBOT_KHEPERA:
	  iName = "khepera";
	  break;
	case SENSOR_EARS:
	  iName = "ear";
	  break;
	case SENSOR_OPTOMOTOR:
	  iName = "optomotor";
	  break;
	case SENSOR_BUMP:
	  iName = "bump";
	  break;
	case SENSOR_HAIR:
	  iName = "hair";
	  break;
	case SENSOR_ANTENNA:
	  iName = "antenna";
	  break;
	case SENSOR_TILT:
	  iName = "tilt";
	  break;
	case SENSOR_TICK:
	  iName = "tick";
	  break;
	case MOTOR_SPEED:
	  iName = "motorspeed";
	  break;
	case MOTOR_POSITION:
	  iName = "motorposition";
	  break;
	case MOTOR_SOUND:
	  iName = "sound";
	  break;
	case INPUT_FILE:
	  iName = "file";
	  break;
	case INPUT_RANDOM:
	  iName = "random";
	  break;
	case INPUT_CONSTANT:
	  iName = "constant";
	  break;
	case INPUT_INTERVAL:
	  iName = "interval";
	  break;
	case OUTPUT_SONG:
	  iName = "song";
	  break;
	case TRACKER_CAMERA:
	  iName = "camera";
	  break;
	case TRACKER_TICK:
	  iName = "trackertick";
	  break;
	case TRACKER_REEL:
	  iName = "reel";
	  break;
	case SENSOR_COMPASS:
	  iName = "compass";
	  break;
	case SENSOR_DIGITAL:
	  iName = "inputdigital";
	  break;
	case SENSOR_AMBIENT:
	  iName = "ambient";
	  break;
	case SENSOR_INTERNAL:
	  iName = "internal";
	  break;
	case SENSOR_ANALOGUE:
	  iName = "inputanalogue";
	  break;
	case SENSOR_IR:
	  iName = "ir";
	  break;
	case TRACKER_COMPASS:
	  iName = "trackercompass";
	  break;
	case MOTOR_SPIKE:
	  iName = "motorspike";
	  break;
	default:
	  iName = "< unknown >";
	  break;
	}
    }
  };
}

#endif//ndef __BAD_PERIPHERAL_HH
