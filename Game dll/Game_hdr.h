#ifndef VOID_GAMEDLL_HEADER
#define VOID_GAMEDLL_HEADER

/*
================================================
All game source files should include this.
================================================
*/

#include "Com_defs.h"
#include "Com_mem.h"
#include "Com_vector.h"
#include "Com_cvar.h"
#include "Net_defs.h"
#include "I_game.h"


//Game imports
extern I_GameHandler * g_pImports;


//Global Game CVars

extern CVar	g_varGravity;
extern CVar g_varMaxSpeed;
extern CVar g_varFriction;


#endif