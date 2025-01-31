#include "Sys_hdr.h"

#include "Com_buffer.h"
#include "Com_vector.h"
#include "Com_util.h"
#include "Com_world.h"
#include "Com_camera.h"

#include "Net_client.h"
#include "Cl_base.h"
#include "Cl_hdr.h"
#include "Cl_cmds.h"
#include "Cl_game.h"

/*
================================================
Constructor
================================================
*/
CGameClient::CGameClient(I_ClientGame * pClGame) : 
				m_pClGame(pClGame)
{

	m_pCmdHandler = new CClientGameInput();

	m_ingame = false;
	m_numEnts = 0;
	m_numClients = 0;
	
	m_clNum = -1;
	m_pGameClient = 0;

	m_fFrameTime = 0.0f;
	
	m_pCamera = 0;
	m_pWorld = 0;

	I_Console * pConsole = I_Console::GetConsole();

	m_cvKbSpeed = pConsole->RegisterCVar("cl_kbspeed","5.0", CVAR_FLOAT, CVAR_ARCHIVE,this);
	m_cvClip = pConsole->RegisterCVar("cl_clip","1", CVAR_BOOL,0,this);
	m_cvLocalMove = pConsole->RegisterCVar("cl_localMove","0", CVAR_BOOL, CVAR_ARCHIVE,this);
	m_cvViewTilt = pConsole->RegisterCVar("cl_viewtilt","0.015", CVAR_FLOAT, CVAR_ARCHIVE,this);
	m_cvInRate = pConsole->RegisterCVar("cl_inRate","2500",	CVAR_INT,	CVAR_ARCHIVE,this);
	m_cvOutRate = pConsole->RegisterCVar("cl_outRate","2500",	CVAR_INT,	CVAR_ARCHIVE,this);
	m_cvName = pConsole->RegisterCVar("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE,this);
	m_cvCharacter = pConsole->RegisterCVar("cl_char", "Ratamahatta/Ratamahatta", CVAR_STRING, CVAR_ARCHIVE,this);
	m_cvDefaultChar = pConsole->RegisterCVar("cl_defaultChar","Amber/Amber",CVAR_STRING, CVAR_READONLY,this);

	//Register Commands
	for(int i=0; g_clGameCmds[i].szCmd; i++)
		pConsole->RegisterCommand(g_clGameCmds[i].szCmd,g_clGameCmds[i].id,this);	

	pConsole->RegisterCommand("say", CMD_TALK, this);
	pConsole->RegisterCommand("bind",CMD_BIND,this);
	pConsole->RegisterCommand("bindlist",CMD_BINDLIST,this);
	pConsole->RegisterCommand("cam",CMD_CAM,this);
	pConsole->RegisterCommand("unbind",CMD_UNBIND,this);
	pConsole->RegisterCommand("unbindall",CMD_UNBINDALL,this);
	pConsole->RegisterCommand("pos", CMD_DEBUG, this);

	m_hsTalk    = m_pClGame->RegisterSound("sounds/Interface/notify.wav", CACHE_LOCAL);
	m_hsMessage = m_pClGame->RegisterSound("sounds/Interface/click one.wav", CACHE_LOCAL);

	m_pCmdHandler->IntializeBinds();

}

/*
================================================
Destructor
================================================
*/
CGameClient::~CGameClient()
{
	m_pCmdHandler->WriteBinds("vbinds.cfg");
	UnloadWorld();
	delete m_pCmdHandler;
}

/*
================================================
Run a Client frame
================================================
*/
void CGameClient::RunFrame(float frameTime)
{
	m_fFrameTime = frameTime;

	//Check
	assert(m_ingame != false);

	//Run Input
	if(m_pCmdHandler->CursorChanged())
	{
		vector_t vecMouseAngles;
		m_pCmdHandler->UpdateCursorPos(vecMouseAngles.x, vecMouseAngles.y, vecMouseAngles.z);
		RotateRight(vecMouseAngles.x);
		RotateUp(vecMouseAngles.y);
	}
	m_pCmdHandler->RunCommands();

	
	//Get Angle vectors
	m_pGameClient->angles.AngleToVector(&m_vecForward, &m_vecRight, &m_vecUp);
	m_vecForward.Normalize();
	m_vecRight.Normalize();
	m_vecUp.Normalize();

	UpdateVelocity();
	UpdatePosition();
	UpdateViewAngles();
	UpdateViewBlends();

	//Save current view to send to the server
	m_cmd.angles = m_pGameClient->angles;
	m_cmd.time = frameTime * 1000.0f;
	if(m_cmd.time > 255.0f)
		m_cmd.time = 255.0f;

	m_cmd.Reset();
	m_vecDesiredAngles.Set(0,0,0);

	//Print misc crap
//	m_pClGame->HudPrintf(0,100,0,"ORIGIN: %s", m_pGameClient->origin.ToString());
//	m_pClGame->HudPrintf(0,120,0,"VELOCITY: %s",m_pGameClient->velocity.ToString());

	//Drawing
	//FIXME: PVS Check should be here
	int i;
	for(i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse && 
		  (m_entities[i].mdlIndex > -1) &&
		  (m_entities[i].skinNum > -1))
			m_pClGame->DrawModel(m_entities[i]);	
	}
	
	//Draw clients
	for(i=0; i< GAME_MAXCLIENTS; i++)
	{
		if(m_clients[i].inUse && 
		  (&m_clients[i] != m_pGameClient) && 
		  (m_clients[i].mdlIndex >=0))
		{
			m_pClGame->DrawModel(m_clients[i]);
		}
	}
}

/*
======================================
Check for changes to Command, then write
them to the buffer
======================================
*/
void CGameClient::WriteCmdUpdate(CBuffer &buf)
{
	buf.WriteByte(CL_MOVE);
	buf.WriteByte(((int)m_cmd.time));

	buf.WriteByte(m_cmd.moveFlags);

	//Temp
	buf.WriteFloat(m_pGameClient->origin.x);
	buf.WriteFloat(m_pGameClient->origin.y);
	buf.WriteFloat(m_pGameClient->origin.z);

	buf.WriteFloat(m_cmd.angles.x);
	buf.WriteFloat(m_cmd.angles.y);
	buf.WriteFloat(m_cmd.angles.z);

//	m_cmd.Reset();
}

/*
================================================
Write full update
================================================
*/
void CGameClient::WriteFullUpdate(CBuffer &buf)
{

}

/*
======================================
Update View blends depending on where we are
======================================
*/
void CGameClient::UpdateViewBlends()
{
	int contents = m_pWorld->PointContents(m_pGameClient->origin);
	if(contents & CONTENTS_SOLID)
		m_pCamera->blend.Set(0.4f, 0.4f, 0.4f);
	else if(contents & CONTENTS_WATER)
		m_pCamera->blend.Set(0, 1, 1);
	else if(contents & CONTENTS_LAVA)
		m_pCamera->blend.Set(1, 0, 0);
	else
		m_pCamera->blend.Set(1,1,1);
}

//==========================================================================
//==========================================================================

/*
================================================
Load any Client-Game related world data here
================================================
*/
bool CGameClient::LoadWorld(CWorld * pWorld)
{
	//setup
	m_pWorld = pWorld;
	EntMove::SetWorld(m_pWorld);

ComPrintf("CLGAME::Load World\n");
	return true;

}

/*
================================================
Unload Client-Game related resources here
================================================
*/
void CGameClient::UnloadWorld()
{
	if(!m_ingame)
		return;

	EntMove::SetWorld(0);

	delete m_pCamera;
	m_pCamera = 0;
	
	m_pWorld = 0;

	m_clNum = -1;
	m_pGameClient = 0;
	
	for(int i=0; i< GAME_MAXCLIENTS; i++)
		m_clients[i].Reset();

	for(i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse)
		{
			if(m_entities[i].sndIndex > -1)
				m_pClGame->RemoveSoundSource(&m_entities[i]);
		}
		m_entities[i].Reset();
	}

	m_numEnts = 0;
	m_numClients = 0;
	m_ingame = false;

	m_pClGame->StopMusicTrack();

	ComPrintf("CLGAME: Unload world\n");
}


/*
======================================
Print a message 
======================================
*/
void CGameClient::Print(const char * msg, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);

	I_Console::GetConsole()->ComPrint(textBuffer);
}

/*
================================================
Should be in client Game
================================================
*/
void CGameClient::Spawn(vector_t &origin, vector_t &angles)
{
}

//==========================================================================
//==========================================================================

I_InKeyListener	* CGameClient::GetKeyListener()
{	return (dynamic_cast<I_InKeyListener*>(m_pCmdHandler));
}

I_InCursorListener	* CGameClient::GetCursorListener()
{	return (dynamic_cast<I_InCursorListener*>(m_pCmdHandler));
}

CCamera *	CGameClient::GetCamera()
{	return m_pCamera;
}

int  CGameClient::GetOutgoingRate() const
{	return m_cvOutRate->ival;
}


//==========================================================================
//==========================================================================

/*
==========================================
Handle Registered Commands
==========================================
*/
void CGameClient::HandleCommand(int cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_MOVE_FORWARD:
		MoveForward();
		break;
	case CMD_MOVE_BACKWARD:
		MoveBackward();
		break;
	case CMD_MOVE_LEFT:
		MoveLeft();
		break;
	case CMD_MOVE_RIGHT:
		MoveRight();
		break;
	case CMD_ROTATE_LEFT:
		RotateLeft(m_cvKbSpeed->fval);
		break;
	case CMD_ROTATE_RIGHT:
		RotateRight(m_cvKbSpeed->fval);
		break;
	case CMD_ROTATE_UP:
		RotateUp(m_cvKbSpeed->fval);
		break;
	case CMD_ROTATE_DOWN:
		RotateDown(m_cvKbSpeed->fval);
		break;
	case CMD_JUMP:
		Jump();
		break;
	case CMD_CROUCH:
		Crouch();
		break;
	case CMD_BIND:
		m_pCmdHandler->BindFuncToKey(parms);
		break;
	case CMD_BINDLIST:
		m_pCmdHandler->BindList();
		break;
	case CMD_UNBIND:
		m_pCmdHandler->Unbind(parms);
		break;
	case CMD_UNBINDALL:
		m_pCmdHandler->Unbindall();
		break;
	case CMD_TALK:
		Talk(parms.String());
		break;
	case CMD_DEBUG:
		if(m_pGameClient)
			ComPrintf("Current Pos: %s\n", m_pGameClient->origin.ToString());
		break;
	}
}

/*
==========================================
Validate/Handle any CVAR changes
==========================================
*/
bool CGameClient::HandleCVar(const CVar * cvar, const CStringVal &strval)
{
	if(cvar == m_cvKbSpeed)
	{
/*		float val = strval.FloatVal();
		if(val < 0.6 || val > 10.0)
		{
			ComPrintf("Out of range. Should be between 0.6 and 10.0\n");
			return false;
		}
*/
		return true;
	}
	else if(cvar == m_cvInRate)
		return ValidateRate(strval);
	else if(cvar == m_cvOutRate)
	{
		int val = strval.IntVal();
		if(val < 1000 || val > 10000)
		{
			ComPrintf("Out of range. Should be between 1000 and 10000\n");
			return false;
		}
		m_pClGame->SetNetworkRate(val);
		return true;
	}
	else if(cvar == m_cvName)
		return ValidateName(strval);
	else if(cvar == m_cvCharacter)
		return ValidateCharacter(strval);
	else if(cvar == m_cvDefaultChar)
	{
		return false;
	}
	return false;
}
