#ifndef RENDERER_STANDARD_H
#define RENDERER_STANDARD_H

//disable those damn warnings
#pragma warning(disable : 4305)		//truncation from const double to float

#include "Com_defs.h"
#include "Com_mem.h"
#include "Com_vector.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "I_hunkmem.h"
#include "I_console.h"
#include "I_file.h"
#include "I_renderer.h"

#include "Rast_main.h"

/*
================================================
Global vars
================================================
*/
//Renderer Info
extern RenderInfo_t g_rInfo;

// Rasterizer Interface
extern CRasterizer  * g_pRast;
//The World
extern CWorld	*world;

/*
================================================
Global funcs
================================================
*/
float GetCurTime();
float GetFrameTime();

void  FError(char *error, ...);		//Fatal Error, shutdown and exit
void  Error(char *error, ...);		//Throw a message box

const char * GetCurPath();

I_FileReader * CreateFileReader(EFileMode mode);

#endif
