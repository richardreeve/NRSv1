// $Id: tilt.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
/*
 * Author: Richard Reeve
 * Goal  : Interface to the tilt on the Koala
 */

#ifndef __TILT_H
#define __TILT_H
#include "base.h"

// read tilt signal (current only one tilt sensor - num=0)
INT get_tilt(INT num);

#endif //ndef __TILT_H
