#include "Net_hdr.h"
#include "Net_sock.h"
#include "Cl_main.h"
#include "Cl_cmds.h"
#include "Com_util.h"

using namespace VoidClient;
using namespace VoidNet;


//======================================================================================
//======================================================================================

world_t	*g_pWorld;
int PointContents(vector_t &v);

/*
======================================
Constructor
======================================
*/
CClient::CClient(I_Renderer * prenderer):
					m_buffer(CNetBuffer::DEFAULT_BUFFER_SIZE),	
					m_clport("cl_port","36667", CVar::CVAR_INT,0),
					m_noclip("cl_noclip","0",   CVar::CVAR_INT,0),
					m_clname("cl_name","Player",CVar::CVAR_STRING,	CVar::CVAR_ARCHIVE),
					m_clrate("cl_rate","0",		CVar::CVAR_INT,		CVar::CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientCmdHandler(this);

	m_pSock = new CNetSocket(&m_buffer);

	m_ingame = false;
	m_clState = CL_INACTIVE;
	m_szLastOOBMsg = 0;
	m_fNextConReq = 0.0f;
	m_bLocalServer = false;
	m_challenge= 0;
	
	m_pHud = 0;
	m_pRender = prenderer;

	g_pWorld = 0;

	System::GetConsole()->RegisterCVar(&m_clport);
	System::GetConsole()->RegisterCVar(&m_clrate);
	System::GetConsole()->RegisterCVar(&m_clname);
	System::GetConsole()->RegisterCVar(&m_noclip);

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
	System::GetConsole()->RegisterCommand("say", CMD_TALK, this);
}

/*
======================================
Destroy the client
======================================
*/
CClient::~CClient()
{
	m_pRender = 0;
	m_pHud = 0;

	m_szLastOOBMsg = 0;

	g_pWorld = 0;

	delete m_pSock;
	delete m_pCmdHandler;
}

/*
=====================================
Load the world for the client to render
=====================================
*/
bool CClient::LoadWorld(world_t *world)
{
//	char configname[128];
//	strcpy(configname,g_gamedir);
//	strcat(configname,"\\void.cfg");
//	g_pCons->ExecConfig(configname);
//	g_pCons->ExecConfig("void.cfg");

	// load the textures
	if(!m_pRender->LoadWorld(world,1))
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

	m_hsTalk = System::GetSoundManager()->RegisterSound("sounds/talk.wav");

	//Spawn ourselves into the world
/*	if(!InitGame())//&g_pWorld->sectors[0]))
	{
		ComPrintf("CVoid::InitGame: couldnt init client\n");
		UnloadWorld();
		return false;
	}
*/	
	g_pWorld = world;
	m_ingame = true;

	SetInputState(true);
	System::SetGameState(INGAME);
	
	ComPrintf("CClient::Load World: OK\n");
	return true;
}


/*
=====================================
Unload the world
=====================================
*/
bool CClient::UnloadWorld()
{
	if(!m_pRender->UnloadWorld())
	{
		ComPrintf("CClient::UnloadWorld - Renderer couldnt unload world\n");
		return false;
	}
	
	if(!m_bLocalServer)
		world_destroy(g_pWorld);

	System::GetSoundManager()->UnregisterAll();

	g_pWorld = 0;
	m_ingame = false;

	SetInputState(false);
	System::SetGameState(INCONSOLE);
	return true;
}



void CClient::SendConnectParms()
{
	//Create Connection less packet
	m_buffer.Reset();
	m_buffer.WriteInt(-1);

	//Write Connection Parms
	m_buffer.WriteString(C2S_CONNECT);			//Header
	m_buffer.WriteInt(VOID_PROTOCOL_VERSION);	//Protocol Version
	m_buffer.WriteInt(m_challenge);				//Challenge Req
//	m_buffer.WriteInt(m_virtualPort);			//Virtual Port
	
	//User Info
	m_buffer.WriteString(m_clname.string);

	m_pSock->Send(m_pSock->m_srcAddr, m_buffer.GetData(), m_buffer.GetSize());
	m_szLastOOBMsg = C2S_CONNECT;
	
	m_fNextConReq = System::g_fcurTime + 2.0f;

	ComPrintf("Sending Connection Parms %s\n", m_svServerAddr);
}

/*
======================================
Process any waiting packets
======================================
*/
void CClient::ReadPackets()
{
	while(m_pSock->Recv())
	{
//		ComPrintf("Client Recv'ed message from %s\n", m_pSock->m_srcAddr.ToString());
		switch(m_clState)
		{
		case CL_INACTIVE:
			break;
		case CL_CONNECTING:
			{
				int packetId = m_buffer.ReadInt();
				if(packetId == -1)
				{
					char * msg = m_buffer.ReadString();
					
					if(!strcmp(msg,S2C_CHALLENGE))
					{
						m_challenge = m_buffer.ReadInt();
						ComPrintf("challenge %d\n", m_challenge);

						//Got challenge, now send connection request
						SendConnectParms();
						return;
					}
					
					if(!strcmp(msg,S2C_ACCEPT))
					{
						char *map = m_buffer.ReadString();
						ComPrintf("map %s\n", map);

						//Now load the map
						char mappath[COM_MAXPATH];
						
						strcpy(mappath,szWORLDDIR);
						strcat(mappath, map);
						Util::SetDefaultExtension(mappath,".bsp");

						g_pWorld = world_create(mappath);
						LoadWorld(g_pWorld);
						return;
					}
				}
				break;
			}
		case CL_SPAWNING:
			break;
		case CL_INGAME:
			break;
		}
	}
}

/*
======================================
RunClient
======================================
*/
void CClient::RunFrame()
{
	static float frametime=0.0f;

	if(m_pSock->ValidSocket())
		ReadPackets();

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
		if(m_pHud)
		{
			m_pHud->HudPrintf(0, 70,0, "%.2f", 1/(System::g_fcurTime - frametime));
			frametime = System::g_fcurTime;
			m_pHud->HudPrintf(0, 50,0, "%.2f, %.2f, %.2f",eye.origin.x, eye.origin.y, eye.origin.z);
		}

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

		m_pRender->DrawFrame(&eye.origin,&eye.angles, &screenblend);
	}
	else
	{
		//draw the console or menues etc
		m_pRender->DrawFrame(0,0,0);
	}
}

/*
=======================================
Disconnect any current connection and 
initiates a new connection request
to the specified address
=======================================
*/
void CClient::SendConnectReq()
{
	CNetAddr netAddr(m_svServerAddr);
	if(!netAddr.IsValid())
	{
		ComPrintf("CClient::ConnectTo: Invalid address\n");
		Disconnect();
		return;
	}

	//Now initiate a connection request

	//Create Connection less packet
	m_buffer.Reset();
	m_buffer.WriteInt(-1);
	m_buffer.WriteString(C2S_GETCHALLENGE);

	m_pSock->Send(netAddr, m_buffer.GetData(), m_buffer.GetSize());
	
	m_szLastOOBMsg = C2S_GETCHALLENGE;
	m_fNextConReq = System::g_fcurTime + 2.0f;

	ComPrintf("Requesting Challenge : %s\n", m_svServerAddr);
}

void CClient::ConnectTo(const char * ipaddr)
{
	if(!ipaddr)
	{
		ComPrintf("usage - connect ipaddr(:port)\n");
		return;
	}

	if(m_ingame)
		Disconnect();

	if(!m_pSock->ValidSocket())
	{
		if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
		{
			PrintSockError(WSAGetLastError(),"CClient::Init: Couldnt create socket");
			return;
		}
	}

	//Send a connection request
	CNetAddr netAddr(ipaddr);
	if(!netAddr.IsValid())
	{
		ComPrintf("CClient::ConnectTo: Invalid address\n");
		return;
	}
	strcpy(m_svServerAddr, netAddr.ToString());

	CNetAddr localAddr("localhost");
	if(localAddr == netAddr)
		m_bLocalServer = true;

	//Now initiate a connection request
	m_clState = CL_CONNECTING;
	SendConnectReq();
}


/*
=====================================
Disconnect if connected to a server
=====================================
*/
void CClient::Disconnect()
{
	if(m_ingame)
	{
		UnloadWorld();

		if(m_bLocalServer)
			System::GetConsole()->ExecString("killserver");
	}

	m_svServerAddr[0] = 0;
	m_clState = CL_INACTIVE;
	m_fNextConReq = 0.0f;
	m_bLocalServer = false;
}

/*
=====================================
Talk message sent to server if 
we are connected
=====================================
*/
void CClient::Talk(const char *msg)
{
#if 0
	if((m_ingame==true) &&  (argc > 1))
	{
		char message[80];
		char *p = message;

		//reconstruct message
		for(int x=0;x<=argc;x++)
		{
			char *c = argv[x];
			
			while(*c && *c!='\0')
			{
				*p=*c;
				c++;
				p++;
			}
			
			//no more strings, break
			if((x+1)>=argc)
			{
				*p = '\0';
				break;
			}
			//add a space
			*p = ' ';
			p++;
		}
/*
		g_pClient->m_sendBuf.WriteLong(g_pClient->m_sendseq);
		g_pClient->m_sendBuf.WriteByte(CL_STRING);
		g_pClient->m_sendBuf.WriteString(message);
		g_pClient->m_sendseq++;
		g_pClient->m_sock.bsend=true;
*/
//		ComPrintf("%s:%s\n",g_pClient->m_clname->string,message);
		return;
	}
#endif
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
		ConnectTo(parms.StringTok(1));
		break;
	case CMD_DISCONNECT:
		Disconnect();
	case CMD_TALK:
		Talk(parms.StringTok(1));
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
	return false;
}

void CClient::Spawn(vector_t * origin, vector_t *angles)
{
}
