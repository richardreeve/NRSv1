// $Id: digital.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
#ifndef __DIGITAL_H
#define __DIGITAL_H

#include "base.h"

/* Compass sensor
  --------------------------
  |    ___                 B | Brown wire
  |   /   \                O | Orange wire
  |   \___/                Y | Yellow wire
  |                        P | Purple wire
  |                          |
  |                        R | Red (+5v) wire
  |                        B | Blue (ground) wire
  --------------------------
  
  BOYP high except:
  
  N down  (v) Brown  low
  N left  (<) Orange low
  N up    (^) Yellow low
  N right (>) Purple low
*/

#define COMP_B 1
#define COMP_O 2
#define COMP_Y 4
#define COMP_P 8

// 0 means Yellow is north (up)
// 2 means Purple  --"--   (left)
// 4 means Brown   --"--   (down)
// 6 means Orange  --"--   (right)
#define COMPASS_OFFSET 0

INT get_all_digital();
INT get_digital( INT number );

#endif //ndef __DIGITAL_H
