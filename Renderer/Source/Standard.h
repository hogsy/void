#ifndef RENDERER_STANDARD_H
#define RENDERER_STANDARD_H

// disable those damn warnings
#pragma warning(disable : 4018)     // signed/unsigned mismatch
#pragma warning(disable : 4305)		// truncation from const double to float

#define WIN32_LEAN_AND_MEAN

#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

#ifdef _DEBUG
#include <crtdbg.h>
#define MALLOC(size) _malloc_dbg(size, _NORMAL_BLOCK, __FILE__, __LINE__);
#else
#define MALLOC(size) malloc(size)
#endif


#include "gl.h"
#include "I_renderer.h"

#include "Con_main.h"

#include "3dMath.h"
#include "World.h"
#include "Util.h"

typedef unsigned char byte;

#define ConPrint g_prCons->Printf


#define MAX_VERTS_PER_POLY 64


extern RenderInfo_t* rInfo;

extern float * g_pCurTime;			//Current Time
extern float * g_pFrameTime;		//Frame Time
extern char  * g_szGamePath;	//Path to game dir

extern world_t	*world;


void FError(char *error, ...);		//Fatal Error, shutdown and exit
void Error(char *error, ...);		//Throw a message box

#endif
