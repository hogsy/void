#ifndef RENDERER_STANDARD_H
#define RENDERER_STANDARD_H

//disable those damn warnings
#pragma warning(disable : 4305)		//truncation from const double to float

#define WIN32_LEAN_AND_MEAN

#include <objbase.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <windows.h>

#include "Com_defs.h"
#include "I_renderer.h"
#include "I_file.h"

#include "gl.h"
#include "3dMath.h"
#include "World.h"
#include "Util.h"

#define ConPrint ComPrintf

//Renderer Info
extern RenderInfo_t g_rInfo;	

//Console Interface for registering CVars
extern I_Console *  g_pConsole;

//The World
extern world_t	*world;

//Timing variables
extern float * g_pCurTime;		
extern float * g_pFrameTime;	


void FError(char *error, ...);		//Fatal Error, shutdown and exit
void Error(char *error, ...);		//Throw a message box

#endif
