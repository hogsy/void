#include "Cl_main.h"
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
					m_noclip("cl_noclip","0",   CVAR_INT,0),
					m_clport("cl_port","20011", CVAR_INT,	CVAR_ARCHIVE| CVAR_LATCH),
					m_clrate("cl_rate","2500",	CVAR_INT,	CVAR_ARCHIVE),
					m_clname("cl_name","Player",CVAR_STRING,CVAR_ARCHIVE),
					m_pRender(prenderer),	
					m_pSound(psound),
					m_pMusic(pmusic)
{
	m_pCmdHandler = new CClientCmdHandler(*this);
	
	//Setup network listener
	m_pNetCl= new CNetClient(this);
	m_pNetCl->SetRate(2500);

	m_ingame = false;
	m_fFrameTime = 0.0f;
	
	m_pHud = 0;
	m_pModel = 0;
	
	m_pCamera = 0;
	
	g_pWorld = 0;

	m_hsTalk = 0;
	m_hsMessage = 0;

	System::GetConsole()->RegisterCVar(&m_noclip);
	System::GetConsole()->RegisterCVar(&m_clport,this);
	System::GetConsole()->RegisterCVar(&m_clrate,this);
	System::GetConsole()->RegisterCVar(&m_clname,this);
	
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

	m_pCmdHandler->SetDefaultBinds();
}

/*
======================================
Destroy the client
======================================
*/
CClient::~CClient()
{
	m_pNetCl->Disconnect(false);

	if(m_pCamera)
		delete m_pCamera;

	if(m_pModel)
		m_pModel->UnloadModelAll();

	m_pRender = 0;
	m_pHud = 0;
	m_pModel = 0;

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
//	char configname[128];
//	strcpy(configname,g_gamedir);
//	strcat(configname,"\\void.cfg");
//	g_pCons->ExecConfig(configname);

	m_pHud = m_pRender->GetHud();
	if(!m_pHud)
	{
		ComPrintf("CClient::Init:: Couldnt get hud interface from renderer\n");
		return false;
	}

	m_pModel = m_pRender->GetModel();
	if(!m_pModel)
	{
		ComPrintf("CClient::Init:: Couldnt get model interface from renderer\n");
		return false;
	}

	//Create local ent
/*	Void3d::VectorSet(m_entQuad.angle,0,0,0);
	Void3d::VectorSet(m_entQuad.origin,0,0, 32);
	
	m_entQuad.frame = 0;
	m_entQuad.skinnum= 0;
	m_entQuad.cache = MODEL_CACHE_LOCAL;
	
//	m_entQuad.index = m_pModel->LoadModel("Models/Quad/tris.md2", -1 ,MODEL_CACHE_LOCAL);
	m_entQuad.index = m_pModel->LoadModel("models/players/Hueteotl/tris.md2", 1, MODEL_CACHE_LOCAL);
*/	


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

	m_hsTalk    = m_pSound->RegisterSound("sounds/talk.wav");
	m_hsMessage = m_pSound->RegisterSound("sounds/message.wav");

	
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

	VectorSet(&m_gameClient.angles, 0.0f,0.0f,0.0f);
	VectorSet(&m_gameClient.origin, 0.0f,0.0f,48.0f);	// FIXME - origin + view height
	VectorSet(&m_gameClient.mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&m_gameClient.maxs, 10.0f, 10.0f, 10.0f);
	VectorSet(&m_screenBlend,0.0f,0.0f,0.0f);

	m_pCamera = new CCamera(m_gameClient.origin, m_gameClient.angles, m_screenBlend);

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

	m_pModel->UnloadModelCache(MODEL_CACHE_GAME);

	delete m_pCamera;
	m_pCamera = 0;

	
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

		m_pHud->HudPrintf(0, 70,0, "%.2f : %.2f", 1/(System::g_fcurTime - m_fFrameTime), System::g_fcurTime);
		m_fFrameTime = System::g_fcurTime;


		vector_t forward, up, velocity;
		VectorSet(&velocity, 0,0,0);
		AngleToVector(&m_gameClient.angles, &forward, 0, &up);
		m_pHud->HudPrintf(0, 90,0, "FORWARD: %.2f, %.2f, %.2f", forward.x, forward.y, forward.z);
		m_pHud->HudPrintf(0, 110,0,"UP     : %.2f, %.2f, %.2f", up.x,  up.y,  up.z);		

		//Print Networking stats
		const NetChanState & chanState = m_pNetCl->GetChanState();
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

		//draw the ents in pvs


/*	m_entQuad.frame = 0;
	m_entQuad.skinnum= 0;
	m_entQuad.cache = MODEL_CACHE_LOCAL;
	m_entQuad.index = m_pModel->LoadModel("Models/Quad/tris.md2", -1 ,MODEL_CACHE_LOCAL);
*/
//		m_pModel->DrawModel(m_entQuad.index, MODEL_CACHE_LOCAL, m_entQuad);

		m_pRender->Draw(m_pCamera);
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
void CClient::WriteBindTable(FILE *fp){	m_pCmdHandler->WriteBindTable(fp);   }

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
		RotateLeft();
		break;
	case CMD_ROTATE_RIGHT:
		RotateRight();
		break;
	case CMD_ROTATE_UP:
		RotateUp();
		break;
	case CMD_ROTATE_DOWN:
		RotateDown();
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
		m_pNetCl->Disconnect(true);
		break;
	case CMD_RECONNECT:
		m_pNetCl->Reconnect();
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
	if(cvar == reinterpret_cast<CVarBase*>(&m_clport))
	{
		int port = parms.IntTok(1);
		if(port < 1024 || port > 32767)
		{
			ComPrintf("Port is out of range, select another\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase*>(&m_clrate))
		return ValidateRate(parms);
	else if(cvar == reinterpret_cast<CVarBase*>(&m_clname))
		return ValidateName(parms);
	return false;
}


void CClient::Spawn(vector_t * origin, vector_t *angles)
{
/*	static int hHowl = 0;
	if(!hHowl)
		hHowl = m_pSound->RegisterSound("sounds/wind.wav");

	static vector_t horigin;
	VectorSet(&horigin,0,0,48);
	m_pSound->PlaySnd(hHowl, VoidSound::CHAN_WORLD, &horigin, 0, true);
*/
}
