#include "Sys_hdr.h"

#include "Com_buffer.h"
#include "Com_vector.h"
#include "Com_util.h"
#include "Com_World.h"

#include "Cl_main.h"

#include "I_clientRenderer.h"
#include "I_renderer.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"

#include "Net_defs.h"
#include "Net_protocol.h"
#include "Net_client.h"


#include "Cl_cmds.h"
#include "Cl_game.h"


/*
======================================
Constructor
======================================
*/
CClient::CClient(I_Renderer * prenderer,
				 CSoundManager * psound,
				 CMusic	* pmusic):
					m_cvClip("cl_clip","1",     CVAR_BOOL,0),
					m_cvPort("cl_port","20011", CVAR_INT,	CVAR_ARCHIVE| CVAR_LATCH),
					m_cvRate("cl_rate","2500",	CVAR_INT,	CVAR_ARCHIVE),
					m_cvName("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE),
					m_cvModel("cl_model", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE),
					m_cvSkin("cl_skin", "Ratamahatta", CVAR_STRING, CVAR_ARCHIVE),
					m_cvNetStats("cl_netstats","1", CVAR_BOOL, CVAR_ARCHIVE),
					m_pRender(prenderer),	
					m_pSound(psound),
					m_pMusic(pmusic)
{
	m_pClRen = m_pRender->GetClient();
	m_pHud   = m_pRender->GetHud();

	//Setup network listener
	m_pClState = new CGameClient(*this, m_pClRen, m_pHud, m_pSound, m_pMusic);
	m_pNetCl= new CNetClient(m_pClState);
	

	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvClip);
	System::GetConsole()->RegisterCVar(&m_cvNetStats);
	System::GetConsole()->RegisterCVar(&m_cvPort,this);
	
	System::GetConsole()->RegisterCVar(&m_cvRate,this);
	System::GetConsole()->RegisterCVar(&m_cvName,this);
	System::GetConsole()->RegisterCVar(&m_cvModel,this);
	System::GetConsole()->RegisterCVar(&m_cvSkin,this);

	System::GetConsole()->RegisterCommand("connect", CMD_CONNECT, this);
	System::GetConsole()->RegisterCommand("disconnect", CMD_DISCONNECT, this);
	System::GetConsole()->RegisterCommand("reconnect", CMD_RECONNECT, this);
	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);

	m_pNetCl->SetRate(m_cvRate.ival);
}

/*
======================================
Destroy the client
======================================
*/
CClient::~CClient()
{
	m_pNetCl->Disconnect(false);

	if(m_pClRen)
	{
		m_pClRen->UnloadModelAll();
		m_pClRen->UnloadImageAll();
	}
	m_pClRen = 0;
	m_pHud = 0;
	m_pRender = 0;
	
	m_pSound = 0;
	m_pMusic = 0;


	delete m_pClState;

	delete m_pNetCl;
}

/*
=====================================
Load the world for the client to render
=====================================
*/
bool CClient::LoadWorld(const char *worldname)
{
	char mappath[COM_MAXPATH];
	
	strcpy(mappath,GAME_WORLDSDIR);
	strcat(mappath, worldname);
	Util::SetDefaultExtension(mappath,VOID_DEFAULTMAPEXT);

	m_pWorld = CWorld::CreateWorld(mappath);

	if(!m_pWorld)
	{
		ComPrintf("CClient::LoadWorld: World not found\n");
		m_pNetCl->Disconnect(false);
		return false;
	}

	// load the textures
	if(!m_pRender->LoadWorld(m_pWorld,1))
	{
		ComPrintf("CClient::LoadWorld: Renderer couldnt load world\n");
		m_pNetCl->Disconnect(false);
		return false;
	}

	m_pClState->LoadWorld(m_pWorld);

	ComPrintf("CClient::Load World: OK\n");
	return true;
}

/*
=====================================
Unload the world
=====================================
*/
void CClient::UnloadWorld()
{
	if(!m_pWorld)
		return;

	if(!m_pRender->UnloadWorld())
	{
		ComPrintf("CClient::UnloadWorld - Renderer couldnt unload world\n");
		return;
	}

	m_pClRen->UnloadModelCache(CACHE_GAME);
	m_pClRen->UnloadImageCache(CACHE_GAME);

	m_pClState->UnloadWorld();

	CWorld::DestroyWorld(m_pWorld);
	m_pWorld = 0;

	m_pSound->UnregisterAll();
	System::SetGameState(INCONSOLE);

	m_pClState->m_ingame = false;
}

/*
======================================
Client frame
======================================
*/
void CClient::RunFrame()
{
	m_pNetCl->ReadPackets();

	//draw the console or menues etc
	if(!m_pClState->m_ingame)
	{
		m_pRender->Draw();
		
	}
	else 
	{
		m_pClState->RunFrame(System::GetFrameTime());

		m_pHud->Printf(0, 70,0, "%3.2f : %4.2f", 
			1/(System::GetCurTime() - m_fFrameTime), System::GetCurTime());
		
		m_pHud->Printf(0, 150,0, "%d", (int)(System::GetFrameTime() * 1000));

		m_fFrameTime = System::GetCurTime();

		vector_t forward, up, velocity;
		VectorSet(&velocity, 0,0,0);
		AngleToVector(&m_pClState->m_pGameClient->angles, &forward, 0, &up);
		m_pHud->Printf(0, 90,0, "FORWARD: %.2f, %.2f, %.2f", forward.x, forward.y, forward.z);
		m_pHud->Printf(0, 110,0,"UP     : %.2f, %.2f, %.2f", up.x,  up.y,  up.z);		

		if(m_cvNetStats.bval)
			ShowNetStats();

		m_pSound->UpdateListener(m_pClState->m_pGameClient->origin, velocity, up, forward);

		//fix me. draw ents only in the pvs
		for(int i=0; i< GAME_MAXENTITIES; i++)
		{
			if(m_pClState->m_entities[i].inUse && (m_pClState->m_entities[i].mdlIndex >= 0))
				m_pClRen->DrawModel(m_pClState->m_entities[i]);	
		}
		
		//Draw clients
		for(i=0; i< GAME_MAXCLIENTS; i++)
		{
			if(m_pClState->m_clients[i].inUse && m_pClState->m_clients[i].mdlIndex >=0)
				m_pClRen->DrawModel(m_pClState->m_clients[i]);
		}

		m_pClState->UpdateView();
		m_pRender->Draw(m_pClState->m_pCamera);
		
		WriteUpdate();
	}
	
	//Write updates
	m_pNetCl->SendUpdate();
}


/*
======================================

======================================
*/
void CClient::WriteUpdate()
{
	//Write any updates
	if(m_pNetCl->CanSend())
	{
		//Write all updates
		CBuffer &buf = m_pNetCl->GetSendBuffer();
		
		buf.Reset();
		buf.WriteByte(CL_MOVE);
		buf.WriteFloat(m_fFrameTime);

		m_pClState->WriteCmdUpdate(buf);
	}
}



//======================================================================================
//======================================================================================

void CClient::SetInputState(bool on)  
{	
	if(on == true)
	{
		System::GetInputFocusManager()->SetCursorListener(m_pClState->m_pCmdHandler);
		System::GetInputFocusManager()->SetKeyListener(m_pClState->m_pCmdHandler,false);
	}
	else
	{
		System::GetInputFocusManager()->SetCursorListener(0);
		System::GetInputFocusManager()->SetKeyListener(0);
	}

}

/*
==========================================
Handle Registered Commands
==========================================
*/
void CClient::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
/*	case CMD_MOVE_FORWARD:
		m_pClState->MoveForward();
		break;
	case CMD_MOVE_BACKWARD:
		m_pClState->MoveBackward();
		break;
	case CMD_MOVE_LEFT:
		m_pClState->MoveLeft();
		break;
	case CMD_MOVE_RIGHT:
		m_pClState->MoveRight();
		break;
	case CMD_ROTATE_LEFT:
		m_pClState->RotateLeft(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_RIGHT:
		m_pClState->RotateRight(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_UP:
		m_pClState->RotateUp(m_cvKbSpeed.fval);
		break;
	case CMD_ROTATE_DOWN:
		m_pClState->RotateDown(m_cvKbSpeed.fval);
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
*/

	case CMD_CAM:
//		CamPath();
		break;
	case CMD_CONNECT:
		{
			char addr[NET_IPADDRLEN];
			parms.StringTok(1,addr,NET_IPADDRLEN);
			m_pNetCl->ConnectTo(addr);
			break;
		}
	case CMD_DISCONNECT:
		m_pNetCl->Disconnect(false);
		break;
	case CMD_RECONNECT:
		m_pNetCl->Reconnect(false);
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
bool CClient::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == reinterpret_cast<CVarBase*>(&m_cvPort))
	{
		int port = parms.IntTok(1);
		if(port < 1024 || port > 32767)
		{
			ComPrintf("Port is out of range, select another\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvRate))
		return ValidateRate(parms);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_cvName))
		return ValidateName(parms);
/*	else if(cvar == reinterpret_cast<CVarBase *>(&m_cvKbSpeed))
	{
		float val = parms.FloatTok(1);
		if(val <= 0.0 || val >= 1.0)
		{
			ComPrintf("Out of range. Should be between 0.0 and 1.0\n");
			return false;
		}
		return true;
	}
*/
	return false;
}


void CClient::SetState(int state)
{
	switch(state)
	{
	case CL_DISCONNECTED:
		m_pNetCl->Disconnect(true);
		break;
	case CL_RECONNECTING:
		m_pNetCl->Reconnect(true);
		break;
	case CL_INGAME:
		System::SetGameState(::INGAME);
		SetInputState(true);
		break;
	};
}

