#include "Net_sock.h"
#include "Sv_main.h"
#include "Net_defs.h"
#include "Com_util.h"

namespace
{
	enum 
	{
		CMD_MAP = 1,
		CMD_KILLSERVER = 2
	};
	
	const int  MAX_CHALLENGES = 512;

	const char szLOOPBACKADDR[] = "loopback";
	const char szWORLDDIR[]     = "Worlds/";
}

struct CServer::NetChallenge
{
	NetChallenge()	{ challenge = 0;	time = 0.0f;  }

	VoidNet::CNetAddr	addr;
	int		challenge;
	float	time;
};

using namespace VoidNet;

//======================================================================================
//======================================================================================
/*
==========================================
Constructor/Destructor
==========================================
*/
CServer::CServer(CClient * client) :
					 m_recvBuf(CNetBuffer::DEFAULT_BUFFER_SIZE),
					 m_sendBuf(CNetBuffer::DEFAULT_BUFFER_SIZE),
					 m_cPort("sv_port", "20010", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cDedicated("sv_dedicated", "0", CVar::CVAR_BOOL, CVar::CVAR_LATCH),
					 m_cHostname("sv_hostname", "Skidz", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE)
{
	memset(m_boundAddr,0,sizeof(m_boundAddr));

	m_challenges = new NetChallenge[MAX_CHALLENGES];

	m_pWorld = 0;

	m_pClient = client;
	
	m_pSock = new CNetSocket(&m_recvBuf);

	m_active = false;

	System::GetConsole()->RegisterCVar(&m_cDedicated);
	System::GetConsole()->RegisterCVar(&m_cHostname);
	System::GetConsole()->RegisterCVar(&m_cGame);
	
	System::GetConsole()->RegisterCVar(&m_cPort,this);
	System::GetConsole()->RegisterCVar(&m_cMaxClients,this);
	
	System::GetConsole()->RegisterCommand("map",CMD_MAP, this);
	System::GetConsole()->RegisterCommand("killserver",CMD_KILLSERVER, this);
}	

CServer::~CServer()
{
	m_pClient = 0;

	if(m_pWorld)
		world_destroy(m_pWorld);

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
	if(!m_pSock->Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
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
	SOCKADDR_IN* pAddrInet=0;
	
	for (int i=0; i<numAddrs; i++) 
	{
		pAddrInet = (SOCKADDR_IN*)&localAddr[i].iiAddress;
		pAddrString = inet_ntoa(pAddrInet->sin_addr);
		
		if (pAddrString)
			ComPrintf("IP: %s  ", pAddrString);
		
		addrFlags = localAddr[i].iiFlags;
		if (addrFlags & IFF_UP)
		{
			if(!(addrFlags & IFF_LOOPBACK))
			{
				strcpy(m_boundAddr,pAddrString);
				ComPrintf("  This interface is up");
			}
		}
		if (addrFlags & IFF_LOOPBACK)
			ComPrintf("  This is the loopback interface");
//		if (addrFlags & IFF_POINTTOPOINT)
//			ComPrintf(". this is a point-to-point link");
	}

	if(m_boundAddr[0] == '\0')
		strcpy(m_boundAddr,szLOOPBACKADDR);
	strcat(m_boundAddr,":");
	strcat(m_boundAddr, m_cPort.string);

	CNetAddr netaddr;
	netaddr= (const char*)m_boundAddr;
	//netaddr= "127.0.0.1:20010";

	if(!m_pSock->Bind(netaddr,false))
	{
		ComPrintf("CServer::Init:Unable to bind socket\n");
		memset(m_boundAddr,0,sizeof(m_boundAddr));
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

	//Send disconnect messages to clients, if active
	if(m_pWorld)
	{
		//HACK calling client directly right now
		m_pClient->UnloadWorld();
		world_destroy(m_pWorld);
		m_pWorld = 0;
	}

	m_pSock->Close();
	m_active = false;

	ComPrintf("CServer::Shutdown OK\n");
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
	//Shutdown if currently active
	if(m_active)
		Shutdown();
	
	if(!Init())
		return;

	//Now load the map
	char mappath[COM_MAXPATH];
	
	strcpy(mappath, szWORLDDIR);
	strcat(mappath, mapname);
	Util::SetDefaultExtension(mappath,".bsp");
	
	m_pWorld = world_create(mappath);
	if(!m_pWorld)
	{
		ComPrintf("CServer::LoadWorld: couldnt load map %s\n", mappath);
		Shutdown();
		return;
	}
	
	//if its not a dedicated server, then push "connect loopback" into the console
	if(!m_cDedicated.bval)
	{
		//HACK
		if(!m_pClient->LoadWorld(m_pWorld))
		{
			Shutdown();
			return;
		}
	}
}

//======================================================================================
//======================================================================================

void CServer::HandleStatusREQ()
{
}

void CServer::HandleConnectREQ()
{
}

/*
==========================================
Respond to challenge request
==========================================
*/
void CServer::HandleChallengeREQ()
{
	int	  oldestchallenge = 0;
	float oldesttime  = 0.0f;

	for(int i=0; i< MAX_CHALLENGES; i++)
	{
		//Found a match, we already got a request from this guy
		if(m_challenges[i].addr == m_pSock->m_srcAddr)
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
		m_challenges[i].addr = m_pSock->m_srcAddr;
		m_challenges[i].time = System::g_fcurTime;
	}

	m_sendBuf.Reset();
	m_sendBuf.WriteInt(-1);
	m_sendBuf.WriteString(" challenge ");
	m_sendBuf.WriteInt(m_challenges[i].challenge);

	//Send response packet
	m_pSock->Send(m_challenges[i].addr, m_sendBuf.GetData(), m_sendBuf.GetSize());
}


/*
==========================================
Process an OOB Server Query message
==========================================
*/
void CServer::ProcessQueryPacket()
{
	ComPrintf("Query from %s:%d\n", m_pSock->m_srcAddr.ToString());

	char * msg = m_recvBuf.ReadString();
	
	//Ping Request
	if(strcmp(msg, C2S_PING) == 0)
	{
		m_pSock->Send(m_pSock->m_srcAddr, (byte*)C2S_PING, strlen(C2S_PING));
	}
	else if(strcmp(msg, C2S_STATUS) == 0)
	{

	}
	else if(strcmp(msg, C2S_CONNECT) == 0)
	{
	}
	else if(strcmp(msg, C2S_GETCHALLENGE) == 0)
	{
	}
}


/*
==========================================
Read any waiting packets
==========================================
*/
void CServer::ReadPackets()
{
	int packetId = 0;
	
	while(m_pSock->Recv())
	{
		ComPrintf("Got Data\n");

		packetId = m_recvBuf.ReadInt();

		if(packetId == -1)
		{
			ProcessQueryPacket();
			continue;
		}
	}
}

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

	//Process messages
	ReadPackets();
}

/*
==========================================
Handle CVars
==========================================
*/
bool CServer::HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)
{
	return false;
}

/*
==========================================
Handle Commands
==========================================
*/
void CServer::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CMD_MAP:
		LoadWorld(szArgs[1]);
		break;
	case CMD_KILLSERVER:
		Shutdown();
		break;
	}
}













//======================================================================================
//======================================================================================
#if 0
#include "Sv_main.h"
#include "Sv_client.h"
CNetClients g_netclients;
char		g_gamedir[MAXPATH];
/*
========================================
Constructor
========================================
*/
CServer::CServer() 
{
	m_pWorld=0;
	m_pNetworkEvents = new WSANETWORKEVENTS;
	m_protocolversion = PROTOCOL_VERSION;
	m_active = false;

	g_pCons->RegisterCVar(&m_port,"sv_port","36666",CVAR_INT,CVAR_ARCHIVE|CVAR_LATCH);	
	g_pCons->RegisterCVar(&m_hostname,"sv_hostname","Skidz",CVAR_STRING,CVAR_ARCHIVE);
	g_pCons->RegisterCVar(&m_dedicated,"sv_dedicated","0",CVAR_INT,CVAR_ARCHIVE|CVAR_LATCH);
	g_pCons->RegisterCVar(&m_maxclients,"sv_maxclients","2",CVAR_INT,CVAR_ARCHIVE|CVAR_LATCH);
	g_pCons->RegisterCVar(&m_game,"game","Game",CVAR_STRING,CVAR_LATCH);		

	strcpy(g_gamedir,m_game->string);
}


/*
========================================
Destructor
========================================
*/
CServer::~CServer()
{
	delete m_pNetworkEvents;
}

/*
=======================================
Private Function to Initialize Winsock
=======================================
*/

bool CServer::Init()
{
	g_pCons->RegisterCFunc("sv_status",&SV_Status);
	strcpy(g_gamedir,m_game->string);
	g_pCons->dprintf("Server::Init: Gamedir:%s\n",g_gamedir);

	//Seed random number generator for challenge strings
	srand((unsigned)time(NULL));

	g_pCons->dprintf("CServer::Init:%s\n:%s\n",g_computerName,g_ipaddr);
	return true;
}



/*
=======================================
Init Server
Map changes will NOT call this
Game changes will
=======================================
*/
//bool CServer::InitGame(world_t *world)
bool CServer::InitGame(char *mapname)
{
	char worldname[128];
	
	Util::DefaultExtension(mapname,".bsp");
//	sprintf(worldname,"%s\\%s\\worlds\\%s",g_exedir,g_gamedir,mapname);
//	sprintf(worldname,"%s\\worlds\\%s",g_exedir,mapname);

	if(m_pWorld != 0)
		world_destroy(m_pWorld);
	m_pWorld = 0;

	m_pWorld = world_create(worldname);
	
	if(!m_pWorld)
	{
		g_pCons->dprintf("CServer::InitGame: couldnt load %s\n",mapname);
		return false;
	}

	Util::RemoveExtension(mapname,m_mapname);

	if(m_socket != INVALID_SOCKET)
		closesocket(m_socket);

	//if no address is specified, or local host
//	m_addr.sin_addr.s_addr = inet_addr(m_ipaddr);
	m_addr.sin_addr.s_addr = htonl(INADDR_ANY);       
	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons((int)m_port->value);

	m_socket = socket(AF_INET,SOCK_DGRAM,0);
	if (m_socket == INVALID_SOCKET)
	{
		g_pCons->dprintf("CServer::Init:ERROR:Couldnt create socket\n");
		closesocket(m_socket);
		return false;
	}

	// bind the socket to the internet address 
	if (bind(m_socket, (struct sockaddr *)&m_addr, sizeof(m_addr)) == SOCKET_ERROR) 
	{
		g_pCons->dprintf("CServer::Init:ERROR:Couldnt bind socket\n");
		closesocket(m_socket);
		return false;
	}

	//Create Event
	m_event = WSACreateEvent();
	if(m_event ==WSA_INVALID_EVENT)
	{
		g_pCons->dprintf("CServer::Init:Error creating event\n");
		closesocket(m_socket);
		return false;
	}

	if(WSAEventSelect(m_socket,m_event,FD_READ|FD_WRITE)==SOCKET_ERROR)
	{
		g_pCons->dprintf("CServer::Init:Error selecting Event\n");
		closesocket(m_socket);
		return false;
	}

	//init clients
	g_netclients.Init(m_addr,(int)m_maxclients->value, (int)m_port->value);

//  m_world = world;
	m_active = true;

	g_pCons->dprintf("CServer::Init:Server on:%s:%d\n",g_ipaddr,(int)m_port->value);
	return true;
}

/*
=======================================
Shuts down the server
=======================================
*/
bool CServer::Shutdown()
{
	if(m_pWorld)
		world_destroy(m_pWorld);
	m_pWorld = 0;


	//network shutdown
	WSAEventSelect(m_socket,m_event,0);
	closesocket(m_socket);
	memset(&m_addr, 0, sizeof(struct sockaddr_in));

	m_active = false;
	
	m_sockBuf.Reset();
	m_datagram.Reset();
	m_reliable_datagram.Reset();

	g_netclients.Shutdown();

	return true;
}



/*
=======================================
Send data in Listener buffer 
to specified address
=======================================
*/

bool CServer::SendNBuffer(SOCKADDR_IN * addr)
{
	int nSent;
	
	if (m_socket == INVALID_SOCKET)
		return false;

	nSent = sendto(m_socket, (char *)m_sockBuf.data,m_sockBuf.cursize,0,
				  (LPSOCKADDR)addr, sizeof(struct sockaddr_in));

	if(nSent == SOCKET_ERROR)
	{
		g_pCons->dprintf("CServer:SendData:error sending %s to %s\n",
								(char *)m_sockBuf.data,inet_ntoa(addr->sin_addr));
		PrintSockError();
		return false;
	}
	g_pCons->dprintf("CServer:SendData:%s\nto:%s\nsize:%d/%d\n",
								(char *)m_sockBuf.data,inet_ntoa(addr->sin_addr),nSent,m_sockBuf.cursize);
	m_sockBuf.Reset();
	return true;
}


/*
=====================================
Check for new connections
the listener socket will respond to the following stings

getinfo				//brief info	-map,num players and names
getstatus			//detailed info	-frags,time,pings
getchallenge		//get a randomized challenge number, 
					//which is then passed when trying to connect
connect
=====================================
*/

void CServer::CheckNewConnections()
{
	if(m_socket != INVALID_SOCKET)
	{
		int rlen = sizeof(m_raddr);
		int i = recvfrom(m_socket, (char *)m_sockBuf.data,MAX_DATAGRAM,0,(LPSOCKADDR)&m_raddr, &rlen); //MSG_PEEK

		if(i==SOCKET_ERROR)
		{
			m_sockBuf.Reset();
			return;
		}

		m_sockBuf.cursize = i;
		char *buf = m_sockBuf.ReadString(INFOSTRING_DELIM);
			
		g_pCons->dprintf("CServer::Recv'ed (%s) from (%s)\n", buf,inet_ntoa(m_raddr.sin_addr));
			
		//just asking for game info
		if(strncmp(buf,"getinfo",7)==0)
		{
			m_sockBuf.Reset();
			WriteInfoMsg(&m_sockBuf);
			SendNBuffer(&m_raddr);
			return;
		}

		//asking for complete server status
		if(strncmp(buf,"getstatus",9)==0)
		{
			m_sockBuf.Reset();
			WriteStatusMsg(&m_sockBuf);
			SendNBuffer(&m_raddr);
			return;
		}	

		
		//update the connection queue
		for(i=0;i<MAX_CONNECTQ;i++)
		{
			if(m_connectq[i].inuse)	
			{
				//assume its timed out
				if((g_fcurTime - m_connectq[i].lastmsg) > CON_TIMEOUT)
				{
					g_pCons->dprintf("CServer::CheckNewConnections:%s timed out\n",m_connectq[i].ipaddr);
					m_connectq[i].inuse = false;
					return;
				}
			}
		}
			
		char ip[16];
		sprintf(ip,"%s",inet_ntoa(m_raddr.sin_addr));
			
		//asking for a challenge number
		if(strncmp(buf,"getchallenge",12)==0)
		{
			int slot=0;
				
			//see if we have any free connections
			for(i=0;i<MAX_CONNECTQ;i++)
			{
				//already in the process of connecting
				if(!strcmp(m_connectq[i].ipaddr,ip) && m_connectq[i].inuse)	
				{
					g_pCons->dprintf("CServer::CheckNewConnections:multiple challenge req from %s\n",ip);
					return;
				}
				else if(!m_connectq[i].inuse && !slot)
				{	slot = i;
				}
			}

			//send out a challege number if we found an empty slot for the challege request
			if(slot)
			{
				m_connectq[slot].inuse = true;
				strcpy(m_connectq[slot].ipaddr,ip);
				m_connectq[slot].lastmsg = g_fcurTime;
				m_connectq[slot].challenge= rand();
					
				//get random challenge number and send it
				m_sockBuf.Reset();
				m_sockBuf.WriteString("challenge\\");
				m_sockBuf.WriteLong(m_connectq[slot].challenge);
				SendNBuffer(&m_raddr);
			}
			return;
		}
			
		if(strncmp(buf,"connect",7)==0)
		{
			//send reject packet if server is full
			if(g_netclients.m_curclients >= g_netclients.m_maxclients)
			{
				SendRejectPacket(&m_raddr,"Server is full");
				return;
			}

			int challenge = m_sockBuf.ReadLong();
			int	clport = 0;
			
			buf = m_sockBuf.ReadString(INFOSTRING_DELIM);
			if(buf && strncmp(buf,"protocol",8))
			{
				SendRejectPacket(&m_raddr,"No protocol version passed");
				return;
			}

			int ver = m_sockBuf.ReadShort();
			if(ver != PROTOCOL_VERSION)
			{
				char buf[32];
				sprintf(buf,"Server is running protocol %d\n",PROTOCOL_VERSION);
				SendRejectPacket(&m_raddr,buf);
				return;
			}

			buf = m_sockBuf.ReadString(INFOSTRING_DELIM);
			if(buf && strncmp(buf,"port",4))
			{
				clport = m_sockBuf.ReadLong();
			}

			if(clport <= 0)
				clport = 36667;

			//compare ip address and challenge number
			//accept and create new client if they match
			for(i=0;i<MAX_CONNECTQ;i++)
			{
				//in the process of connecting - ready to be accepted
				if(!strcmp(m_connectq[i].ipaddr,ip) && m_connectq[i].inuse)	
				{
					if(challenge == m_connectq[i].challenge)
					{
						//Create a new client object in the server at an unused port
						//and send over the port info to the connecting client, 
						//all further communication for this client will be handled elsewhere

						CSVClient *cl = g_netclients.GetFreeClient();

						if(cl && cl->SV_Connect(m_raddr,clport,g_netclients.m_curclients))
						{
							m_sockBuf.Reset();
							m_sockBuf.WriteString("accept\\");
							m_sockBuf.WriteLong(cl->m_port);
							SendNBuffer(&m_raddr);
							return;
						}
					}
				}
			}
			//send rejection message, bad challenge
			SendRejectPacket(&m_raddr,"Bad Challenge returned");
		}
	}
}



/*
=======================================
-Update the Game
-Run Clients
-Send out updates
=======================================
*/

void CServer::RunFrame()
{
	if(m_active)
	{
		int wsaerr;
		wsaerr = WSAEnumNetworkEvents(m_socket,m_event,m_pNetworkEvents);

		if(wsaerr == SOCKET_ERROR)
		{
			g_pCons->dprintf("CServer::RunFrame: WSAEnumNetevetns error\n");
			PrintSockError();
			return;
		}

		if(m_pNetworkEvents->lNetworkEvents & FD_READ)
		{
			//have a client that wants to connect
			CheckNewConnections();
		}

		g_netclients.RunClients();

		//send broadcast info

	}
}

/*
=====================================
send a connection rejection packet to
this address with this reason
=====================================
*/

bool CServer::SendRejectPacket(SOCKADDR_IN *addr, char *reason)
{
	m_sockBuf.Reset();
	m_sockBuf.WriteString("reject//");
	m_sockBuf.WriteString(reason);
	
	return SendNBuffer(addr);
}



/*
=====================================
send a connection rejection packet to
this address with this reason
=====================================
*/

bool CServer::SendAcceptPacket(SOCKADDR_IN *addr, int port)
{
	m_sockBuf.Reset();
	m_sockBuf.WriteString("accept//");
	m_sockBuf.WriteLong(port);
	
	return SendNBuffer(addr);
}


/*
=====================================
send server info to this address
=====================================
*/

void CServer::WriteInfoMsg(CNBuffer *dest)
{
	if(!dest)
		return;

	char msg[256];
	char temp[8];

	memset(msg,0,256);
	memset(temp,0,8);
	
	strcpy(msg,"svinfo");

	strcat(msg,"\\map\\");
	strcat(msg,m_mapname);

	strcat(msg,"\\protocol\\");
	sprintf(temp,"%d",m_protocolversion);
	strcat(msg,temp);
	memset(temp,0,8);

	strcat(msg,"\\game\\");
	strcat(msg,g_gamedir);

	strcat(msg,"\\hostname\\");
	strcat(msg,m_hostname->string);

	strcat(msg,"\\maxclients\\");
	sprintf(temp,"%d",(short)m_maxclients->value);
	strcat(msg, temp);
	memset(temp,0,8);

	strcat(msg,"\\numclients\\");
	sprintf(temp,"%d",g_netclients.m_curclients);
	strcat(msg, temp);

	dest->WriteString(msg);

	
/*	dest->WriteString("svinfo");

	dest->WriteString("\\protocol\\");
	dest->WriteShort(m_protocolversion);
	
	dest->WriteString("\\game\\");
	dest->WriteString(g_gamedir);
	
	dest->WriteString("\\map\\");
	dest->WriteString("untitled");

	dest->WriteString("\\hostname\\");
	dest->WriteString(m_maxclients->string);

	dest->WriteString("\\numclients\\");
	dest->WriteShort(g_netclients.m_curclients);

	dest->WriteString("\\maxclients\\");
	dest->WriteShort((short)m_maxclients->value);
*/
	//dest->WriteString("\\map\\");
	//dest->WriteString(mapname);
	//dest->WriteString("\\version\\");
	//dest->WriteString(
}

/*

bool CServer::SendInfoPacket(SOCKADDR_IN *addr)		//brief server info
{
	m_sockBuf.Reset();

	m_sockBuf.WriteString("svinfo\\");
	
	m_sockBuf.WriteString("game\\");
	m_sockBuf.WriteString(g_gamedir);

	m_sockBuf.WriteString("\\protocol\\");
	m_sockBuf.WriteShort(m_protocolversion);

	m_sockBuf.WriteString("\\hostname\\");
	m_sockBuf.WriteString(m_maxclients->string);

	m_sockBuf.WriteString("\\maxclients\\");
	m_sockBuf.WriteShort((short)m_maxclients->value);

	m_sockBuf.WriteString("\\numclients\\");
	m_sockBuf.WriteShort(m_netclients.m_curclients);
	
	//m_sockBuf.WriteString("\\map\\");
	//m_sockBuf.WriteString(mapname);
	//m_sockBuf.WriteString("\\version\\");
	//m_sockBuf.WriteString(
	
	return SendNBuffer(addr);
}
*/

/*
=====================================
send server status info to this address
=====================================
*/


void CServer::WriteStatusMsg(CNBuffer *dest)
{
	dest->WriteString("svstatus\\");
	
	dest->WriteString("game\\");
	dest->WriteString(g_gamedir);

	dest->WriteString("\\protocol\\");
	dest->WriteShort(m_protocolversion);

	dest->WriteString("\\hostname\\");
	dest->WriteString(m_maxclients->string);

	dest->WriteString("\\maxclients\\");
	dest->WriteShort((short)m_maxclients->value);

	dest->WriteString("\\numclients\\");
	dest->WriteShort(g_netclients.m_curclients);
	
	//dest.WriteString("\\map\\");
	//dest.WriteString(mapname);
	//dest.WriteString("\\version\\");
	//dest.WriteString(

	//client info, fraglimits etc
}

/*
bool CServer::SendStatusPacket(SOCKADDR_IN *addr)		//brief server info
{
	m_sockBuf.Reset();

	m_sockBuf.WriteString("svstatus\\");
	
	m_sockBuf.WriteString("game\\");
	m_sockBuf.WriteString(g_gamedir);

	m_sockBuf.WriteString("\\protocol\\");
	m_sockBuf.WriteShort(m_protocolversion);

	m_sockBuf.WriteString("\\hostname\\");
	m_sockBuf.WriteString(m_maxclients->string);

	m_sockBuf.WriteString("\\maxclients\\");
	m_sockBuf.WriteShort((short)m_maxclients->value);

	m_sockBuf.WriteString("\\numclients\\");
	m_sockBuf.WriteShort(m_netclients.m_curclients);
	
	//m_sockBuf.WriteString("\\map\\");
	//m_sockBuf.WriteString(mapname);
	//m_sockBuf.WriteString("\\version\\");
	//m_sockBuf.WriteString(

	//client info, fraglimits etc
	
	return SendNBuffer(addr);
}

*/

bool CServer::WritePlayerInfo(CNBuffer *dest,int playernum)
{
	return false;
}

void CServer::SV_Status(int argc,char **argv)
{
}
#endif