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
		CMD_KILLSERVER = 2
	};
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
CServer::CServer() : m_recvBuf(CNetBuffer::DEFAULT_BUFFER_SIZE),
					 m_sendBuf(CNetBuffer::DEFAULT_BUFFER_SIZE),
					 m_cPort("sv_port", "20010", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cDedicated("sv_dedicated", "0", CVar::CVAR_BOOL, CVar::CVAR_LATCH),
					 m_cHostname("sv_hostname", "Skidz", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVar::CVAR_STRING, CVar::CVAR_ARCHIVE)
{
	m_pSock = new CNetSocket(&m_recvBuf);
	
	m_challenges = new NetChallenge[MAX_CHALLENGES];

	m_pWorld = 0;
	
	m_active = false;
	m_numClients = 0;

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
			ComPrintf("IP: %s", pAddrString);
		
		addrFlags = localAddr[i].iiFlags;
		if (addrFlags & IFF_UP)
		{
			if(!(addrFlags & IFF_LOOPBACK))
			{
				strcpy(boundAddr,pAddrString);
				ComPrintf("  This interface is up");
			}
		}
		if (addrFlags & IFF_LOOPBACK)
			ComPrintf("  This is the loopback interface");
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

		world_destroy(m_pWorld);
		m_pWorld = 0;
		m_worldName[0] = 0;
	}

	m_pSock->Close();
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
	if(!mapname)
	{
		if(m_worldName[0])
			ComPrintf("Playing %s\n",m_worldName);
		else
			ComPrintf("No world loaded\n");
		return;
	}

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

	//Create Sigon-message.
	//=======================

	//Set worldname
	Util::RemoveExtension(m_worldName,COM_MAXPATH, mapname);

	//all we need is the map name right now
	m_numSignOnBuffers = 1;
	strcpy(m_szSignOnBuf[0], mapname);
	m_signOnBufSize[0]= strlen(m_szSignOnBuf[0]);

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
	
//	m_pSock->SendTo(m_sendBuf.GetData(), m_sendBuf.GetSize(), m_pSock->GetSource());
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

	//Client is ready to be accepted

	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_ACCEPT;
	m_sendBuf += m_worldName;
	
//	m_pSock->SendTo(m_sendBuf.GetData(), m_sendBuf.GetSize(), m_pSock->GetSource());
	m_pSock->Send(m_sendBuf);

	//All clear, accept the client
	m_numClients ++;
	ComPrintf("%s connected\n", m_recvBuf.ReadString());
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

	m_sendBuf.Reset();
	m_sendBuf += -1;
	m_sendBuf += S2C_CHALLENGE;
	m_sendBuf += m_challenges[i].challenge;

	//Send response packet
//	m_pSock->SendTo(m_sendBuf.GetData(), m_sendBuf.GetSize(),m_challenges[i].addr);
	m_pSock->SendTo(m_sendBuf, m_challenges[i].addr); 

	ComPrintf("Sent CHAL %d to %s\n", m_challenges[i].challenge, m_challenges[i].addr.ToString());
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
//		m_pSock->SendTo((byte*)VNET_PING, strlen(VNET_PING), m_pSock->GetSource());
	else if(strcmp(msg, C2S_GETSTATUS) == 0)
		HandleStatusReq();
	else if(strcmp(msg, C2S_CONNECT) == 0)
		HandleConnectReq();
	else if(strcmp(msg, C2S_GETCHALLENGE) == 0)
		HandleChallengeReq();
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
		packetId = m_recvBuf.ReadInt();

		if(packetId == -1)
		{
			ProcessQueryPacket();
			continue;
		}

		//find out type of message
	}
}

//======================================================================================
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

	//send updates
}


//======================================================================================
//======================================================================================

/*
==========================================
Handle CVars
==========================================
*/
bool CServer::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	return false;
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
		break;
	}
}


