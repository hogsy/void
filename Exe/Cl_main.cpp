#include "Cl_main.h"
#include "I_renderer.h"
#include "I_hud.h"
#include "Snd_main.h"
#include "Mus_main.h"
#include "Cl_cmds.h"
#include "Com_util.h"
#include "Net_defs.h"
#include "Net_protocol.h"

//======================================================================================
//======================================================================================
world_t	*g_pWorld;
int PointContents(vector_t &v);

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
					m_cvKbSpeed("cl_kbspeed","0.6", CVAR_FLOAT, CVAR_ARCHIVE),
					m_pRender(prenderer),	
					m_pSound(psound),
					m_pMusic(pmusic)
{

	m_pHud   = m_pRender->GetHud();
	m_pModel = m_pRender->GetModel();
	m_pImage = m_pRender->GetImage();

	m_pCmdHandler = new CClientCmdHandler(*this);

	//Setup network listener
	m_pNetCl= new CNetClient(this);
	m_pNetCl->SetRate(2500);

	m_ingame = false;
	m_fFrameTime = 0.0f;

	m_numEnts = 0;
	
	m_pCamera = 0;
	
	g_pWorld = 0;

	m_hsTalk = 0;
	m_hsMessage = 0;

	System::GetConsole()->RegisterCVar(&m_cvClip);
	System::GetConsole()->RegisterCVar(&m_cvKbSpeed,this);
	System::GetConsole()->RegisterCVar(&m_cvPort,this);
	System::GetConsole()->RegisterCVar(&m_cvRate,this);
	System::GetConsole()->RegisterCVar(&m_cvName,this);
	System::GetConsole()->RegisterCVar(&m_cvModel,this);
	System::GetConsole()->RegisterCVar(&m_cvSkin,this);
	
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
	System::GetConsole()->RegisterCommand("connect", CMD_CONNECT, this);
	System::GetConsole()->RegisterCommand("disconnect", CMD_DISCONNECT, this);
	System::GetConsole()->RegisterCommand("reconnect", CMD_RECONNECT, this);
	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);

	m_pCmdHandler->IntializeBinds();
}

/*
======================================
Destroy the client
======================================
*/
CClient::~CClient()
{
	m_pCmdHandler->WriteBinds("vbinds.cfg");

	m_pNetCl->Disconnect(false);

	if(m_pCamera)
		delete m_pCamera;

	if(m_pModel)
		m_pModel->UnloadModelAll();
	
//	if(m_pImage)
//		m_pImage->UnloadImageAll();


	m_pRender = 0;
	m_pHud = 0;
	m_pModel = 0;
	m_pImage =0;

	m_pSound = 0;
	m_pMusic = 0;

	g_pWorld = 0;

	delete m_pNetCl;
	delete m_pCmdHandler;
}

/*
=====================================
Load the world for the client to render
=====================================
*/
bool CClient::LoadWorld(const char *worldname)
{
	char mappath[COM_MAXPATH];
	
	strcpy(mappath,szWORLDDIR);
	strcat(mappath, worldname);
	Util::SetDefaultExtension(mappath,".bsp");

	g_pWorld = world_create(mappath);
	if(!g_pWorld)
	{
		ComPrintf("CClient::LoadWorld: World not found\n");
		return false;
	}

	// load the textures
	if(!m_pRender->LoadWorld(g_pWorld,1))
	{
		ComPrintf("CClient::LoadWorld: Renderer couldnt load world\n");
		return false;
	}

	m_hsTalk    = m_pSound->RegisterSound("sounds/talk.wav", CACHE_LOCAL);
	m_hsMessage = m_pSound->RegisterSound("sounds/message.wav", CACHE_LOCAL);

	
	ComPrintf("CClient::Load World: OK\n");
	return true;
}

/*
======================================
Enter game
======================================
*/
void CClient::BeginGame()
{
	m_campath = -1;
	m_acceleration = 400.0f;
	m_maxvelocity =  200.0f;
	
	VectorSet(&desired_movement, 0, 0, 0);

	VectorSet(&m_gameClient.angle, 0.0f,0.0f,0.0f);
	VectorSet(&m_gameClient.origin, 0.0f,0.0f,32.0f);	// FIXME - origin + view height
	VectorSet(&m_gameClient.mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&m_gameClient.maxs, 10.0f, 10.0f, 10.0f);
	VectorSet(&m_screenBlend,0.0f,0.0f,0.0f);
	
	
	m_pCamera = new CCamera(m_gameClient.origin, m_gameClient.angle, m_screenBlend);

	m_ingame = true;

	System::SetGameState(INGAME);
	SetInputState(true);

	Spawn(0,0);
}

/*
=====================================
Unload the world
=====================================
*/
void CClient::UnloadWorld()
{
	if(!m_ingame)
		return;

	if(!m_pRender->UnloadWorld())
	{
		ComPrintf("CClient::UnloadWorld - Renderer couldnt unload world\n");
		return;
	}

	m_pModel->UnloadModelCache(CACHE_GAME);

	delete m_pCamera;
	m_pCamera = 0;


	int i;
	for(i=0; i< GAME_MAXCLIENTS; i++)
		if(m_clients[i].inUse) 
			m_clients[i].Reset();

	for(i=0; i< GAME_MAXENTITIES; i++)
		if(m_entities[i].inUse)
			m_entities[i].Reset();
	
	world_destroy(g_pWorld);
	g_pWorld = 0;

	m_pSound->UnregisterAll();
	System::SetGameState(INCONSOLE);

	m_ingame = false;
}

/*
======================================
Client frame
======================================
*/
void CClient::RunFrame()
{
	m_pNetCl->ReadPackets();

	if(m_ingame)
	{
		m_pCmdHandler->RunCommands();

		if (!((desired_movement.x==0) && 
			  (desired_movement.y==0) && 
			  (desired_movement.z==0)) || 
			 (m_campath != -1))
		{
			VectorNormalize(&desired_movement);
			Move(&desired_movement, System::g_fframeTime * m_maxvelocity);
			desired_movement.x = desired_movement.y = desired_movement.z = 0;
		}

		//Print Stats
		m_pHud->HudPrintf(0, 50,0, "%.2f, %.2f, %.2f", m_gameClient.origin.x,
													   m_gameClient.origin.y,
													   m_gameClient.origin.z);

		m_pHud->HudPrintf(0, 70,0, "%.2f : %.2f : %f", 
			1/(System::g_fcurTime - m_fFrameTime), System::g_fcurTime, System::g_fframeTime);
		m_fFrameTime = System::g_fcurTime;


		vector_t forward, up, velocity;
		VectorSet(&velocity, 0,0,0);
		AngleToVector(&m_gameClient.angle, &forward, 0, &up);
		m_pHud->HudPrintf(0, 90,0, "FORWARD: %.2f, %.2f, %.2f", forward.x, forward.y, forward.z);
		m_pHud->HudPrintf(0, 110,0,"UP     : %.2f, %.2f, %.2f", up.x,  up.y,  up.z);		

		//Print Networking stats
		const NetChanState & chanState = m_pNetCl->GetChanState();

		m_pHud->HudPrintf(0,390,0, "Latency %f", chanState.latency);
		m_pHud->HudPrintf(0,400,0, "Drop stats %d/%d. Choked %d", chanState.dropCount, 
							chanState.dropCount + chanState.goodCount, chanState.numChokes);
		m_pHud->HudPrintf(0,410,0, "In      %d", chanState.inMsgId);
		m_pHud->HudPrintf(0,420,0, "In  Ack %d", chanState.inAckedId);
		m_pHud->HudPrintf(0,430,0, "Out     %d", chanState.outMsgId);
		m_pHud->HudPrintf(0,440,0, "Out Ack %d", chanState.lastOutReliableId);

		// FIXME - put this in game dll
		int contents = PointContents(m_gameClient.origin);
		if(contents & CONTENTS_SOLID)
			VectorSet(&m_screenBlend, 0.4f, 0.4f, 0.4f);
		else if(contents & CONTENTS_WATER)
			VectorSet(&m_screenBlend, 0, 1, 1);
		else if(contents & CONTENTS_LAVA)
			VectorSet(&m_screenBlend, 1, 0, 0);
		else
			VectorSet(&m_screenBlend, 1, 1, 1);

		m_pSound->UpdateListener(m_gameClient.origin, velocity, up, forward);

		//fix me. draw ents only in the pvs
		for(int i=0; i< GAME_MAXENTITIES; i++)
		{
			if(m_entities[i].inUse && m_entities[i].index >= 0)
			{
				m_pModel->DrawModel(m_entities[i]);	
			}
		}


		for(i=0; i< GAME_MAXCLIENTS; i++)
		{
			if(m_clients[i].inUse && m_clients[i].index >=0)
			{
				m_pModel->DrawModel(m_clients[i]);
			}
		}

		m_pRender->Draw(m_pCamera);

		if(m_pNetCl->CanSend())
		{

			//Write all updates
			CBuffer &buf = m_pNetCl->GetSendBuffer();
			
			buf.Reset();
			buf.WriteByte(CL_MOVE);
			buf.WriteCoord(m_gameClient.origin.x);
			buf.WriteCoord(m_gameClient.origin.y);
			buf.WriteCoord(m_gameClient.origin.z);
			buf.WriteAngle(m_gameClient.angle.x);
			buf.WriteAngle(m_gameClient.angle.y);
			buf.WriteAngle(m_gameClient.angle.z);
		}
	}
	else
	{
		//draw the console or menues etc
		m_pRender->DrawConsole();
	}

	//Write updates
	m_pNetCl->SendUpdate();

}


//======================================================================================
//======================================================================================

void CClient::SetInputState(bool on)  {	m_pCmdHandler->SetListenerState(on); }

/*
==========================================
Handle Registered Commands
==========================================
*/
void CClient::HandleCommand(HCMD cmdId, const CParms &parms)
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
	case CMD_CAM:
		CamPath();
		break;
	case CMD_CONNECT:
		{
			char addr[MAX_IPADDR_LEN];
			parms.StringTok(1,addr,MAX_IPADDR_LEN);
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
	else if(cvar == reinterpret_cast<CVarBase *>(&m_cvKbSpeed))
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


void CClient::Spawn(vector_t * origin, vector_t *angles)
{
	static ClEntity entHowl;
	
	entHowl.soundIndex = m_pSound->RegisterSound("sounds/wind.wav", CACHE_LOCAL);
	Void3d::VectorSet(entHowl.origin,0,0,48);
	m_pSound->PlaySnd(&entHowl, entHowl.soundIndex, CACHE_LOCAL, 0,0, CHAN_WORLD | CHAN_LOOPING);
}
