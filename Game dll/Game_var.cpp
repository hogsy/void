#include "Game_hdr.h"
#include "Game_main.h"
#include "Com_parms.h"


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
	I_Console::GetConsole()->RegisterCVar(&g_varGravity,this);
	I_Console::GetConsole()->RegisterCVar(&g_varMaxSpeed,this);
	I_Console::GetConsole()->RegisterCVar(&g_varFriction,this);
}

/*
================================================
Validate and handle changes in CVars.
================================================
*/
bool CGame::HandleCVar(const CVarBase * cvar, const CStringVal &strVal)
{
	if(cvar == reinterpret_cast<CVarBase *>(&g_varGravity))
	{
		float val = strVal.FloatVal();

ComPrintf("GAME: Grav changing to %f\n", val);

		for(int i=0; i<numClients; i++)
		{
			clients[i]->gravity = val;
			clients[i]->sendFlags |= SVU_GRAVITY;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase *>(&g_varMaxSpeed))
	{
		float val = strVal.FloatVal();
		for(int i=0; i<numClients; i++)
		{
			clients[i]->maxSpeed = val;
			clients[i]->sendFlags |= SVU_MAXSPEED;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase *>(&g_varFriction))
	{
		float val = strVal.FloatVal();
		for(int i=0; i<numClients; i++)
		{
			clients[i]->friction = val;
			clients[i]->sendFlags |= SVU_FRICTION;
		}
		return true;
	}
	
	return false;
}

/*
================================================
Handler game commands
================================================
*/
void CGame::HandleCommand(int cmdId, const CParms &parms)
{
}