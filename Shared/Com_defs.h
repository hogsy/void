#ifndef VOID_COMMON_DEFINATIONS
#define VOID_COMMON_DEFINATIONS

#include <stdio.h>
#include <string.h>
#include <windows.h>

#ifdef _DEBUG

#include <crtdbg.h>
#define MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
#else
#define MALLOC(size) malloc(size)

#endif

//The apps common print function
void ComPrintf(char* text, ...);

#define COM_MAXPATH		256
#define COM_MAXFILENAME 128


#ifndef byte
typedef unsigned char byte;
#endif

typedef unsigned long ulong; 
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned word;

#endif