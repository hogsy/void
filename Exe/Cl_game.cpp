#include "Sys_hdr.h"
#include "Com_vector.h"
#include "Com_camera.h"
#include "Com_world.h"
#include "Com_buffer.h"
#include "Cl_defs.h"
#include "Cl_main.h"
#include "Cl_game.h"
#include "Cl_cmds.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"


CGameClient::	CGameClient(CClient & rClient, I_HudRenderer * pHud,
				 CSoundManager * pSound,
				CMusic		   * pMusic) : 
			m_rClient(rClient), m_pHud(pHud), m_pSound(pSound), m_pMusic(pMusic),
			m_cvKbSpeed("cl_kbspeed","0.6", CVAR_FLOAT, CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientGameInput(*this);

	m_ingame = false;

	m_pGameClient = 0;

	m_numEnts = 0;
	
	m_pCamera = 0;
	
	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvKbSpeed,this);

	System::GetConsole()->RegisterCommand("+forward",CMD_MOVE_FORWARD,this);
	System::GetConsole()->RegisterCommand("+back",CMD_MOVE_BACKWARD,this);
	System::GetConsole()->RegisterCommand("+moveleft",CMD_MOVE_LEFT,this);
	System::GetConsole()->RegisterCommand("+moveright",CMD_MOVE_RIGHT,this);
	System::GetConsole()->RegisterCommand("+right",CMD_ROTATE_RIGHT,this);
	System::GetConsole()->RegisterCommand("+left",CMD_ROTATE_LEFT,this);
	System::GetConsole()->RegisterCommand("+lookup",CMD_ROTATE_UP,this);
	System::GetConsole()->RegisterCommand("+lookdown",CMD_ROTATE_DOWN,this);

	System::GetConsole()->RegisterCommand("bind",CMD_BIND,this);
	System::GetConsole()->RegisterCommand("bindlist",CMD_BINDLIST,this);
	System::GetConsole()->RegisterCommand("cam",CMD_CAM,this);
	System::GetConsole()->RegisterCommand("unbind",CMD_UNBIND,this);
	System::GetConsole()->RegisterCommand("unbindall",CMD_UNBINDALL,this);

	m_pCmdHandler->IntializeBinds();

}

CGameClient::~CGameClient()
{
	m_pCmdHandler->WriteBinds("vbinds.cfg");

	if(m_pCamera)
		delete m_pCamera;
	m_pWorld = 0;

	m_pHud=0;
	m_pSound=0;
	m_pMusic=0;

	delete m_pCmdHandler;

	m_pGameClient = 0;
}


void CGameClient::Spawn(vector_t	*origin, vector_t *angles)
{
}

void CGameClient::RunFrame(float frameTime)
{
	m_pCmdHandler->RunCommands();

//	RotateRight(m_moveAngles.x);
//	RotateUp(m_moveAngles.y);

//	m_moveAngles.Set(0,0,0);

	VectorNormalize(&desired_movement);
	Move(desired_movement, frameTime * m_maxvelocity);
	desired_movement.Set(0,0,0);

	m_pHud->Printf(0, 50,0, "%.2f, %.2f, %.2f", 
				m_pGameClient->origin.x,  m_pGameClient->origin.y, m_pGameClient->origin.z);
}


/*
======================================

======================================
*/
void CGameClient::WriteCmdUpdate(CBuffer &buf)
{
	buf.WriteShort(m_cmd.forwardmove);
	buf.WriteShort(m_cmd.rightmove);
	buf.WriteShort(m_cmd.upmove);

	buf.WriteInt(m_cmd.angles[0]);
	buf.WriteInt(m_cmd.angles[1]);
	buf.WriteInt(m_cmd.angles[2]);
}


/*
======================================

======================================
*/
void CGameClient::UpdateView()
{
	// FIXME - put this in game dll
	int contents = m_pWorld->PointContents(m_pGameClient->origin);
	if(contents & CONTENTS_SOLID)
		VectorSet(&m_screenBlend, 0.4f, 0.4f, 0.4f);
	else if(contents & CONTENTS_WATER)
		VectorSet(&m_screenBlend, 0, 1, 1);
	else if(contents & CONTENTS_LAVA)
		VectorSet(&m_screenBlend, 1, 0, 0);
	else
		VectorSet(&m_screenBlend, 1, 1, 1);
}


//spawn for the first time.
void CGameClient::BeginGame()
{
	m_campath = -1;
	m_maxvelocity =  200.0f;
	
	m_pGameClient->moveType = MOVETYPE_STEP;
	VectorSet(&m_pGameClient->angles, 0.0f,0.0f,0.0f);
	VectorSet(&m_pGameClient->origin, 0.0f,0.0f,48.0f);
	VectorSet(&m_pGameClient->mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&m_pGameClient->maxs, 10.0f, 10.0f, 10.0f);
	VectorSet(&m_screenBlend,0.0f,0.0f,0.0f);

	VectorSet(&desired_movement, 0, 0, 0);

	//Register static sound sources with SoundManager
	for(int i=0; i< GAME_MAXENTITIES; i++)
	{
		if(m_entities[i].inUse && m_entities[i].sndIndex > -1)
		{
			m_entities[i].sndCache = CACHE_GAME;
			m_entities[i].volume = 10;
			m_entities[i].attenuation = 5;
			m_pSound->AddStaticSource(&m_entities[i]);
		}
	}
	
	
	m_pCamera = new CCamera(m_pGameClient->origin, m_pGameClient->angles, m_screenBlend);

	m_ingame = true;

	Spawn(0,0);
}


bool CGameClient::LoadWorld(CWorld * pWorld)
{
	//setup
	m_pWorld = pWorld;
	EntMove::SetWorld(m_pWorld);

	ComPrintf("CGameClient::Load World: OK\n");
	return true;

}

void CGameClient::UnloadWorld()
{
	if(!m_ingame)
		return;

	EntMove::SetWorld(0);

	delete m_pCamera;
	m_pCamera = 0;

	m_pGameClient = 0;

	int i;
	for(i=0; i< GAME_MAXCLIENTS; i++)
		if(m_clients[i].inUse) 
			m_clients[i].Reset();

	for(i=0; i< GAME_MAXENTITIES; i++)
		if(m_entities[i].inUse)
		{
			if(m_entities[i].sndIndex > -1)
				m_pSound->RemoveStaticSource(&m_entities[i]);
			m_entities[i].Reset();
		}

	m_pWorld = 0;

	m_ingame = false;
}




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
		if(val <= 0.0 || val >= 1.0)
		{
			ComPrintf("Out of range. Should be between 0.0 and 1.0\n");
			return false;
		}
		return true;
	}
	return false;
}

