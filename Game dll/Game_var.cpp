#include "Game_hdr.h"
#include "Game_main.h"
#include "Com_parms.h"


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
	g_pCons->RegisterCVar(&g_varFriction,this);
}


/*
================================================
Validate and handle changes in CVars.
================================================
*/
bool CGame::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == reinterpret_cast<CVarBase *>(&g_varGravity))
	{
		if(parms.NumTokens() == 1)
		{
			ComPrintf("Game: Gravity = %.2f\n", g_varGravity.fval);
			return false;
		}
		
		float val = parms.FloatTok(1);
		for(int i=0; i<numClients; i++)
		{
			clients[i]->gravity = val;
			clients[i]->sendFlags |= SVU_GRAVITY;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase *>(&g_varMaxSpeed))
	{
		if(parms.NumTokens() == 1)
		{
			ComPrintf("Game: MaxSpeed = %.2f\n", g_varMaxSpeed.fval);
			return false;
		}

		float val = parms.FloatTok(1);
		for(int i=0; i<numClients; i++)
		{
			clients[i]->maxSpeed = val;
			clients[i]->sendFlags |= SVU_MAXSPEED;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase *>(&g_varFriction))
	{
		if(parms.NumTokens() == 1)
		{
			ComPrintf("Game: Friction = %.2f\n", g_varFriction.fval);
			return false;
		}

		float val = parms.FloatTok(1);
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
void CGame::HandleCommand(HCMD cmdId, const CParms &parms)
{
}