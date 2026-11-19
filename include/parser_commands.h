// commands for parser
// $Id: parser_commands.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
// Richard Reeve

#ifndef __PARSE_COMMANDS_H
#define __PARSE_COMMANDS_H

#include "base.h"

typedef enum {
  NAME_PARAM=0,
  NAME_NEUR_TYPE,
  NAME_SYN_TYPE,
  NAME_SYN_SIGN,
  NAME_PER_TYPE,
  NAME_CONST_INT,
  NAME_CONST_DOUBLE,
  NAME_INT,
  NAME_DOUBLE,
  NAME_SENSOR,
  NAME_MOTOR,
  NAME_NEURON,
  NAME_SYNAPSE,
  NAME_UNUSED
} NameType;

typedef struct symrec {
  char *name;
  NameType type;
  INT ref;
  double value;
  struct symrec *next;
} symbol;

extern symbol *Symbol_table;

#define MAX_INCLUDE_DEPTH 20

extern   INT Include_stack_ptr;
extern CHAR Current_file[MAX_INCLUDE_DEPTH][BUFF_MAX];
extern INT Current_line[MAX_INCLUDE_DEPTH];
void my_unput( int c );


void initialise_db();
void close_db();
NameType name_get_type(CHAR *name);            // find out name type from name
NameType ref_get_type(INT ref);                // find out name type from ref
INT name_get_ref(CHAR *name);                  // return reference for name
CHAR *ref_get_name(INT ref);                   // return reference for name
BOOL name_set_type(CHAR *name, NameType type); // set name type
BOOL name_set_type_val(CHAR *name, NameType type, double value); // set val too
BOOL name_is_object(CHAR *name);               // is a sensor, motor or neuron
BOOL name_is_defined(CHAR *name);              // is used
BOOL name_remove(CHAR *name);
BOOL ref_remove(INT ref);

// variables/params
double name_get_val(CHAR *name);               // return variable value
double ref_get_val(INT ref);                   // return value for variable ref
BOOL name_set_val(CHAR *name, double val);     // set variable value
BOOL ref_set_val(INT ref, double val);         // set value for variable ref

// output
CHAR *name_type_string(CHAR *name);
CHAR *ref_type_string(INT ref);
void output_all();

// Sleep a fixed number of milliseconds
void mysleep(INT ms, BOOL first);

// set working directory for include files from next file to be included
void set_cwd( CHAR *inFile );
// use cwd to complete filename
void complete_filename( CHAR *inFile, CHAR *outFile );
// open stdin for parser
void open_stdin();

// unmentioned builtins for parser
void yyerror( CHAR *s );
int yylex( void );
int yyparse( void );

#endif //ndef __PARSE_COMMANDS_H
