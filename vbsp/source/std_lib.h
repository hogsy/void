

#ifndef STD_LIB_H
#define STD_LIB_H

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>


void Error(const char *err, ...);
void FError(const char *err, ...);
void ComPrintf(const char *err, ...);
void v_printf(const char *msg, ...);
void VFSError(const char *err);




#endif
