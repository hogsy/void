#include "Sv_main.h"
#include "World.h"
#include "Sv_ents.h"
#include "Com_util.h"
#include "Net_defs.h"
#include "Net_protocol.h"

//======================================================================================
enum 
{
	CMD_MAP = 1,
	CMD_KILLSERVER = 2,
	CMD_STATUS	= 3
};

/*
======================================
Constructor/Destructor
======================================
*/
CServer::CServer() : m_cPort("sv_port", "20010", CVar::CVAR_INT, CVar::CVAR_LATCH|CVar::CVAR_ARCHIVE),
					 m_cDedicated("sv_dedicated", "0", CVar::CVAR_BOOL, CVar::CVAR_LATCH),
					 m_cHostname("sv_hostname", "Void Server", CVar::CVAR_STRING, CVar::CVAR_LATCH|CVar::CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVar::CVAR_STRING, CVar::CVAR_LATCH|CVar::CVAR_ARCHIVE),
					 m_chanWriter(m_net)
{
	//Initialize Network Server
	m_net.Create(this, &m_svState);

	m_client= new CEntClient[SV_MAX_CLIENTS];

	//Default State values
	strcpy(m_svState.gameName,"Game");
	strcpy(m_svState.hostName,"Void Server");
	m_svState.worldname[0] = 0;
	m_svState.levelId = 0;
	m_svState.maxClients = 4;
	m_svState.port = SV_DEFAULT_PORT;

	m_pWorld = 0;
	m_active = false;
	
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
	delete [] m_client;
}


/*
======================================
Initialize the Server
======================================
*/
bool CServer::Init()
{
	if(!m_net.Init())
		return false;
	
	//more initialization here ?
	strcpy(m_svState.localAddr, m_net.GetLocalAddr());
	m_active = true;

	return true;
}

/*
======================================
Shutdown the server
======================================
*/
void CServer::Shutdown()
{
	if(!m_active)
		return;

	m_net.Shutdown();

	m_svState.numClients = 0;
	m_svState.levelId = 0;
	memset(m_svState.worldname,0,sizeof(m_svState.worldname));
	m_active = false;
	
	//destroy world data
	if(m_pWorld)
		world_destroy(m_pWorld);
	m_pWorld = 0;
	
	ComPrintf("CServer::Shutdown OK\n");
}


/*
======================================
Restart the server
======================================
*/
void CServer::Restart()
{
	if(!m_active)
		return;

	m_active = false;

	m_net.Restart();

	if(m_pWorld)
	{
		world_destroy(m_pWorld);
		m_pWorld = 0;
		m_svState.worldname[0] = 0;
	}
	ComPrintf("CServer::Restart OK\n");
}


/*
==========================================
Run a server frame
==========================================
*/
void CServer::RunFrame()
{
	if(m_active == false)
		return;

	//Re-seed current time
	srand((uint)System::g_fcurTime);

	//Get updates
	m_net.ReadPackets();

	//Run game

	//write to clients
	m_net.SendPackets();
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
		if(m_svState.worldname[0])
			ComPrintf("Playing %s\n",m_svState.worldname);
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
	Util::RemoveExtension(m_svState.worldname,COM_MAXPATH, worldname);

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
	m_svState.levelId ++;
	m_active = true;

	//Create Sigon-message. includes static entity baselines
	//=======================
	//all we need is the map name right now

	NetSignOnBufs & buf = m_net.GetSignOnBufs();
	
	buf.gameInfo.Write(SVC_INITCONNECTION);
	buf.gameInfo.Write(m_svState.gameName);
	buf.gameInfo.Write(m_svState.worldname);

	//if its not a dedicated server, then push "connect loopback" into the console
	if(!bRestarting && !m_cDedicated.bval)
		System::GetConsole()->ExecString("connect localhost");
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
	ComPrintf("Game Path  : %s\n", m_svState.gameName);
	ComPrintf("Hostname	  : %s\n", m_svState.hostName);
	ComPrintf("Max clients: %d\n", m_svState.maxClients);
	ComPrintf("Port       : %d\n", m_svState.port);

	if(!m_active)
	{
		ComPrintf("Server is inactive\n");
		return;
	}

	ComPrintf("Local Addr : %d\n", m_svState.localAddr);
	ComPrintf("Map name   : %s\n", m_svState.worldname);
	ComPrintf("Map id     : %d\n", m_svState.levelId);

	for(int i=0; i<m_svState.maxClients; i++)
	{
		if(m_client[i].inUse)
		{
			ComPrintf("%s:\n", m_client[i].name);

			const NetChanState & state = m_net.ChanGetState(i);
			ComPrintf("  Rate:%.2f\n  In:%d\n  Acked:%d\n  Out:%d\n", 
						1.0/state.rate, state.inMsgId, state.inAckedId, state.outMsgId);
			ComPrintf("  Dropped:%d\n  Good:%d\n  Chokes:%d\n", 
					state.dropCount, state.goodCount, state.numChokes);
		}
	}
}

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
		{
			char mapname[64];
			parms.StringTok(1,(char*)mapname,64);
			LoadWorld(mapname);
			break;
		}
	case CMD_KILLSERVER:
		Shutdown();
		break;
	case CMD_STATUS:
		PrintServerStatus();
		break;
	}
}

