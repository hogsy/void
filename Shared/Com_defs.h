#ifndef VOID_COMMON_DEFINATIONS
#define VOID_COMMON_DEFINATIONS

#undef  STRICT
#define STRICT

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN

//disable horrible vc problem of identifiers 
//getting truncated to 255 char 
#pragma warning(disable : 4786)

//Only include the following if not 
//being used in an MFC app
#ifndef __AFXWIN_H__

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

#endif

//STL
#include <list>
#include <string>
#include <vector>

const int COM_MAXPATH	  = 256;
const int COM_MAXFILENAME = 128;

typedef std::list<std::string>   StrList;
typedef std::list<std::string>::iterator StrListIt;

typedef std::vector<std::string> StrVec;
typedef std::vector<std::string>::iterator StrVecIt;

#ifndef byte
typedef unsigned char byte;
#else
#undef byte
typedef unsigned char byte;
#endif

typedef unsigned long  ulong; 
typedef unsigned int   uint;
typedef unsigned short ushort;
typedef unsigned word;

//Every apps common print function
void ComPrintf(const char* text, ...);

#endif