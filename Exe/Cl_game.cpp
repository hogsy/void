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
				m_cvInRate("cl_inRate","2500",	CVAR_INT,	CVAR_ARCHIVE),
				m_cvOutRate("cl_outRate","2500",	CVAR_INT,	CVAR_ARCHIVE),
				m_cvName("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE),
				m_cvCharacter("cl_char", "Amber/Amber", CVAR_STRING, CVAR_ARCHIVE),
				m_cvViewTilt("cl_viewtilt","0.015", CVAR_FLOAT, CVAR_ARCHIVE),
				m_cvDefaultChar("cl_defaultChar","Amber/Amber",CVAR_STRING, CVAR_ARCHIVE),
				m_cvLocalMove("cl_localMove","0", CVAR_BOOL, CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientGameInput();

	m_ingame = false;
	m_numEnts = 0;
	m_numClients = 0;
	
	m_clNum = -1;
	m_pGameClient = 0;

	m_fLastUpdate = 0.0f;
	m_fFrameTime = 0.0f;
	
	m_pCamera = 0;
	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvKbSpeed,this);
	System::GetConsole()->RegisterCVar(&m_cvClip);
	System::GetConsole()->RegisterCVar(&m_cvLocalMove);
	System::GetConsole()->RegisterCVar(&m_cvViewTilt);
	System::GetConsole()->RegisterCVar(&m_cvInRate,this);
	System::GetConsole()->RegisterCVar(&m_cvOutRate,this);
	System::GetConsole()->RegisterCVar(&m_cvName,this);
	System::GetConsole()->RegisterCVar(&m_cvCharacter, this);
	System::GetConsole()->RegisterCVar(&m_cvDefaultChar, this);


	//Register Commands
	for(int i=0; g_clGameCmds[i].szCmd; i++)
		System::GetConsole()->RegisterCommand(g_clGameCmds[i].szCmd,g_clGameCmds[i].id,this);	

	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);
	System::GetConsole()->RegisterCommand("bind",CMD_BIND,this);
	System::GetConsole()->RegisterCommand("bindlist",CMD_BINDLIST,this);
	System::GetConsole()->RegisterCommand("cam",CMD_CAM,this);
	System::GetConsole()->RegisterCommand("unbind",CMD_UNBIND,this);
	System::GetConsole()->RegisterCommand("unbindall",CMD_UNBINDALL,this);
	System::GetConsole()->RegisterCommand("pos", CMD_DEBUG, this);

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

	//Reset move and angles stuff from the old frame
	m_vecDesiredAngles.Set(0,0,0);
	m_cmd.Reset();

	//Run Input
	if(m_pCmdHandler->CursorChanged())
	{
		vector_t vecMouseAngles;

		m_pCmdHandler->UpdateCursorPos(vecMouseAngles.x, vecMouseAngles.y, vecMouseAngles.z);
		RotateRight(vecMouseAngles.x);
		RotateUp(vecMouseAngles.y);
	}
	m_pCmdHandler->RunCommands();

	//Process Movement
	
	//Get Angle vectors
	m_pGameClient->angles.AngleToVector(&m_vecForward, &m_vecRight, &m_vecUp);
	m_vecForward.Normalize();
	m_vecRight.Normalize();
	m_vecUp.Normalize();

	// find forward and right vectors that lie in the xy plane
	vector_t f, r, vecDesiredMove;
	f = m_vecForward;
	f.z = 0;
	if (f.Normalize() < 0.3)
	{
		if (m_vecForward.z < 0)
			f = m_vecUp;
		else
			m_vecUp.Scale(f, -1);

		f.z = 0;
		f.Normalize();
	}

	r = m_vecRight;
	r.z = 0;
	if (r.Normalize() < 0.3)
	{
		if (m_vecRight.z > 0)
			r = m_vecUp;
		else
			m_vecUp.Scale(r, -1);

		r.z = 0;
		r.Normalize();
	}

	//Get desired move
	if(m_cmd.moveFlags & ClCmd::MOVEFORWARD)
		vecDesiredMove.VectorMA(vecDesiredMove, m_pGameClient->maxSpeed, f);
	if(m_cmd.moveFlags & ClCmd::MOVEBACK)
		vecDesiredMove.VectorMA(vecDesiredMove,-m_pGameClient->maxSpeed, f);
	if(m_cmd.moveFlags & ClCmd::MOVERIGHT)
		vecDesiredMove.VectorMA(vecDesiredMove,m_pGameClient->maxSpeed, r);
	if(m_cmd.moveFlags & ClCmd::MOVELEFT)
		vecDesiredMove.VectorMA(vecDesiredMove,-m_pGameClient->maxSpeed, r);
	if(m_cmd.moveFlags & ClCmd::JUMP)
		vecDesiredMove.z += 300;

	// always add gravity
	vecDesiredMove.z -= (m_pGameClient->gravity * frameTime);

	// gradually slow down (friction)
	m_pGameClient->velocity.x *= (m_pGameClient->friction * frameTime);
	m_pGameClient->velocity.y *= (m_pGameClient->friction * frameTime);
	if (m_pGameClient->velocity.x < 0.01f)
		m_pGameClient->velocity.x = 0;
	if (m_pGameClient->velocity.y < 0.01f)
		m_pGameClient->velocity.y = 0;

	m_pGameClient->velocity += vecDesiredMove;

	// FIXME - cap velocity somewhere around here

	//Perform the actual move and update angles
	UpdatePosition(frameTime);
	UpdateViewAngles(frameTime);
	UpdateViewBlends();

	//Save current view to send to the server
	m_cmd.angles = m_pGameClient->angles;
	m_cmd.time = frameTime * 1000.0f;
	if(m_cmd.time > 255.0f)
		m_cmd.time = 255.0f;

	//Print misc crap
	m_pClGame->HudPrintf(0,100,0,"ORIGIN: %.2f, %.2f, %.2f", m_pGameClient->origin.x,m_pGameClient->origin.y,m_pGameClient->origin.z);
//	m_pClGame->HudPrintf(0,120,0,"MINS  : %.2f,%.2f,%.2f", m_pGameClient->mins.x,m_pGameClient->mins.y,m_pGameClient->mins.z);
//	m_pClGame->HudPrintf(0,140,0,"MAXS  : %.2f,%.2f,%.2f", m_pGameClient->maxs.x,m_pGameClient->maxs.y,m_pGameClient->maxs.z);
//	m_pClGame->HudPrintf(0,120,0,"FORWARD: %.2f,%.2f,%.2f", m_vecForward.x,m_vecForward.y,m_vecForward.z);
//	m_pClGame->HudPrintf(0,140,0,"UP     : %.2f,%.2f,%.2f", m_vecUp.x,m_vecUp.y,m_vecUp.z);	
//	m_pClGame->HudPrintf(0,160,0,"ANGLES : %.2f,%.2f,%.2f", m_pGameClient->angles.x,m_pGameClient->angles.y,m_pGameClient->angles.z);

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
	buf.WriteByte(m_cmd.time);
	buf.WriteByte(m_cmd.moveFlags);
	buf.WriteFloat(m_cmd.angles.x);
	buf.WriteFloat(m_cmd.angles.y);
	buf.WriteFloat(m_cmd.angles.z);
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

/*	m_pClGame->UnregisterModelCache(CACHE_GAME);
	m_pClGame->UnregisterImageCache(CACHE_GAME);
	m_pClGame->UnregisterSoundCache(CACHE_GAME);
*/

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
			{
ComPrintf("CLGAME: removed sound %d\n", i);
				m_pClGame->RemoveSoundSource(&m_entities[i]);
			}
		}
		m_entities[i].Reset();
	}

	ComPrintf("CLGAME: Unload world\n");

	m_numEnts = 0;
	m_numClients = 0;
	
	m_ingame = false;
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
{	return m_cvOutRate.ival;
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
		ComPrintf("CL: Pos %.2f %.2f %.2f\n", 
			m_pGameClient->origin.x,m_pGameClient->origin.y,m_pGameClient->origin.z);
		ComPrintf("CL: Angles %.2f %.2f %.2f\n", 
			m_pGameClient->angles.x,m_pGameClient->angles.y,m_pGameClient->angles.z);
		break;
	}
}

/*
==========================================
Validate/Handle any CVAR changes
==========================================
*/
bool CGameClient::HandleCVar(const CVarBase * cvar, const CStringVal &strval)
{
	if(cvar == reinterpret_cast<CVarBase *>(&m_cvKbSpeed))
	{
		float val = strval.FloatVal();
		if(val < 0.6 || val >= 10.0)
		{
			ComPrintf("Out of range. Should be between 0.6 and 10.0\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvInRate))
		return ValidateRate(strval);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvOutRate))
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
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvName))
		return ValidateName(strval);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvCharacter))
		return ValidateCharacter(strval);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvDefaultChar))
	{
		return false;
	}
	return false;
}













