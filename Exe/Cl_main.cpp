#include "Cl_main.h"
#include "Cl_cmds.h"

using namespace VoidClient;


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
					m_clport("cl_port","36667", CVar::CVAR_INT,		CVar::CVAR_ARCHIVE),
					m_clname("cl_name","Player",CVar::CVAR_STRING,	CVar::CVAR_ARCHIVE),
					m_clrate("cl_rate","0",		CVar::CVAR_INT,		CVar::CVAR_ARCHIVE),
					m_noclip("cl_noclip","0",   CVar::CVAR_INT,		CVar::CVAR_ARCHIVE)
{

	m_pCmdHandler = new CClientCmdHandler(this);

	m_connected=false;
	m_ingame = false;
	m_active = false;

	m_campath = -1;
	m_acceleration = 400.0f;
	m_maxvelocity =  200.0f;

	m_pHud = 0;
	m_pRender = prenderer;

	g_pWorld = 0;

	// FIXME - should be actual player size
	VectorSet(&eye.mins, -10, -10, -40);
	VectorSet(&eye.maxs,  10,  10,  10);
	VectorSet(&desired_movement, 0, 0, 0);

	// FIXME - should be taken care of at spawn
	eye.angles.ROLL = 0;
	eye.angles.PITCH = 0;
	eye.angles.YAW = 0;
	eye.origin.x = 0;
	eye.origin.y = 0;
	eye.origin.z = 48;


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

	g_pWorld = 0;
	
	delete m_pCmdHandler;
}

/*
=====================================
Load the world for the client to render
=====================================
*/
bool CClient::LoadWorld(world_t *world)
{
	// load the textures
	if(!m_pRender->LoadWorld(world,1))
	{
		ComPrintf("CClient::LoadWorld: Renderer couldnt load world\n");
		return false;
	}

//	char configname[128];
//	strcpy(configname,g_gamedir);
//	strcat(configname,"\\void.cfg");
//	g_pCons->ExecConfig(configname);
//	g_pCons->ExecConfig("void.cfg");

	m_pHud = m_pRender->GetHud();
	if(!m_pHud) //m_pRender->GetHud(&m_pHud))
	{
		ComPrintf("CClient::Init:: Couldnt get hud interface from renderer\n");
		return false;
	}

// FIXME - should be taken care of at spawn
// FIXME - should be actual player size

	VectorSet(&eye.mins, -10, -10, -40);
	VectorSet(&eye.maxs,  10,  10,  10);


// FIXME - should be taken care of at spawn
	eye.angles.ROLL = 0;
	eye.angles.PITCH = 0;
	eye.angles.YAW = 0;
	eye.origin.x = 0;
	eye.origin.y = 0;
	eye.origin.z = 48;	// FIXME - origin + view height

	//Spawn ourselves into the world
/*	if(!InitGame())//&g_pWorld->sectors[0]))
	{
		ComPrintf("CVoid::InitGame: couldnt init client\n");
		UnloadWorld();
		return false;
	}
*/	
	g_pWorld = world;

	m_connected = true;
	m_ingame = true;
	SetInputState(true);
	
	ComPrintf("CClient::Load World: OK\n");

	System::SetGameState(INGAME);
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

	g_pWorld = 0;

	m_ingame = false;
	m_connected = false;

	SetInputState(false);
	System::SetGameState(INCONSOLE);
	return true;
}

/*
======================================
RunClient
======================================
*/
void CClient::RunFrame()
{
	if(m_ingame)
	{
		m_pCmdHandler->RunCommands();

		if (!((desired_movement.x==0) && (desired_movement.y==0) && (desired_movement.z==0)) || (m_campath != -1))
		{
			VectorNormalize(&desired_movement);
			Move(&desired_movement, System::g_fframeTime * m_maxvelocity);
			desired_movement.x = 0;
			desired_movement.y = 0;
			desired_movement.z = 0;
		}

		//Print Stats
		if(m_pHud)
		{
		m_pHud->HudPrintf(0, 70,0, "%.2f", 1 / System::g_fframeTime);
		m_pHud->HudPrintf(0, 50,0, "%.2f, %.2f, %.2f",eye.origin.x, 
											    eye.origin.y, 
												eye.origin.z);
		}

		// FIXME - put this in game dll
		vector_t screenblend;
		if (PointContents(eye.origin) & CONTENTS_SOLID)
		{
			VectorSet(&screenblend, 0.4f, 0.4f, 0.4f);
		}
		else if (PointContents(eye.origin) & CONTENTS_WATER)
		{
			VectorSet(&screenblend, 0, 1, 1);
		}
		else if (PointContents(eye.origin) & CONTENTS_LAVA)
		{
			VectorSet(&screenblend, 1, 0, 0);
		}
		else
		{
			VectorSet(&screenblend, 1, 1, 1);
		}

		m_pRender->DrawFrame(&eye.origin,&eye.angles, &screenblend);
	}
	else
	{
		//draw the console or menues etc
		m_pRender->DrawFrame(0,0,0);
	}


#ifndef __VOIDALPHA
	m_sock.Run();
	
	//in the process of connecting and received something
	if(m_sock.bcansend)
	{
		if(!m_connected)
		{
			if(!m_sock.brecv)
				return;

			char * s = m_recvBuf.ReadString(INFOSTRING_DELIM);
			
			if(!s)
			{
				m_sock.brecv= false;
				m_recvBuf.Reset();
				return;
			}

			//received challenge number, use it to send a connection request
			if(!strncmp(s,"challenge",9))
			{
				int challenge = m_recvBuf.ReadLong();

				//Sent Connection Request here
				m_sendBuf.Reset();
				m_sendBuf.WriteString("connect\\");
				m_sendBuf.WriteLong(challenge);
				m_sendBuf.WriteString("\\protocol\\");
				m_sendBuf.WriteShort(PROTOCOL_VERSION);
				m_sock.bsend=true;
			}
			
			//connection has been accepted on the listener port
			//try to connect at the allocated port now, then the game will start
			else if(!strncmp(s,"accept",6))
			{
				int port = m_recvBuf.ReadLong();

				//connect to this port now
				m_sock.Close();
				
				if(!InitNet())
				{
					ComPrintf("Client Couldnt reinit net\n");
					return;
				}

				if(m_sock.Connect(m_svipaddr,port)) //port
				{
					m_svport = port;

					m_sendBuf.Reset();
					
					//Client info
					m_sendBuf.WriteString("n\\");
					m_sendBuf.WriteString(m_clname->string);
//					m_sendBuf.WriteString(m_clname.str.c_str());
					m_sendBuf.WriteString("\\r\\");
					m_sendBuf.WriteLong((int)m_clrate->value);
//					m_sendBuf.WriteLong((int)m_clrate.value);
					
					m_sock.bsend = true;
					ComPrintf("Client trying to connect to %s:%d\n",m_svipaddr,port);
					return;
				}
				ComPrintf("CClient::Unable to connect to  %s:%d\n",m_svipaddr,port);
				m_sendBuf.Reset();
				m_recvBuf.Reset();
				m_sock.Close();
				return;
			}

			//connection was rejected, print out why and close the socket
			else if(!strncmp(s,"reject",6))
			{
				char *reason = m_recvBuf.ReadString(INFOSTRING_DELIM);
				
				if(reason)
					ComPrintf("CClient::Connection refused:%s\n", reason);
				else
					ComPrintf("CClient::Connection refused:Unknown reason\n");
				m_sendBuf.Reset();
				m_recvBuf.Reset();
				m_sock.Close();
				return;
			}

			//connected to the allocated port and the remote side
			//is about to start sending reliable server info
			else if(!strncmp(s,"svinfo",6) &&
					(m_sock.m_state == CSocket::SOCK_CONNECTING))
			{
				//parse server info. map etc
				char mapname[64];
				if(BufParseKey((char *)m_recvBuf.data,"map",mapname,64))
				{
					if(g_pVoid->LoadWorld(mapname) &&
					   LoadWorld(g_pWorld))
					{	
						m_sock.AcceptConnection();
						m_connected = true;
						
						ComPrintf("CClient::Accepted Connection on port %d\n", m_svport);
						
						m_sock.brecv= false;
						m_recvBuf.Reset();
						return;
					}
					ComPrintf("CClient::Couldnt load map %s\n", mapname);
					Disconnect();
					return;
				}
				ComPrintf("CClient::Couldnt read mapname\n");
				Disconnect();
			}
			return;
		}
		
		//we are connected now, get spawning info
		//this should be fairly reliable, packets should be numbered and all
		if(!m_ingame)	//&& m_sock.brecv)
		{
			m_ingame = true;
			return;
		}


		//received something
		if(m_sock.brecv)
		{
			int i=m_recvBuf.ReadLong();
			char b= m_recvBuf.ReadByte();
			
			switch(b)
			{
			case SV_NOP:
				{
					m_recvBuf.Reset();
					break;
				}
			case SV_PRINT:
				{
					char * buf = m_recvBuf.ReadString();
					if(!buf) { return;}
					ComPrintf("%s",buf);
//					g_pSound->Play("talk.wav",0);
					m_recvBuf.Reset();
					break;
				}
			}
		}

		//we are in the game. normal packet parsing
		if((g_fcurTime - m_sock.lastsendtime) > 1.0)
		{
			m_sendBuf.Reset();
					
			//Client info
			m_sendBuf.WriteLong(m_sendseq);
			m_sendBuf.WriteByte(CL_NOP);
//			m_sendBuf.WriteString("nop");
			m_sock.bsend = true;
			m_sendseq ++;
		}
	}
#endif

}

/*
=======================================
Disconnect all current connections
and initiate a new connection
to the specified address


assumes its a vaild IP address
=======================================
*/

bool CClient::ConnectTo(char *ipaddr, int port)
{
#ifndef __VOIDALPHA
	if(m_ingame)
	{
		Disconnect();
	}

	if(!m_active)
	{
		ComPrintf("CClient::Connect To- client is inactive\n");
		InitNet();
	}

	if(m_sock.bcansend)
		m_sock.Close();

	char ip[16];

	if(!strcmp(ipaddr,"loopback"))
		strcpy(ip,g_ipaddr);
	else
		strcpy(ip,ipaddr);

	if(m_sock.Connect(ip,port))
	{
		m_sendBuf.WriteString("getchallenge");
		m_sock.bsend = true;
		strcpy(m_svipaddr,ip);
		ComPrintf("CClient::connecting to %s:%d\n",ip,port);
		return true;
	}
	ComPrintf("CClient::Unable to connect to  %s:%d\n",ip,port);
	return false;
#endif
	return true;
}


/*
=====================================
Disconnect if connected to a server
=====================================
*/

bool CClient::Disconnect()
{
	if(m_ingame)
		UnloadWorld();
	System::SetGameState(INCONSOLE);
//	g_pWorld = 0;
	return true;
}


void CClient::Spawn(vector_t	*origin, vector_t *angles)
{
}



/*
=====================================
Talk message sent to server if 
we are connected
=====================================
*/

void Talk(int argc,char **argv)
{
#ifndef __VOIDALPHA
	if((g_pClient->m_ingame==true) && 
	   (g_pClient->m_connected==true) &&
	   (argc > 1))
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

		g_pClient->m_sendBuf.WriteLong(g_pClient->m_sendseq);
		g_pClient->m_sendBuf.WriteByte(CL_STRING);
		g_pClient->m_sendBuf.WriteString(message);
		g_pClient->m_sendseq++;
		g_pClient->m_sock.bsend=true;
//		ComPrintf("%s:%s\n",g_pClient->m_clname->string,message);
		return;
	}
#endif
	ComPrintf("Not connected to the server\n");
}

//======================================================================================
//======================================================================================

/*
==========================================

==========================================
*/

void CClient::SetInputState(bool on)
{	m_pCmdHandler->SetListenerState(on);
}


/*
==========================================

==========================================
*/
void CClient::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
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
		m_pCmdHandler->BindFuncToKey(numArgs,szArgs);
		break;
	case CMD_BINDLIST:
		m_pCmdHandler->BindList();
		break;
	case CMD_UNBIND:
		m_pCmdHandler->Unbind(numArgs,szArgs);
		break;
	case CMD_UNBINDALL:
		m_pCmdHandler->Unbindall();
		break;
	case CMD_CAM:
		CamPath(numArgs,szArgs);
		break;
	}
}

/*
======================================
Writes the current Bind table to a config file
called from the Console Shutdown func
======================================
*/

void CClient::WriteBindTable(FILE *fp)
{	m_pCmdHandler->WriteBindTable(fp);
}


bool CClient::HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)
{
	return false;
}