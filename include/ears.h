// $Id: ears.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
/*
 * Author: Richard Reeve
 * Goal  : Interface to the ears on the Koala or Khepera
 */

#ifndef __EARS_H
#define __EARS_H

#include "base.h"

// Send a command to the ears, with 8 bits of data
// (last 4 may not be used on some mode commands)
void send_command(UCHAR command, UCHAR data);

// Set mixing gain (AN) - K_LEFT/K_RIGHT, 1-3, 0 (=-1) to 255 (=+1)
void set_mix_gain(UCHAR side, UCHAR number, UCHAR data);

// Set preamp gain (PR) - K_LEFT/K_RIGHT, 1-2, 0 (=0) to 255 (=loud)
void set_preamp_gain(UCHAR side, UCHAR number, UCHAR data);

// Set low byte of delay on synth 1 or 2, delay in 0.625 microseconds 
// (0-159.375microsecs)
void set_delay(UCHAR number, UCHAR delay);
// affects next delay, in units of 160us, neg=TRUE for negative delay
void set_high_delay(BOOL neg, UCHAR ldelay);
// Set whole 12 bits of delay with sign on synth 1 or 2, delay in 0.625us
// -2.56ms to +2.56ms
void set_long_delay(UCHAR number, INT delay);

// Set LEDs:
// one is 0 (=on) or 1(=off)
// two is 0 (=off) 1-5, 7 (flashing modes), and 6 on
void set_LED(UCHAR one, UCHAR two);

// Store neuron setup on ears (1-3)
void save(UCHAR store_num);

// Reload neuron setup from ears (1-3)
void load(UCHAR store_num);

// Reset ears to starting setup
void reset_ears();

// Set two/four ear mode
void set_num_ears(UCHAR number);

// Set new ears parameters (16*8 bits), in order:
// num_ears, AN1R, AN2R, AN3R, AN1L, AN2L, AN3L, PR1R, PR2R, PR1L, PR2L,
// Synth1 sign (neg=TRUE, pos=FALSE), high nibble, low byte
// Synth2 sign (neg=TRUE, pos=FALSE), high nibble, low byte
void set_new_ears(UCHAR* data);

// Read from ear analogue output (K_RIGHT, K_LEFT, or -1 - not used (V_ref))
LONG read_ear_out(INT number);

// 
BOOL initialise_ears( INT numEars );

#endif//ndef __EARS_H
