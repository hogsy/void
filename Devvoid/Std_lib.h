#ifndef STD_LIB_H
#define STD_LIB_H

#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)     // truncate from double to float

#include "Com_defs.h"
#include "I_file.h"

//Add whatever flags here
extern bool	g_bFastBSP;
extern unsigned char g_ambient[3];
extern int	g_dSamples;

I_FileReader * CreateFileReader(EFileMode mode);

void FError(const char *err, ...);
void Error(const char *err, ...);
void ComPrintf(const char *err, ...);
const char * GetVoidPath();

//Progress control
void Progress_SetRange(int min, int max);
void Progress_SetStep(int step);
int  Progress_Step();

#endif
