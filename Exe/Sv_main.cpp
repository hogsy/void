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

	m_worldName[0] = 0;
	m_pWorld = 0;
	
	//Server State
	m_active = false;
	m_numClients = 0;
	
	m_levelNum = 0;
	m_numSignOnBuffers=0;

	memset(m_printBuffer,0,sizeof(m_printBuffer));

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

	//Default to loopback
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
	CNetAddr::SetLocalServerAddr(boundAddr);
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

	//Send disconnect messages to clients, if active
	for(int i=0;i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_state != CL_SPAWNED) 
			continue;
		SendDisconnect(m_clients[i],"Server quit");
	}

	//destroy world data
	if(m_pWorld)
		world_destroy(m_pWorld);
	m_pWorld = 0;
	m_worldName[0] = 0;

	//Update State
	m_numClients = 0;
	m_active = false;
	m_levelNum = 0;
	
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

	for(int i=0;i<m_cMaxClients.ival;i++)
	{
		if(m_clients[i].m_state == CL_SPAWNED)
			SendReconnect(m_clients[i]);
	}

	if(m_pWorld)
	{
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


	bool bRestarting = false;
	char mappath[COM_MAXPATH];
	char worldname[64];

	strcpy(worldname, mapname);
	strcpy(mappath, szWORLDDIR);
	strcat(mappath, worldname);
	Util::SetDefaultExtension(mappath,".bsp");

	//Shutdown if currently active
	if(m_active)
	{
		Restart();
		bRestarting = true;
	}
	else if(!Init())
		return;

	m_pWorld = world_create(mappath);
	if(!m_pWorld)
	{
		ComPrintf("CServer::LoadWorld: couldnt load map %s\n", mappath);
		Shutdown();
		return;
	}

	//Set worldname
	Util::RemoveExtension(m_worldName,COM_MAXPATH, worldname);

	//Load entities
	ComPrintf("%d entities, %d keys\n",m_pWorld->nentities, m_pWorld->nkeys);
	
	//create a spawnstring 
	//char entparms[512];
/*	int classkey = -1, j=0;
	CBuffer		entbuffer(512);

	for(int i=0; i< m_pWorld->nentities; i++)
	{
		ComPrintf("Entity num %d\n", i);
		classkey = -1;

		entbuffer.Reset();

		for(j=0; j< m_pWorld->entities[i].num_keys; j++)
		{
			if(strcmp("classname",m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name) == 0)
			{
				classkey = m_pWorld->entities[i].first_key + j;
				entbuffer += m_pWorld->keys[classkey].value;

				//strcpy(entparms, m_pWorld->keys[classkey].value);
			}
		}

		//found class key, now copy everything else 
		if(classkey == -1)
			continue;

		for(j=0; j< m_pWorld->entities[i].num_keys; j++)
		{
			if(m_pWorld->entities[i].first_key + j != classkey)
			{
				entbuffer += m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name;
				entbuffer += m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].value;
				
				//strcat(entparms," ");
				//strcat(entparms,m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name);
				//strcat(entparms," ");
				//strcat(entparms,m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].value);
			}
		}
		
	}

//			ComPrintf("%d: %s = %s\n",j, m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name,
//					 m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].value);
*/

	//update state
	m_levelNum ++;
	m_active = true;

	//Create Sigon-message. includes static entity baselines
	//=======================
	//all we need is the map name right now
	m_numSignOnBuffers = 1;
	m_signOnBuf[0].Write(SVC_INITCONNECTION);
	m_signOnBuf[0].Write(m_cGame.string);
	m_signOnBuf[0].Write(m_worldName);

	//if its not a dedicated server, then push "connect loopback" into the console
	if(!bRestarting && !m_cDedicated.bval)
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

/*
======================================
Print Status info
======================================
*/
void CServer::PrintServerStatus()
{
	ComPrintf("Game Path   : %s\n", m_cGame.string);
	ComPrintf("Hostname	   : %s\n", m_cHostname.string);
	ComPrintf("Max clients : %d\n", m_cMaxClients.ival);
	ComPrintf("Port        : %d\n", m_cPort.ival);

	if(!m_active)
	{
		ComPrintf("Server is inactive\n");
		return;
	}
	
	ComPrintf("Ip address  : %s\n", CNetAddr::GetLocalServerAddr());
	ComPrintf("Map name    : %s\n", m_worldName);
	ComPrintf("Num Clients : %d\n========================\n", m_numClients);
	for(int i=0; i<m_numClients; i++)
	{
		if(m_clients[i].m_state == CL_CONNECTED)
			ComPrintf("%s : Connecting\n", m_clients[i].m_name);
		else if(m_clients[i].m_state == CL_SPAWNED)
		{
			ComPrintf("%s :", m_clients[i].m_name);
			m_clients[i].m_netChan.PrintStats();
		}
//			ComPrintf("%s: Rate %.2f: Chokes %d\n", m_clients[i].m_name, 
//				1/m_clients[i].m_netChan.m_rate, m_clients[i].m_netChan.m_numChokes);
	}
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
		break;
	case CMD_STATUS:
		PrintServerStatus();
		break;
	}
}