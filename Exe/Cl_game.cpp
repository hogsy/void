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
				m_pClGame(pClGame),
				m_cvKbSpeed("cl_kbspeed","5.0", CVAR_FLOAT, CVAR_ARCHIVE),
				m_cvClip("cl_clip","1",     CVAR_BOOL,0),
				m_cvRate("cl_rate","2500",	CVAR_INT,	CVAR_ARCHIVE),
				m_cvName("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE),
				m_cvModel("cl_model", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE),
				m_cvSkin("cl_skin", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientGameInput();

	m_ingame = false;
	m_numEnts = 0;
	
	m_pGameClient = 0;
	m_pCamera = 0;
	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvKbSpeed,this);
	System::GetConsole()->RegisterCVar(&m_cvClip);
	System::GetConsole()->RegisterCVar(&m_cvRate,this);
	System::GetConsole()->RegisterCVar(&m_cvName,this);
	System::GetConsole()->RegisterCVar(&m_cvModel,this);
	System::GetConsole()->RegisterCVar(&m_cvSkin,this);

	//Register Commands
	for(int i=0; g_clGameCmds[i].szCmd; i++)
		System::GetConsole()->RegisterCommand(g_clGameCmds[i].szCmd,g_clGameCmds[i].id,this);	

	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);
	System::GetConsole()->RegisterCommand("bind",CMD_BIND,this);
	System::GetConsole()->RegisterCommand("bindlist",CMD_BINDLIST,this);
	System::GetConsole()->RegisterCommand("cam",CMD_CAM,this);
	System::GetConsole()->RegisterCommand("unbind",CMD_UNBIND,this);
	System::GetConsole()->RegisterCommand("unbindall",CMD_UNBINDALL,this);

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

static float blahFrameTime = 0.0f;

/*
================================================
Run a Client frame
================================================
*/
void CGameClient::RunFrame(float frameTime)
{
	//Reset move and angles stuff from the old frame
	m_vecDesiredMove.Set(0,0,0);
	m_vecDesiredAngles.Set(0,0,0);
	
	m_cmd.forwardmove = m_cmd.rightmove = m_cmd.upmove = 0;
	m_cmd.angles[0] = m_cmd.angles[1] = m_cmd.angles[2] = 0;

	//Input
	if(m_pCmdHandler->CursorChanged())
	{
		m_pCmdHandler->UpdateCursorPos(m_vecMouseAngles.x, m_vecMouseAngles.y, m_vecMouseAngles.z);
		RotateRight(m_vecMouseAngles.x);
		RotateUp(m_vecMouseAngles.y);
	}
	m_pCmdHandler->RunCommands();

	//Movement
	//Get Normalized angle vectors
	m_pGameClient->angles.AngleToVector(&m_vecForward, &m_vecRight, &m_vecUp);
	m_vecForward.Normalize();
	m_vecRight.Normalize();
	m_vecUp.Normalize();

	//Get desired move
	m_vecDesiredMove.VectorMA(m_vecDesiredMove,m_cmd.forwardmove, m_vecForward);
	m_vecDesiredMove.VectorMA(m_vecDesiredMove,m_cmd.rightmove, m_vecRight);
	m_vecDesiredMove.VectorMA(m_vecDesiredMove,m_cmd.upmove, m_vecUp);

	//Perform the actual move and update angles
	UpdatePosition(m_vecDesiredMove, frameTime);
	UpdateAngles(m_vecDesiredAngles,frameTime);

	//Save current view to send to the server
	m_cmd.angles[0] = m_pGameClient->angles.x;
	m_cmd.angles[1] = m_pGameClient->angles.y;
	m_cmd.angles[2] = m_pGameClient->angles.z;

	//Print misc crap
	m_pClGame->HudPrintf(0,100,0,"%.2f, %.2f, %.2f", m_pGameClient->origin.x,m_pGameClient->origin.y,m_pGameClient->origin.z);
	m_pClGame->HudPrintf(0,120,0,"FORWARD: %.2f,%.2f,%.2f", m_vecForward.x,m_vecForward.y,m_vecForward.z);
	m_pClGame->HudPrintf(0,140,0,"UP     : %.2f,%.2f,%.2f", m_vecUp.x,m_vecUp.y,m_vecUp.z);	
//	m_pClGame->HudPrintf(0,160,0,"ANGLES  : %.2f,%.2f,%.2f", m_pGameClient->angles.x,
//				m_pGameClient->angles.y,m_pGameClient->angles.z);	

	//Drawing
	//fix me. draw ents only in the pvs
	for(int i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse && (m_entities[i].mdlIndex >= 0))
			m_pClGame->DrawModel(m_entities[i]);	
	}
	
	//Draw clients
	for(i=0; i< GAME_MAXCLIENTS; i++)
	{
		if(m_clients[i].inUse && m_clients[i].mdlIndex >=0)
			m_pClGame->DrawModel(m_clients[i]);
	}

	UpdateViewBlends();

	blahFrameTime = frameTime;
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
	buf.WriteFloat(blahFrameTime);

	buf.WriteShort(m_cmd.forwardmove);
	buf.WriteShort(m_cmd.rightmove);
	buf.WriteShort(m_cmd.upmove);

	buf.WriteFloat(m_cmd.angles[0]);
	buf.WriteFloat(m_cmd.angles[1]);
	buf.WriteFloat(m_cmd.angles[2]);


	m_pClGame->HudPrintf(0,180,0,"CMD: %d,%d,%d, ANG: %.2f,%.2f,%.2f, %.2fms", 
		m_cmd.forwardmove, m_cmd.rightmove, m_cmd.upmove, m_cmd.angles[0],m_cmd.angles[1],m_cmd.angles[2],
		blahFrameTime);



//	ComPrintf("CL: %d %d %d\n", m_clients[clNum]->clCmd.forwardmove,
//					m_clients[clNum]->clCmd.rightmove, m_clients[clNum]->clCmd.upmove);
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
		VectorSet(&m_vecBlend, 0.4f, 0.4f, 0.4f);
/*	else if(contents & CONTENTS_WATER)
		VectorSet(&m_vecBlend, 0, 1, 1);
	else if(contents & CONTENTS_LAVA)
		VectorSet(&m_vecBlend, 1, 0, 0);
*/
	else
		VectorSet(&m_vecBlend, 1, 1, 1);
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

	ComPrintf("CGameClient::Load World: OK\n");
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

	m_ingame = false;

	EntMove::SetWorld(0);

	delete m_pCamera;
	m_pCamera = 0;

	m_pGameClient = 0;
	m_pWorld = 0;
	
	for(int i=0; i< GAME_MAXCLIENTS; i++)
		if(m_clients[i].inUse) 
			m_clients[i].Reset();

	for(i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse)
		{
			if(m_entities[i].sndIndex > -1)
				m_pClGame->RemoveSoundSource(&m_entities[i]);
			m_entities[i].Reset();
		}
	}
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

	System::GetConsole()->ComPrint(textBuffer);
}


/*
================================================

================================================
*/
void CGameClient::Spawn(vector_t	*origin, vector_t *angles)
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


//==========================================================================
//==========================================================================

/*
==========================================
Handle Registered Commands
==========================================
*/
void CGameClient::HandleCommand(HCMD cmdId, const CParms &parms)
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
		RotateLeft(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_RIGHT:
		RotateRight(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_UP:
		RotateUp(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_DOWN:
		RotateDown(m_cvKbSpeed.fval);
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
	}
}

/*
==========================================
Validate/Handle any CVAR changes
==========================================
*/
bool CGameClient::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == reinterpret_cast<CVarBase *>(&m_cvKbSpeed))
	{
		float val = parms.FloatTok(1);
		if(val < 0.6 || val >= 10.0)
		{
			ComPrintf("Out of range. Should be between 0.6 and 10.0\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvRate))
		return ValidateRate(parms);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvName))
		return ValidateName(parms);
	return false;
}













