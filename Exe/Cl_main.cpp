//#include "Cl_net.h"
#include "Cl_main.h"
#include "Cl_cmds.h"
#include "Com_util.h"
#include "Net_defs.h"

//======================================================================================
//======================================================================================
world_t	*g_pWorld;
int PointContents(vector_t &v);

//using namespace VoidNet;

/*
======================================
Constructor
======================================
*/
CClient::CClient(I_Renderer * prenderer):
					m_noclip("cl_noclip","0",   CVar::CVAR_INT,0),
					m_clport("cl_port","20011", CVar::CVAR_INT,	CVar::CVAR_ARCHIVE| CVar::CVAR_LATCH),
					m_clrate("cl_rate","2500",	CVar::CVAR_INT,	CVar::CVAR_ARCHIVE),
					m_clname("cl_name","Player",CVar::CVAR_STRING,CVar::CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientCmdHandler(*this);
	m_pNetCl	  = new CNetClient(this);

	m_ingame = false;
	
	m_fFrameTime = 0.0f;
	
	m_pHud = 0;
	m_pRender = prenderer;

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
	m_pNetCl->Disconnect();
//	UnloadWorld();

	m_pRender = 0;
	m_pHud = 0;

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

	char mappath[COM_MAXPATH];
	
//	strcpy(mappath,VoidNet::szWORLDDIR);
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

	m_pHud = m_pRender->GetHud();
	if(!m_pHud)
	{
		ComPrintf("CClient::Init:: Couldnt get hud interface from renderer\n");
		return false;
	}

// FIXME - should be taken care of at spawn
// FIXME - should be actual player size
	m_campath = -1;
	m_acceleration = 400.0f;
	m_maxvelocity =  200.0f;
	VectorSet(&eye.mins, -10, -10, -40);
	VectorSet(&eye.maxs,  10,  10,  10);
	VectorSet(&desired_movement, 0, 0, 0);

	eye.angles.ROLL = 0;
	eye.angles.PITCH = 0;
	eye.angles.YAW = 0;
	eye.origin.x = 0;
	eye.origin.y = 0;
	eye.origin.z = 48;	// FIXME - origin + view height

	m_hsTalk    = System::GetSoundManager()->RegisterSound("sounds/talk.wav");
	m_hsMessage = System::GetSoundManager()->RegisterSound("sounds/message.wav");

	m_ingame = true;

	System::SetGameState(INGAME);
	SetInputState(true);
	
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
	if(!m_ingame)
		return; // true;

	if(!m_pRender->UnloadWorld())
	{
		ComPrintf("CClient::UnloadWorld - Renderer couldnt unload world\n");
		return; // false;
	}
	
	world_destroy(g_pWorld);
	g_pWorld = 0;

	System::GetSoundManager()->UnregisterAll();
	System::SetGameState(INCONSOLE);

	m_ingame = false;
//	return true;
}

//======================================================================================

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
		m_pHud->HudPrintf(0, 50,0, "%.2f, %.2f, %.2f",eye.origin.x, eye.origin.y, eye.origin.z);

		m_pHud->HudPrintf(0, 70,0, "%.2f : %.2f", 1/(System::g_fcurTime - m_fFrameTime), System::g_fcurTime);
		m_fFrameTime = System::g_fcurTime;

		//Networking
		m_pHud->HudPrintf(0,400,0, "Drop stats %d/%d. Choked %d", m_pNetCl->GetChan().m_dropCount, 
							m_pNetCl->GetChan().m_dropCount + m_pNetCl->GetChan().m_goodCount, 
							m_pNetCl->GetChan().m_numChokes);
		m_pHud->HudPrintf(0,410,0, "In      %d", m_pNetCl->GetChan().m_inMsgId);
		m_pHud->HudPrintf(0,420,0, "In  Ack %d", m_pNetCl->GetChan().m_inAckedMsgId);
		m_pHud->HudPrintf(0,430,0, "Out     %d", m_pNetCl->GetChan().m_outMsgId);
		m_pHud->HudPrintf(0,440,0, "Out Ack %d", m_pNetCl->GetChan().m_lastOutReliableMsgId);

		// FIXME - put this in game dll
		vector_t screenblend;
		if (PointContents(eye.origin) & CONTENTS_SOLID)
			VectorSet(&screenblend, 0.4f, 0.4f, 0.4f);
		else if (PointContents(eye.origin) & CONTENTS_WATER)
			VectorSet(&screenblend, 0, 1, 1);
		else if (PointContents(eye.origin) & CONTENTS_LAVA)
			VectorSet(&screenblend, 1, 0, 0);
		else
			VectorSet(&screenblend, 1, 1, 1);

		m_pRender->Draw(&eye.origin,&eye.angles, &screenblend);
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

/*
======================================
Print a message 
======================================
*/
void CClient::Print(ClMsgType type, const char * msg, ...)
{
	static char textBuffer[1024];
	va_list args;
	va_start(args, msg);
	vsprintf(textBuffer, msg, args);
	va_end(args);

	switch(type)
	{
	case CLMSG_SERVER:
		System::GetSoundManager()->Play(m_hsMessage);
		break;
	case CLMSG_TALK:
		System::GetSoundManager()->Play(m_hsTalk);
		break;
	}
	System::GetConsole()->ComPrint(textBuffer);
}

/*
======================================
Say something
======================================
*/
void CClient::Talk(const char * string)
{
	if(!m_ingame)
		return;

	//parse to right after "say"
	const char * msg = string + 4;
	while(*msg && *msg == ' ')
		msg++;

	if(!*msg || *msg == '\0')
		return;

	ComPrintf("%s: %s\n", m_clname.string, msg);
	System::GetSoundManager()->Play(m_hsTalk);

	//Send this reliably ?
	m_pNetCl->GetSendBuffer().Write(CL_TALK);
	m_pNetCl->GetSendBuffer().Write(msg);
}

/*
======================================
Validate name locally before asking 
the server to update it
======================================
*/
bool CClient::ValidateName(const CParms &parms)
{
	const char * name = parms.StringTok(1);
	if(!name)
	{
		ComPrintf("Name = \"%s\"\n", m_clname.string);
		return false;
	}
	strcpy(userInfo.name,name);

	if(!m_ingame)
		return true;

	m_pNetCl->GetSendBuffer().Write(CL_UPDATEINFO);
	m_pNetCl->GetSendBuffer().Write('n');
	m_pNetCl->GetSendBuffer().Write(name);

/*	m_pNetCl->GetReliableBuffer().Write(CL_UPDATEINFO);
	m_pNetCl->GetReliableBuffer().Write('n');
	m_pNetCl->GetReliableBuffer().Write(name);
*/
	return true;
}

/*
======================================
Validate Rate before updating it 
on the server
======================================
*/
bool CClient::ValidateRate(const CParms &parms)
{
	int rate = parms.IntTok(1);
	if(rate == -1)
	{
		ComPrintf("Rate = \"%d\"\n", m_clrate.ival);
		return false;
	}

	if(rate < 1000 || rate > 30000)
	{
		ComPrintf("Rate is out of range\n");
		return false;
	}
	userInfo.rate = rate;
	
	m_pNetCl->SetRate(rate);

	if(!m_ingame)
		return true;

	CBuffer &buffer = m_pNetCl->GetSendBuffer();
	buffer.Write(CL_UPDATEINFO);
	buffer.Write('r');
	buffer.Write(rate);
	return true;
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
		m_pNetCl->ConnectTo(parms.StringTok(1));
		break;
	case CMD_DISCONNECT:
		m_pNetCl->Disconnect();
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
	if(cvar == dynamic_cast<CVarBase*>(&m_clport))
	{
		int port = parms.IntTok(1);
		if(port < 1024 || port > 32767)
		{
			ComPrintf("Port is out of range, select another\n");
			return false;
		}
		return true;
	}
	else if(cvar == dynamic_cast<CVarBase*>(&m_clrate))
		return ValidateRate(parms);
	else if(cvar == dynamic_cast<CVarBase*>(&m_clname))
		return ValidateName(parms);
	return false;
}

	//Parse and handle a game message
void CClient::HandleGameMsg(CBuffer &buffer)
{
	byte msgId = 0;
	while(msgId != 255)
	{
		msgId= (int)buffer.ReadByte();
		//bad message
		if(msgId == 255)
			break;

//		m_pClient->HandleGameMsg(msgId,buffer);

		switch(msgId)
		{
		case SV_TALK:
			{
				char name[32];
				strcpy(name,buffer.ReadString());
				Print(CLMSG_TALK,"%s: %s\n",name,buffer.ReadString());
//				m_bCanSend = true;
				break;
			}
		case SV_DISCONNECT:
			{
				Print(CLMSG_SERVER,"Server quit\n");
				m_pNetCl->Disconnect(true);
				break;
			}
		case SV_PRINT:	//just a print message
			{
				Print(CLMSG_SERVER,"%s\n",buffer.ReadString());
				break;
			}
		case SV_RECONNECT:
			{
				m_pNetCl->Reconnect();
				break;
			}
		}
	}
}

//Parse and handle spawm parms
void CClient::HandleSpawnMsg(const byte &msgId, CBuffer &buffer)
{
	switch(msgId)
	{
	case SVC_INITCONNECTION:
		{
			char * game = buffer.ReadString();
ComPrintf("CL: Game: %s\n", game);
			char * map = buffer.ReadString();
ComPrintf("CL: Map: %s\n", map);
			if(!LoadWorld(map))
				m_pNetCl->Disconnect();
			break;
		}
	case SVC_MODELLIST:
		break;
	case SVC_SOUNDLIST:
		break;
	case SVC_IMAGELIST:
		break;
	case SVC_BASELINES:
		break;
	case SVC_BEGIN:
		{
		
			break;
		}
	}
}

//Handle disconnect from server
void CClient::HandleDisconnect(bool listenserver)
{
	//Kill server if local
	if(listenserver)
		System::GetConsole()->ExecString("killserver");
	UnloadWorld();
}


void CClient::Spawn(vector_t * origin, vector_t *angles)
{
}