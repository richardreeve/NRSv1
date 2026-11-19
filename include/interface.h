// interface header file
// defines communication interface for simulator
// $Id: interface.h,v 1.2 2003/09/18 18:36:03 darrens Exp $
// Richard Reeve

#ifndef __INTERFACE_H
#define __INTERFACE_H

#ifdef __cplusplus
extern "C" {
#endif

// Socket stuff
#define PORT_USER 1170
#define PORT_TRACK 1171
#define USER_ADDRESS "127.0.0.1"//"ganglion"//"127.0.0.1"
#define TRACK_ADDRESS "127.0.0.1"//"ganglion"//"127.0.0.1"

extern CHAR Port_hostname[];
extern CHAR Track_hostname[];

// Are we using an external tracker?
extern BOOL Has_tracker;
// Are we using the internal tracker?
extern BOOL Ticker;
// Gap between tick readings?
extern LONG Ticker_gap;
  
// Download file
#define DOWNLOAD_NAME PROJDIR "../bin/%s/%s.%s"
extern char DownloadName[NAME_LEN];
extern INT Connection;
extern INT ConnectionSpeed;

// Interface level
typedef enum
{
  LEVEL_USER=0,             // User -> Simulator comms 
  LEVEL_SENSOR              // Simulator -> sensor comms
} InterfaceLevel;

// Interface type
typedef enum
{
  INTERFACE_DIRECT=0,
  INTERFACE_SERIAL,
  INTERFACE_PORT,
  INTERFACE_FIFO
} InterfaceType;

#define OUTPUT_SENSOR       1 // Output sensor readings
#define OUTPUT_NEURON_SPIKE 2 // Output neuron spike train
#define OUTPUT_NEURON_MEMB  4 // Output neuron membrane voltage
#define OUTPUT_MOTOR        8 // Output motor readings
#define OUTPUT_SYNAPSE     16 // Output synapse readings
#define OUTPUT_TRACK       32 // Output track or tick count readings

// Interface commands, with following arguments
typedef enum
{
  SIM_RESET=0,              // NONE (Start again, delete everything)
  // base parameters
  SIM_INIT_PARAMS,          // init string (Set parameters to initial values)
  SIM_SAVE_PARAMS,          // NONE
  SIM_RESTORE_PARAMS,       // NONE
  SIM_SET_PARAM,            // param name, value, units
  SIM_GET_UNITS,            // param name
  SIM_GET_DESCRIPTION,      // param name
  SIM_GET_VALUE,            // param name
  // neurons
  SIM_ADD_ROUTER,           // router name (also to change context)
  SIM_ADD_NEURON,           // type and name (also to change params)
  //                           BOOL (has target), target name
  SIM_ADD_SYNAPSE,          // type, name, sign, from, to, delay
  //                           (also to change params)
  SIM_ADD_PERIPHERAL,       // PeripheralType, in a robot, name, args
  SIM_ADD_INPUT,            // PeripheralType, in a robot, name, args
  SIM_ADD_OUTPUT,           // PeripheralType, in a robot, name, args
  SIM_REMOVE_NODE,          // remove a node by name
  SIM_DESCRIBE_NODE,        // describe a node by name (returns string)

  SIM_POP_CONTEXT,          // change context to previous router
  // simulator
  SIM_END_TIME,             // time
  SIM_START_SIM,            // NONE
  SIM_WAIT_COMPLETION,      // NONE
  SIM_IS_RUNNING,           // NONE (returns BOOL)
  SIM_STOP,                 // NONE
  SIM_QUIT,                 // NONE
  // other info
  SIM_SAVE_FILE,            // CHAR* filename to save data to
  SIM_SAVE_LEVEL,
  SIM_SAVE_STOP,
  SIM_OUTPUT_LEVEL,         // INT (amount of info to be printed)
  SIM_OUTPUT_NAMES,         // returns string
  SIM_OUTPUT,               // returns array
  SIM_PING,
  SIM_XML
} SimInterface;

// HIGH SIDE CALLS

// start user side of interface
BOOL call_start(InterfaceLevel level, InterfaceType type );
// send data
BOOL call_send_int(InterfaceLevel level, INT number);
BOOL call_send_long(InterfaceLevel level, LONG long_number);
BOOL call_send_double(InterfaceLevel level, double double_number);
BOOL call_send_data(InterfaceLevel level, INT length, void *data, 
		    BOOL are_ints);
BOOL call_send_string( InterfaceLevel level, char *aString );
BOOL call_send_VarArg(InterfaceLevel level, VarArg arg);
// close user side of interface
BOOL call_close(InterfaceLevel level);

// LOW SIDE CALLS

// start and run simulator side of interface
BOOL handle_start(InterfaceLevel level, InterfaceType type );
// receive data
INT handle_receive_int(InterfaceLevel level);
LONG handle_receive_long(InterfaceLevel level);
double handle_receive_double(InterfaceLevel level);
INT handle_receive_data(InterfaceLevel level, void *data, BOOL are_ints);
INT handle_receive_string( InterfaceLevel level, char *aString );
VarArg handle_receive_VarArg(InterfaceLevel level);
// start and run simulator side of interface
BOOL handle_close(InterfaceLevel level);

BOOL tracker_start();
BOOL tracker_begin();
BOOL tracker_end();
BOOL tracker_close();

#ifdef __cplusplus
  };
#endif

#endif //ndef __INTERFACE_H
