#include "Game_hdr.h"
#include "Game_main.h"
#include "Com_parms.h"

enum
{
	CMD_CLIENTDEBUG
};

CVar *	g_varGravity=0;
CVar *	g_varMaxSpeed=0;
CVar *	g_varFriction=0;

//==========================================================================
//==========================================================================

/*
================================================
Register all the Game CVars
================================================
*/
void CGame::InitializeVars()
{
	I_Console * pConsole = I_Console::GetConsole();

	g_varGravity = pConsole->RegisterCVar("g_gravity", "800", CVAR_FLOAT, 0,this);
	g_varMaxSpeed = pConsole->RegisterCVar("g_maxspeed", "200", CVAR_FLOAT, 0,this);
	g_varFriction = pConsole->RegisterCVar("g_friction", "0.9", CVAR_FLOAT, 0,this);
	
	pConsole->RegisterCommand("gclientInfo",CMD_CLIENTDEBUG,this);
}

/*
================================================
Validate and handle changes in CVars.
================================================
*/
bool CGame::HandleCVar(const CVar * cvar, const CStringVal &strVal)
{
	if(cvar == g_varGravity)
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
	else if(cvar == g_varMaxSpeed)
	{
		float val = strVal.FloatVal();
		for(int i=0; i<numClients; i++)
		{
			clients[i]->maxSpeed = val;
			clients[i]->sendFlags |= SVU_MAXSPEED;
		}
		return true;
	}
	else if(cvar == g_varFriction)
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
	switch(cmdId)
	{
		case CMD_CLIENTDEBUG:
		{
			for(int i=0; i<numClients; i++)
			{
				if(clients[i])
					ComPrintf("%s : %s\n", clients[i]->name, clients[i]->origin.ToString());
			}
			break;
		}
	}
}