// $Id: opto.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
/*
 * Author: Richard Reeve
 * Goal  : Interface to the optomotor on the Koala
 */

#ifndef __OPTO_H
#define __OPTO_H
#include "base.h"

// no movement opto signal
#define OPTO_STILL 795

// read optomotor signal (current only one camera - num=0)
INT get_optomotor(INT num);

#endif //ndef __OPTO_H
