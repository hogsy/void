#ifndef VOID_COMMON_DEFINATIONS
#define VOID_COMMON_DEFINATIONS

#define WIN32_LEAN_AND_MEAN

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

//STL
#include <list>
#include <string>
#include <vector>

const int COM_MAXPATH	  = 256;
const int COM_MAXFILENAME = 128;

typedef std::list<std::string> StringList;

#ifndef byte
typedef unsigned char byte;
#endif

typedef unsigned long ulong; 
typedef unsigned int  uint;
typedef unsigned short ushort;
typedef unsigned word;

//Every apps common print function
void ComPrintf(const char* text, ...);

#endif