// base essential includes for program
// $Id: base.h,v 1.2 2003/07/29 16:52:43 darrens Exp $
// Richard Reeve

#ifndef __BASE_H
#define __BASE_H

#ifdef __cplusplus
extern "C" {
#endif

// RTL does not exist on Koala/Khepera!
#ifdef __RTL__
#undef KOALA
#undef KHEPERA
#endif

#ifdef KOALA
#include <koabios.h>
#include <string.h>
#include <stdlib.h>
typedef int16 INT;
typedef int32 LONG;
typedef uint8 UCHAR;
typedef int8 CHAR;
#define TARGET_STRING "koala"
// only one thread, so no need to do this
//  #undef RESERVE_SER
//  #undef RELEASE_SER
//  #define RESERVE_SER
//  #define RELEASE_SER

#else//ndef KOALA
#ifdef KHEPERA
#include <khebios.h>
#include <string.h>
#include <stdlib.h>
typedef int16 INT;
typedef int32 LONG;
typedef uint8 UCHAR;
typedef int8 CHAR;
#define TARGET_STRING "khepera"
// only one thread, so no need to do this
//  #undef RESERVE_SER
//  #undef RELEASE_SER
//  #define RESERVE_SER
//  #define RELEASE_SER

#else//ndef KHEPERA
#define __NO_ROBOT
typedef short INT; // 16 bit
typedef long LONG; // = int = 32 bit
typedef unsigned char UCHAR;
typedef char CHAR;
#define TARGET_STRING "linux"
#endif
#endif

#define SPIKE 1
#define MS_PER_SEC 1000
#define US_PER_MS 1000
#define DEFAULT_INIT_FILE PROJDIR "initialise/initialise.params"

// Types of Synapse
typedef enum 
{
  SYNAPSES_EXCITATORY=0,
  SYNAPSES_INHIBITORY,
  SYNAPSES_SHUNTING,
  SYNAPSES_SIGNLESS
} SynapseSign;
  
typedef INT BOOL;
// Some people use 1 as TRUE... but !-1=0... I hope this doesn't break anything
#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE -1
#define FALSE 0


// Sides
#define K_LEFT 0
#define K_RIGHT 1

typedef enum
{
  outTypeText = 0,
  outTypePerl,
  outTypeMatlab,
  outTypePython
} OutputType;

// Main Router name
#define ROUTER "router"

// Motor names
#define LEFT_MOTOR "leftMotor"
#define RIGHT_MOTOR "rightMotor"

// Variables ending in B have a binary point and are offset by
// <<BINARY_POINT from imaginary (but referred to) variables without
// the B. This is necessary because of the lack of floating point handling

#define BINARY_POINT 8
#define BINARY_OFFSET 256.0
#define BINARY_MAX_LONG 16777216

// (LONG)(ln(2)<<BINARY_POINT)
#define LN2B 177

// Maximum delay in synapse is length of int
#define MAX_DELAY 16
#define MAX_DELAY2 32

// maximum length of name
#define NAME_LEN 100

// Max buffer size?
#define BUFF_MAX 2000

// default time for a simulation run
#define DEFAULT_TIME 1000

// What are we connecting to?
#define CON_KOALA 117
#define CON_KHEP 35
#define CON_LINUX 66

typedef struct
{
  BOOL   is_int;
  INT    num_var;
  INT    *INT_ptr;
  CHAR  *CHAR_ptr;
} VarArg;

#define OPT_FREQ_ARG 0

#define freeVarArg(x); \
{ \
if (x.is_int) \
free(x.INT_ptr); \
else \
free(x.CHAR_ptr); \
}

// End of output from robot when switched on
#define ROBOT_BOOT "S format Motorola loader mode\015\n"
#define ROBOT_BOOT_SCP "Serial Communication Protocol\015\n"
#define READY_FOR_CODE "Robot has booted, ready for download\n"
// Output from robot when finished
#define ROBOT_MESSAGE_R "program running on %s...\n"
#define ROBOT_MESSAGE_L "program running on %s...\n\015"
#define DOWNLOAD_COMPLETE "Robot running\n"

// Flash commands koala
#define SCP_MSG_KOA "Writing to koala flash - set to 9 to use in future\n"
#define SCP_PRE_KOA "flash E\nsfill\n"
#define SCP_POST_KOA "flash W\nrun user-flash\n"
// Flash commands khepera
#define SCP_MSG_KHE "Running sloader - no flash on khepera\n"
#define SCP_PRE_KHE "sfill\n"
#define SCP_POST_KHE "run sloader\n"

// Debugging
//#ifdef DEBUG
//#define OUTPUT(x,y) printf(x,y)
//#else
//#define OUTPUT(x,y)
//#endif //def DEBUG

#ifdef __cplusplus
  };
#endif

#endif//ndef __BASE_H





