// $Id: ir.h,v 1.1.1.1 2003/07/21 14:46:58 darrens Exp $
#ifndef __IR_H
#define __IR_H

#include "base.h"

INT get_ambient_average();
INT get_ambient_side( INT side );
INT get_ambient_value( INT number );
INT get_reflected_average();
INT get_reflected_side( INT side );
INT get_reflected_value( INT number );

#endif //ndef __IR_H
