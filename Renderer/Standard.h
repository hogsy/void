#ifndef RENDERER_STANDARD_H
#define RENDERER_STANDARD_H

//disable those damn warnings
#pragma warning(disable : 4305)		//truncation from const double to float

#include "Com_defs.h"

#include "I_hunkmem.h"
#include "I_renderer.h"

#include "Com_cvar.h"
#include "Com_vector.h"
#include "Com_world.h"

//Renderer Info
extern RenderInfo_t g_rInfo;	

//Console Interface for registering CVars
extern I_Console *  g_pConsole;

// MUST be after declaration of g_pConsole
#include "Rasterizer.h"

// Rasterizer Interface
extern I_Rasterizer  * g_pRast;


//The World
extern CWorld	*world;

float & GetCurTime();
float & GetFrameTime();
const char * GetCurPath();

void FError(char *error, ...);		//Fatal Error, shutdown and exit
void Error(char *error, ...);		//Throw a message box

#endif
