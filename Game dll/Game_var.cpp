#include "Game_hdr.h"
#include "Game_main.h"


extern I_Console * g_pCons;


CVar	g_varGravity("g_gravity", "800", CVAR_FLOAT, 0);
CVar	g_varMaxSpeed("g_maxspeed", "200", CVAR_FLOAT, 0);
CVar	g_varFriction("g_friction", "0.9", CVAR_FLOAT, 0);

//==========================================================================
//==========================================================================

/*
================================================
Register all the Game CVars
================================================
*/
void CGame::InitializeVars()
{
	g_pCons->RegisterCVar(&g_varGravity,this);
	g_pCons->RegisterCVar(&g_varMaxSpeed,this);
}


/*
================================================
Validate and handle changes in CVars.
================================================
*/
bool CGame::HandleCVar(const CVarBase * cvar, const CParms &parms)
{	
	return false;
}

/*
================================================
Handler game commands
================================================
*/
void CGame::HandleCommand(HCMD cmdId, const CParms &parms)
{
}