#ifndef VOID_COMMON_DEFINATIONS
#define VOID_COMMON_DEFINATIONS

#define WIN32_LEAN_AND_MEAN

//disable horrible vc problem of identifiers 
//getting truncated to 255 char 
#pragma warning(disable : 4786)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

//STL
#include <list>
#include <string>
#include <vector>
#include <map>

const int COM_MAXPATH	  = 256;
const int COM_MAXFILENAME = 128;

typedef std::list<std::string>   StringList;
typedef std::vector<std::string> StringVector;

#ifndef byte
typedef unsigned char byte;
#endif

typedef unsigned long  ulong; 
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned word;

#ifndef VBSP
#include "Com_mem.h"
#endif

//Every apps common print function
void ComPrintf(const char* text, ...);

#endif