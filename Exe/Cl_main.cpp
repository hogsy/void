#include "Sys_hdr.h"

#include "Com_vector.h"
#include "Com_util.h"
#include "Com_World.h"

#include "I_clientRenderer.h"
#include "I_renderer.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "In_defs.h"

#include "Cl_base.h"
#include "Cl_hdr.h"
#include "Cl_main.h"
#include "Cl_export.h"

enum
{
	CMD_CONNECT		  = 1,
	CMD_DISCONNECT	  = 2,
	CMD_RECONNECT	  = 3
};

//==========================================================================
//==========================================================================

/*
======================================
Constructor
======================================
*/
CClient::CClient(I_Renderer * pRenderer,
				 CSoundManager * pSound,
				 CMusic	* pMusic) :
					m_cvPort("cl_port","20011", CVAR_INT,	CVAR_ARCHIVE| CVAR_LATCH),
					m_cvNetStats("cl_netstats","1", CVAR_BOOL, CVAR_ARCHIVE),
					m_pRender(pRenderer),	
					m_pSound(pSound),
					m_pMusic(pMusic)
{
	//Get Renderer Interfaces
	m_pClRen = m_pRender->GetClient();
	m_pHud   = m_pRender->GetHud();
	
	//Create Export Struct
	m_pExports = new CClientExports(*this);
	
	//Create Game Client
	m_pClState = new CGameClient(m_pExports);
	
	//Setup network listener
	m_pNetCl= new CNetClient(m_pClState);
	
	m_clientState = CLIENT_DISCONNECTED;
	m_fFrameTime  = 0.0f;
	m_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_cvPort,this);
	System::GetConsole()->RegisterCVar(&m_cvNetStats);

	System::GetConsole()->RegisterCommand("connect", CMD_CONNECT, this);
	System::GetConsole()->RegisterCommand("disconnect", CMD_DISCONNECT, this);
	System::GetConsole()->RegisterCommand("reconnect", CMD_RECONNECT, this);
}

/*
======================================
Destroy the client
======================================
*/
CClient::~CClient()
{
	m_pNetCl->Disconnect(false);

	delete m_pClState;
	delete m_pNetCl;
	delete m_pExports;

	//Final Cleanup
	if(m_pClRen)
	{
		m_pClRen->UnloadModelAll();
		m_pClRen->UnloadImageAll();
	}
	m_pClRen = 0;
	m_pHud = 0;
	m_pRender = 0;
	m_pWorld = 0;
	
	if(m_pSound)
		m_pSound->UnregisterAll();
	m_pSound = 0;
	m_pMusic = 0;
}

/*
=====================================
Load the world for the client to render
=====================================
*/
CWorld * CClient::LoadWorld(const char *worldname)
{
	char mappath[COM_MAXPATH];
	
	strcpy(mappath,GAME_WORLDSDIR);
	strcat(mappath, worldname);
	Util::SetDefaultExtension(mappath,VOID_DEFAULTMAPEXT);

	m_pWorld = CWorld::CreateWorld(mappath);

	if(!m_pWorld)
		ComPrintf("CClient::LoadWorld: World not found\n");
	else
	{
		if(m_pRender->LoadWorld(m_pWorld,1))
		{
			ComPrintf("CClient::LoadWorld OK\n");
			return m_pWorld;
		}
		ComPrintf("CClient::LoadWorld: Renderer couldnt load world\n");
	}

	m_pNetCl->Disconnect(false);
	return 0;


/*	if(m_pWorld)
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
*/
}

/*
=====================================
Unload the world. Free data caches
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
	m_pSound->UnregisterCache(CACHE_GAME);

//	m_pClState->UnloadWorld();

	if(m_pWorld)
		CWorld::DestroyWorld(m_pWorld);
	m_pWorld = 0;

	System::SetGameState(INCONSOLE);
}

/*
======================================
Client frame
======================================
*/
void CClient::RunFrame()
{
	m_pNetCl->ReadPackets();

	if(m_clientState == CLIENT_INGAME)
	{
		//Print FPS, Update frame time
		m_pHud->Printf(0,50,0, "%3.2f : %4.2f", 1/(System::GetCurTime() - m_fFrameTime), System::GetCurTime());
		m_pHud->Printf(0,70,0, "%d",  (int)(System::GetFrameTime() * 1000));
		m_fFrameTime = System::GetCurTime();

		//Draw NetStats if flagged
		if(m_cvNetStats.bval)
			ShowNetStats();

		//Run Client Frame. All game related processing/drawing
		m_pClState->RunFrame(System::GetFrameTime());
		
		//Update Sound engine with client new pos
		m_pSound->UpdateListener(m_pClState->GetCamera());

		//Draw from the client view point
		m_pRender->Draw(m_pClState->GetCamera());
		
		//Have the client write any outgoing data
		WriteUpdate();
	}
	else
	{
		m_pRender->Draw();
	}
	
	//Write updates
	m_pNetCl->SendUpdate();
}

/*
======================================
Write to the outgoing network buffer
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

/*
================================================
Update Client state
================================================
*/
void CClient::SetClientState(int state)
{
	switch(state)
	{
	case CLIENT_DISCONNECTED:
		m_pNetCl->Disconnect(true);
		break;
	case CLIENT_RECONNECTING:
		m_pNetCl->Reconnect(true);
		break;
	case CLIENT_INGAME:
		System::SetGameState(::INGAME);
		break;
	default:
		return;
	};
	m_clientState = state;
}

/*
================================================
Client system got/lost input focus. 
Route it according
================================================
*/
void CClient::SetInputState(bool on)  
{	
	if(on)
	{
		System::GetInputFocusManager()->SetCursorListener(m_pClState->GetCursorListener());
		System::GetInputFocusManager()->SetKeyListener(m_pClState->GetKeyListener(),false);
	}
	else
	{
		System::GetInputFocusManager()->SetCursorListener(0);
		System::GetInputFocusManager()->SetKeyListener(0);
	}

}


/*
======================================
Displays network stats on screen.
======================================
*/
void CClient::ShowNetStats()
{
	//Print Networking stats
	const NetChanState & chanState = m_pNetCl->GetChanState();

	m_pHud->Printf(0,390,0, "Latency %.2f ms", chanState.latency * 1000);
	m_pHud->Printf(0,400,0, "Drop stats %d/%d. Choked %d", chanState.dropCount, 
						chanState.dropCount + chanState.goodCount, chanState.numChokes);
	m_pHud->Printf(0,410,0, "In      %d", chanState.inMsgId);
	m_pHud->Printf(0,420,0, "In  Ack %d", chanState.inAckedId);
	m_pHud->Printf(0,430,0, "Out     %d", chanState.outMsgId);
	m_pHud->Printf(0,440,0, "Out Ack %d", chanState.lastOutReliableId);

}

void CClient::SetNetworkRate(int rate)
{	m_pNetCl->SetRate(rate);
}





//==========================================================================
//==========================================================================

/*
==========================================
Handle Registered Commands
==========================================
*/
void CClient::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_CONNECT:
		{
			char addr[NET_IPADDRLEN];
			parms.StringTok(1,addr,NET_IPADDRLEN);
			m_pNetCl->ConnectTo(addr);
			break;
		}
	case CMD_DISCONNECT:
		m_clientState = CLIENT_DISCONNECTED;
		m_pNetCl->Disconnect(false);
		break;
	case CMD_RECONNECT:
		m_clientState = CLIENT_RECONNECTING;
		m_pNetCl->Reconnect(false);
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
	return false;
}
