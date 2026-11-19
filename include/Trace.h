// Logging utilities for C code
// $Id: Trace.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $

#ifndef __Trace_H
#define __Trace_H


#define _TRACE_(X) \
printf("TRACE: %s (%s:%i)\n", X, __FILE__, __LINE__)
  

#endif
