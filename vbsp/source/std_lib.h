

#ifndef STD_LIB_H
#define STD_LIB_H

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void Error(char *err, ...);
void FError(char *err, ...);
void ComPrintf(char *err, ...);
void v_printf(char *msg, ...);




#endif
