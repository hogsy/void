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

	m_numSignOnBuffers=0;
	for(int i=0;i<MAX_SIGNONBUFFERS;i++)
		m_signOnBuf[i].Create(MAX_DATAGRAM_SIZE);

	m_worldName[0] = 0;
	m_pWorld = 0;
	
	//Server State
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
	SOCKADDR_IN* pAddrInet=0;

	char   boundAddr[24];
	memset(boundAddr,0,24);
	
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
	if(!netaddr.IsValid())
	{
		ComPrintf("CServer::Init:Unable to resolve ip address\n");
		return false;
	}

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

	//Update State
	m_numClients = 0;
	m_active = false;
	m_levelNum = 0;

	
	//Send disconnect messages to clients, if active
	for(int i=0;i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_state != CL_SPAWNED) 
			continue;

		//send disconect messages
		m_clients[i].m_netChan.m_reliableBuffer.Reset();
		m_clients[i].m_netChan.m_buffer.Reset();
		m_clients[i].m_netChan.m_buffer += SV_DISCONNECT;
		m_clients[i].m_netChan.m_buffer += "Server quit";
		m_clients[i].m_netChan.PrepareTransmit();
		m_pSock->SendTo(m_clients[i].m_netChan.m_sendBuffer, m_clients[i].m_netChan.m_addr);
		m_clients[i].Reset();
	}

	if(m_pWorld)
		world_destroy(m_pWorld);
	m_pWorld = 0;
	m_worldName[0] = 0;
	
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