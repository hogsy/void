#include "Net_sock.h"
#include "Sv_main.h"
#include "Net_defs.h"
#include "Com_util.h"

//======================================================================================
//======================================================================================
namespace
{
	enum 
	{
		CMD_MAP = 1,
		CMD_KILLSERVER = 2,
		CMD_STATUS	= 3
	};
}

struct CServer::NetChallenge
{
	NetChallenge()	{ challenge = 0;	time = 0.0f;  }
	CNetAddr	addr;
	int			challenge;
	float		time;
};

using namespace VoidNet;

//======================================================================================
//======================================================================================
/*
==========================================
Constructor/Destructor
==========================================
*/
CServer::CServer() : m_recvBuf(MAX_BUFFER_SIZE),
					 m_sendBuf(MAX_BUFFER_SIZE),
					 m_cPort("sv_port", "20010", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cDedicated("sv_dedicated", "0", CVar::CVAR_BOOL, CVar::CVAR_LATCH),
					 m_cHostname("sv_hostname", "Void Server", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE)
{
	m_pSock = new CNetSocket(&m_recvBuf);
	
	m_challenges = new NetChallenge[MAX_CHALLENGES];

	m_numSignOnBuffers=0;
	for(int i=0;i<MAX_SIGNONBUFFERS;i++)
		m_signOnBuf[i].Create(MAX_DATAGRAM_SIZE);

	m_worldName[0] = 0;
	m_pWorld = 0;
	
	m_active = false;
	m_numClients = 0;
	m_levelNum = 0;

	System::GetConsole()->RegisterCVar(&m_cDedicated);
	System::GetConsole()->RegisterCVar(&m_cHostname);
	System::GetConsole()->RegisterCVar(&m_cGame);
	System::GetConsole()->RegisterCVar(&m_cPort,this);
	System::GetConsole()->RegisterCVar(&m_cMaxClients,this);
	
	System::GetConsole()->RegisterCommand("map",CMD_MAP, this);
	System::GetConsole()->RegisterCommand("killserver",CMD_KILLSERVER, this);
	System::GetConsole()->RegisterCommand("status",CMD_STATUS, this);
}	

CServer::~CServer()
{
	Shutdown();

	delete [] m_challenges;
	delete m_pSock;
}

//======================================================================================
//======================================================================================
/*
==========================================
Initialize the listener socket
gets computer names and addy
==========================================
*/
bool CServer::Init()
{
	if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP, false))
	{
		PrintSockError(WSAGetLastError(),"CServer::Init: Couldnt create socket");
		return false;
	}

	INTERFACE_INFO localAddr[10];  // Assume there will be no more than 10 IP interfaces 
	int numAddrs = 0;
	
	numAddrs = m_pSock->GetInterfaceList((INTERFACE_INFO **)&localAddr,10);
	if(numAddrs == 0)
	{
		ComPrintf("CServer::Init: Unable to get network interfaces\n");
		return false;
	}

	ulong  addrFlags=0;
	char*  pAddrString=0;
	char   boundAddr[24];
	SOCKADDR_IN* pAddrInet=0;
	
	for (int i=0; i<numAddrs; i++) 
	{
		pAddrInet = (SOCKADDR_IN*)&localAddr[i].iiAddress;
		pAddrString = inet_ntoa(pAddrInet->sin_addr);
		
		if (pAddrString)
			ComPrintf("IP: %s ", pAddrString);
		
		addrFlags = localAddr[i].iiFlags;
		if (addrFlags & IFF_UP)
		{
			//if(!(addrFlags & IFF_LOOPBACK))
			if(!(addrFlags & IFF_LOOPBACK) && pAddrString[0] != '0')
			{
				strcpy(boundAddr,pAddrString);
				ComPrintf(": Active\n");
			}
		}
		if (addrFlags & IFF_LOOPBACK)
			ComPrintf(": Loopback\n");
//		if (addrFlags & IFF_POINTTOPOINT)
//			ComPrintf(". this is a point-to-point link");
	}

	if(boundAddr[0] == '\0')
		strcpy(boundAddr,"127.0.0.1");
	strcat(boundAddr,":");
	strcat(boundAddr, m_cPort.string);

	CNetAddr netaddr(boundAddr);

	//Save Local Address
	CNetAddr::SetLocalAddress(boundAddr);
	
	if(!m_pSock->Bind(netaddr))
	{
		ComPrintf("CServer::Init:Unable to bind socket\n");
		return false;
	}
	m_active = true;
	return true;
}

/*
==========================================
Shutdown the server
==========================================
*/
void CServer::Shutdown()
{
	if(!m_active)
		return;

	m_active = false;

	//Send disconnect messages to clients, if active
	if(m_pWorld)
	{
		if(!m_cDedicated.ival)
			System::GetConsole()->ExecString("disconnect");

		for(int i=0;i<m_cMaxClients.ival;i++)
		{
			if(m_clients[i].m_state != CL_SPAWNED) 
				continue;

			//send disconect messages
			m_clients[i].m_netChan.m_reliableBuffer.Reset();
			m_clients[i].m_netChan.m_buffer.Reset();
			m_clients[i].m_netChan.m_buffer += SV_DISCONNECT;
			m_clients[i].m_netChan.m_buffer += "Server quit";
			m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
			m_clients[i].Reset();
		}

		world_destroy(m_pWorld);
		m_pWorld = 0;
		m_worldName[0] = 0;
	}

	m_pSock->Close();
	ComPrintf("CServer::Shutdown OK\n");
}

/*
======================================
Restart
======================================
*/
void CServer::Restart()
{
	if(!m_active)
		return;

	m_active = false;
	
	//Send disconnect messages to clients, if active
//FIXME change to reconnect here
	if(m_pWorld)
	{
		if(!m_cDedicated.ival)
			System::GetConsole()->ExecString("disconnect");

		for(int i=0;i<m_cMaxClients.ival;i++)
		{
			if(m_clients[i].m_state == CL_SPAWNED)
				m_clients[i].m_state = CL_CONNECTED;
		}

		world_destroy(m_pWorld);
		m_pWorld = 0;
		m_worldName[0] = 0;
	}
	ComPrintf("CServer::Restart OK\n");
}



//======================================================================================
//======================================================================================
/*
==========================================
Load the World
==========================================
*/
void CServer::LoadWorld(const char * mapname)
{
	if(!mapname)
	{
		if(m_worldName[0])
			ComPrintf("Playing %s\n",m_worldName);
		else
			ComPrintf("No world loaded\n");
		return;
	}

	//Now load the map
	char worldname[64];
	strcpy(worldname, mapname);

	//Shutdown if currently active
	if(m_active)
		Restart();
	else if(!Init())
		return;

	m_active = true;

	char mappath[COM_MAXPATH];
	strcpy(mappath, szWORLDDIR);
	strcat(mappath, worldname);
	Util::SetDefaultExtension(mappath,".bsp");

	m_pWorld = world_create(mappath);
	if(!m_pWorld)
	{
		ComPrintf("CServer::LoadWorld: couldnt load map %s\n", mappath);
		Shutdown();
		return;
	}

	//Set worldname
	Util::RemoveExtension(m_worldName,COM_MAXPATH, worldname);

	m_levelNum ++;

	//Create Sigon-message. includes static entity baselines
	//=======================
	//all we need is the map name right now
	m_numSignOnBuffers = 1;
	m_signOnBuf[0] += SVC_INITCONNECTION;
	m_signOnBuf[0] += m_cGame.string;
	m_signOnBuf[0] += m_worldName;

	//if its not a dedicated server, then push "connect loopback" into the console
	if(!m_cDedicated.bval)
		System::GetConsole()->ExecString("connect localhost");
}

//======================================================================================
//======================================================================================

/*
======================================
Send a rejection message to the client
======================================
*/
void CServer::SendRejectMsg(const char * reason)
{
	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_REJECT;
	m_sendBuf += reason;
	m_pSock->Send(m_sendBuf);
}

/*
======================================
Handle a status request
======================================
*/
void CServer::HandleStatusReq()
{
	//Header
	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_STATUS;

	//Status info
	m_sendBuf += VOID_PROTOCOL_VERSION;	//Protocol
	m_sendBuf += m_cGame.string;		//Game
	m_sendBuf += m_cHostname.string;	//Hostname
	m_sendBuf += m_worldName;			//Map name
	m_sendBuf += m_numClients;			//cur clients
	m_sendBuf += m_cMaxClients.ival;		//max clients
	
	m_pSock->Send(m_sendBuf);
}

/*
======================================
Handle a connection request
======================================
*/
void CServer::HandleConnectReq()
{
	//Validate Protocol Version
	int protocolVersion = m_recvBuf.ReadInt();
	if(protocolVersion != VOID_PROTOCOL_VERSION)
	{
		char msg[64];
		sprintf(msg,"Bad Protocol version. Need %d, not %d", 
						VOID_PROTOCOL_VERSION, protocolVersion);
		SendRejectMsg(msg);
		return;
	}

	//Validate Challenge
	int challenge = m_recvBuf.ReadInt();
	for(int i=0; i< MAX_CHALLENGES; i++)
	{
		if((m_challenges[i].addr == m_pSock->GetSource()) &&
		   (m_challenges[i].challenge = challenge))
		   break;
	}
	if(i == MAX_CHALLENGES)
	{
		SendRejectMsg("Bad Challenge");
		return;
	}

	//Check if this client is already connected
	for(i=0;i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_netChan.m_addr == m_pSock->GetSource())
		{
			//Is connected, ignore dup connected
			if(m_clients[i].m_state == CL_CONNECTED)
			{
ComPrintf("SV:DupConnect from %s\n", m_pSock->GetSource().ToString());
				return;
			}
			
			//last connection never finished
			m_clients[i].Reset();
ComPrintf("SV:Reconnect from %s\n", m_pSock->GetSource().ToString());
			break;
		}
	}

	//Didn't find any duplicates. now find an empty slot
	if(i == m_cMaxClients.ival)
	{
		for(i=0;i<m_cMaxClients.ival; i++)
		{
			if(m_clients[i].m_state == CL_FREE)
				break;
		}
		
		//Reject if we didnt find a slot
		if(i == m_cMaxClients.ival)
		{
			SendRejectMsg("Server full");
			return;
		}

		//update client counts
		m_numClients ++;
	}

	//We now have a new client slot. create it
	strcpy(m_clients[i].m_name, m_recvBuf.ReadString());
	m_clients[i].m_state = CL_CONNECTED;
	m_clients[i].m_netChan.Setup(m_pSock->GetSource(),&m_recvBuf);
	m_clients[i].m_netChan.SetRate(m_recvBuf.ReadInt());
	m_clients[i].m_netChan.m_outMsgId = 1;	//we send packet1 when we receive

	//This is the last connectionless message
	//Send the client an accept packet
	//now the client needs to call us to get spawn parms etc
	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_ACCEPT;
	m_sendBuf += m_levelNum;
	m_pSock->Send(m_sendBuf);

ComPrintf("SV: %s connected\n",m_clients[i].m_name) ;
}

/*
==========================================
Respond to challenge request
==========================================
*/
void CServer::HandleChallengeReq()
{
	if(m_numClients >= m_cMaxClients.ival)
	{
ComPrintf("SV: Rejecting, server full\n");
		SendRejectMsg("Server is full");
		return;
	}

	int	  oldestchallenge = 0;
	float oldesttime  = 0.0f;

	for(int i=0; i< MAX_CHALLENGES; i++)
	{
		//Found a match, we already got a request from this guy
		if(m_challenges[i].addr == m_pSock->GetSource())
			break;
		
		//Keep a track of what the oldest challenge is
		if(m_challenges[i].time > oldesttime)
		{
			oldestchallenge = i;
			oldesttime = m_challenges[i].time;
		}
	}

	//Didn't find any old challenges from the same addy
	if (i == MAX_CHALLENGES)
	{
		// overwrite the oldest
		i = oldestchallenge;
		m_challenges[i].challenge = (rand() << 16) ^ rand();
		m_challenges[i].addr = m_pSock->GetSource();
		m_challenges[i].time = System::g_fcurTime;
	}

	//Send response packet
	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_CHALLENGE;
	m_sendBuf += m_challenges[i].challenge;
	m_pSock->SendTo(m_sendBuf, m_challenges[i].addr); 
}

/*
==========================================
Process an OOB Server Query message
==========================================
*/
void CServer::ProcessQueryPacket()
{
	char * msg = m_recvBuf.ReadString();
	
	//Ping Request
	if(strcmp(msg, VNET_PING) == 0)
		m_pSock->Send((byte*)VNET_PING, strlen(VNET_PING));
	else if(strcmp(msg, C2S_GETSTATUS) == 0)
		HandleStatusReq();
	else if(strcmp(msg, C2S_CONNECT) == 0)
		HandleConnectReq();
	else if(strcmp(msg, C2S_GETCHALLENGE) == 0)
		HandleChallengeReq();
}


/*
======================================
Parse client message
======================================
*/
void CServer::ParseClientMessage(SVClient &client)
{
	//Check packet id to see what the client send
	int packetId = m_recvBuf.ReadInt();
	switch(packetId)
	{
	case CL_TALK:
		{
			char msg[256];
			strcpy(msg,m_recvBuf.ReadString());
			int len = strlen(msg);
			msg[len] = 0;
			len += strlen(client.m_name);

//ComPrintf("SV:%s : %s\n", client.m_name, msg);
		
			//Add this to all other connected clients outgoing buffers
			for(int i=0; i<m_cMaxClients.ival;i++)
			{
				if(&m_clients[i] == &client)
					continue;
				if(m_clients[i].m_state == CL_SPAWNED)
				{
					m_clients[i].BeginMessage(SV_TALK,len);
					m_clients[i].WriteString(client.m_name);
					m_clients[i].WriteString(msg);
				}
			}
			break;	
		}
	}
}


/*
==========================================
Read any waiting packets
==========================================
*/
void CServer::ReadPackets()
{
	while(m_pSock->Recv())
	{
		//Check if its an OOB message
		if(m_recvBuf.ReadInt() == -1)
		{
			ProcessQueryPacket();
			continue;
		}

		//then it should belong to a client
		for(int i=0; i<m_cMaxClients.ival;i++)
		{
			//match client
			if(m_clients[i].m_netChan.m_addr == m_pSock->GetSource())
			{
				m_recvBuf.BeginRead();
				m_clients[i].m_netChan.BeginRead();

				//client hasn't spawned yet. is asking for parms
				if(m_clients[i].m_state == CL_CONNECTED)
				{
					//Check if client is trying to spawn into current map
					int levelid = m_recvBuf.ReadInt();
					if( levelid != m_levelNum)
					{
//ComPrintf("SV: Client needs to reconnect, bad levelid %d != %d\n", levelid ,m_levelNum);
						SendReconnect(m_clients[i]);
						continue;
					}

					//Find out what spawn message the client is asking for
					int spawnstate = m_recvBuf.ReadInt();
					if(spawnstate == SVC_BEGIN+1)
					{
						m_clients[i].m_state = CL_SPAWNED;
ComPrintf("SV:%s entered the game\n", m_clients[i].m_name);
					}
					else
					{
						m_clients[i].m_spawnState = spawnstate;
//ComPrintf("SV: Client requesting spawn level %d\n", m_clients[i].m_spawnState);
					}
					m_clients[i].m_bSend = true;
				}
				else if(m_clients[i].m_state == CL_SPAWNED)
				{
//					m_clients[i].m_netChan.BeginRead();
					m_clients[i].m_bSend = true;
//ComPrintf("SV: Msg from Spawned client\n");
					ParseClientMessage(m_clients[i]);
				}
				break;
			}
		}
//ComPrintf("SV: unknown packet from %s\n", m_pSock->GetSource().ToString());
	}
}

//======================================================================================
//======================================================================================

/*
======================================

======================================
*/
void CServer::SendReconnect(SVClient &client)
{
	client.m_netChan.m_buffer.Reset();
	client.m_netChan.m_buffer += SV_RECONNECT;
	client.m_netChan.PrepareTransmit();
	m_pSock->SendTo(client.m_netChan.m_sendBuffer, client.m_netChan.m_addr);
	client.m_state = CL_INUSE;
}

/*
======================================
Send requested spawn parms
======================================
*/
void CServer::SendSpawnParms(SVClient &client)
{
	client.m_netChan.m_buffer.Reset();

	//What spawn level does the client want ?
	switch(client.m_spawnState)
	{
	case SVC_INITCONNECTION:
		{
			//Send 1st signon buffer
			client.m_netChan.m_buffer += m_signOnBuf[0];
			break;
		}
	case SVC_MODELLIST:
			client.m_netChan.m_buffer += SVC_MODELLIST;
		break;
	case SVC_SOUNDLIST:
			client.m_netChan.m_buffer += SVC_SOUNDLIST;
		break;
	case SVC_IMAGELIST:
			client.m_netChan.m_buffer += SVC_IMAGELIST;
		break;
	case SVC_BASELINES:
		{
			client.m_netChan.m_buffer += SVC_BASELINES;
			break;
		}
	case SVC_BEGIN:
		{
			//consider client to be spawned now
			client.m_netChan.m_buffer += SVC_BEGIN;
		}
		break;
	}
	client.m_netChan.PrepareTransmit();
//ComPrintf("SV: Sending spawn parms %d\n",client.m_spawnState);
}

/*
======================================
Send updates to clients
======================================
*/
void CServer::WritePackets()
{
	for(int i=0; i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_state == CL_FREE)
			continue;

		//Check timeouts here, and flag resends if we havent got any 
		//response from this client for a while

		//Will fail if we didnt receive a packet from 
		//this client this frame, or if channels chokes
		if(!m_clients[i].ReadyToSend())
			continue;

		//In game clients
		if(m_clients[i].m_state == CL_SPAWNED)
		{
			m_clients[i].m_netChan.PrepareTransmit();
			m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
			//m_clients[i].m_bSend = false;
//ComPrintf("SV:: writing to spawned client\n");
			continue;
		}
		
		//havent spawned yet. need to send spawn info
		if(m_clients[i].m_state == CL_CONNECTED)
		{
			SendSpawnParms(m_clients[i]);
			m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
			m_clients[i].m_bSend = false;
		}
	}
}

//======================================================================================
/*
==========================================
Run a server frame
==========================================
*/
void CServer::RunFrame()
{
	if(m_active== false)
		return;

	//Re-seed current time
	srand((uint)System::g_fcurTime);

	//Get updates
	ReadPackets();

	//Run game

	//write to clients
	WritePackets();
}

//======================================================================================
//======================================================================================

void CServer::PrintServerStatus()
{
}

/*
==========================================
Handle CVars
==========================================
*/
bool CServer::HandleCVar(const CVarBase * cvar, const CParms &parms)
{	return false;
}

/*
==========================================
Handle Commands
==========================================
*/
void CServer::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_MAP:
		LoadWorld(parms.StringTok(1));
		break;
	case CMD_KILLSERVER:
		Shutdown();
	case CMD_STATUS:
		PrintServerStatus();
		break;
	}
}