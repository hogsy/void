#include "Sys_hdr.h"
#include "Sv_main.h"
#include "Sv_defs.h"

#include "Com_util.h"
#include "Com_world.h"

namespace {
enum 
{
	//Shutdown Server and send reconnects if needed. Then reintialize with new data
	CMD_MAP = 1,		
	//Send Reconnect. and just change map.
	CMD_CHANGELEVEL = 2,
	//Shutdown Server and send disconnects
	CMD_KILLSERVER = 3,
	
	CMD_STATUS	= 4,
	CMD_KICK = 5
};

const float GAME_FRAMETIME = 1/20;

typedef I_Game * (*GAME_LOADFUNC) (I_GameHandler * pImport, I_Console * pConsole);
typedef void (*GAME_FREEFUNC) ();

}

//==========================================================================
//Server Controller interface
//==========================================================================

namespace VoidServer {

static CServer * g_pServer=0;

bool InitializeNetwork()
{	return CNetServer::InitWinsock();
}
void ShutdownNetwork()
{	CNetServer::ShutdownWinsock();
}

void Create()
{	if(!g_pServer)
		g_pServer = new CServer;
}

void Destroy()
{	if(g_pServer)
		delete g_pServer;
}

void RunFrame()
{	g_pServer->RunFrame();
}

}

//==========================================================================
//==========================================================================

/*
======================================
Constructor
======================================
*/
CServer::CServer() : m_cPort("sv_port", "20010", CVAR_INT, CVAR_LATCH|CVAR_ARCHIVE),
					 m_cHostname("sv_hostname", "Void Server", CVAR_STRING, CVAR_LATCH|CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVAR_INT, CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVAR_STRING, CVAR_LATCH|CVAR_ARCHIVE),
					 m_chanWriter(m_net)
{
	m_numModels = 0;
	m_numImages = 0;
	m_numSounds = 0;

	m_entities = 0;
	m_clients = 0;

	m_fGameTime = 0.0f;
	m_pWorld = 0;
	m_active = false;

	memset(m_printBuffer,0,512);

	System::GetConsole()->RegisterCVar(&m_cHostname);
	System::GetConsole()->RegisterCVar(&m_cGame);
	System::GetConsole()->RegisterCVar(&m_cPort,this);
	System::GetConsole()->RegisterCVar(&m_cMaxClients,this);

	System::GetConsole()->RegisterCommand("map",CMD_MAP, this);
	System::GetConsole()->RegisterCommand("changelevel",CMD_CHANGELEVEL, this);
	System::GetConsole()->RegisterCommand("killserver",CMD_KILLSERVER, this);
	System::GetConsole()->RegisterCommand("kick",CMD_KICK, this);
	System::GetConsole()->RegisterCommand("status",CMD_STATUS, this);
}	

/*
======================================
Destructor
======================================
*/
CServer::~CServer()
{	
	Shutdown();

	m_entities = 0;
	m_clients = 0;
}

/*
======================================
Initialize the Server from dead state
======================================
*/
bool CServer::Init()
{
	//Unlatch all CVARS
/*
	m_cGame.Unlatch();
	m_cHostname.Unlatch();
	m_cPort.Unlatch();

	//Initialize State values
	strcpy(m_svState.gameName, m_cGame.string);
	strcpy(m_svState.hostName, m_cHostname.string);
	m_svState.maxClients = m_cMaxClients.ival;
	m_svState.port = m_cPort.ival;
	memset(m_svState.worldname, 0, sizeof(m_svState.worldname));
	m_svState.levelId = 0;
*/
	m_svCmds.clear();
	ResetServerState();

	//Initialize Network
	if(!m_net.Init(this, &m_svState))
		return false;
	strcpy(m_svState.localAddr, m_net.GetLocalAddr());

	//Load the game dll
	m_hGameDll = ::LoadLibrary("vgame.dll");
	if(m_hGameDll == NULL)
	{
		ComPrintf("CServer::Init: Failed to load game dll\n");
		Shutdown();
		return false;
	}

	GAME_LOADFUNC pfnLoadFunc = (GAME_LOADFUNC)::GetProcAddress(m_hGameDll,"GAME_GetAPI");
	if(!pfnLoadFunc)
	{
		ComPrintf("CServer::Init: Failed to get Load Func\n");
		Shutdown();
		return false;
	}

	m_pGame = pfnLoadFunc(this,System::GetConsole());
	if(!m_pGame || !m_pGame->InitGame())
	{
		ComPrintf("CServer::Init: Failed to initialize Game Interface\n");
		Shutdown();
		return false;
	}

	m_entities = m_pGame->entities;
	m_clients = m_pGame->clients;
	m_active = true;

	ComPrintf("CServer::Init OK: %d commands in buffer\n", m_svCmds.size());
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
	
	m_active = false;

	//Send Disconnects
	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(m_clients[i] && m_clients[i]->inUse)
			m_net.SendDisconnect(i, DR_SVQUIT);
	}

	//Kill the network
	m_net.Shutdown();

	//Kill the game
	if(m_pGame)
	{
		m_pGame->ShutdownGame();
		GAME_FREEFUNC pfnFreeFunc = (GAME_FREEFUNC)::GetProcAddress(m_hGameDll,"GAME_Shutdown");
		if(pfnFreeFunc)
			pfnFreeFunc();
		else
			ComPrintf("CServer::Shutdown: Failed to get Load Func\n");
		::FreeLibrary(m_hGameDll);
		m_hGameDll = 0;
	}

	ResetServerState();
/*
	//Unlatch Vars
	m_cGame.Unlatch();
	m_cHostname.Unlatch();
	m_cPort.Unlatch();

	//Reset State info
	strcpy(m_svState.gameName, m_cGame.string);
	strcpy(m_svState.hostName, m_cHostname.string);
	m_svState.maxClients = m_cMaxClients.ival;
	m_svState.port = m_cPort.ival;
	m_svState.levelId = 0;
	m_svState.numClients = 0;
	m_svState.levelId = 0;
	memset(m_svState.worldname,0,sizeof(m_svState.worldname));
	
	//destroy world data
	if(m_pWorld)
		CWorld::DestroyWorld(m_pWorld);
	m_pWorld = 0;
*/

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

	m_pGame->ShutdownGame();
	m_net.Restart();

	ResetServerState();
/*	if(m_pWorld)
	{
		CWorld::DestroyWorld(m_pWorld);
		m_pWorld = 0;
		memset(m_svState.worldname, 0, sizeof(m_svState.worldname));
	}
*/
	m_pGame->InitGame();
	ComPrintf("CServer::Restart OK\n");
}


/*
================================================

================================================
*/
void CServer::ChangeLevel(const char * worldName)
{
}



/*
================================================
Reset Server State Vars
================================================
*/
void CServer::ResetServerState()
{
	//Unlatch Vars
	m_cGame.Unlatch();
	m_cHostname.Unlatch();
	m_cPort.Unlatch();

	//Reset State info
	strcpy(m_svState.gameName, m_cGame.string);
	strcpy(m_svState.hostName, m_cHostname.string);
	m_svState.maxClients = m_cMaxClients.ival;
	m_svState.port = m_cPort.ival;
	m_svState.levelId = 0;
	m_svState.numClients = 0;
	m_svState.levelId = 0;
	memset(m_svState.worldname,0,sizeof(m_svState.worldname));
	
	//destroy world data
	if(m_pWorld)
		CWorld::DestroyWorld(m_pWorld);
	m_pWorld = 0;
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
	srand((uint)System::GetCurTime());

	//Get updates
	m_net.ReadPackets();


	//Run game at fixed speed
	if(m_fGameTime < System::GetCurTime())
	{
		m_fGameTime = System::GetCurTime() + GAME_FRAMETIME;
		m_pGame->RunFrame(System::GetCurTime());
	}

	//run clients
	//go through all the clients, find entities in their pvs and update them
	//Add client info to all connected clients
	for(int i=0;i<m_svState.maxClients;i++)
	{
		if(m_clients[i] && m_clients[i]->spawned && m_net.ChanCanSend(i))
		{
			for(int j=0; j<m_svState.maxClients; j++)
			{
				if(!m_clients[j] || !m_clients[j]->spawned || i==j)
					continue;

				m_net.ChanBeginWrite(i,SV_CLUPDATE, 0);
				m_net.ChanWriteShort(m_clients[j]->num);
				m_net.ChanWriteCoord(m_clients[j]->origin.x);
				m_net.ChanWriteCoord(m_clients[j]->origin.y);
				m_net.ChanWriteCoord(m_clients[j]->origin.z);
				m_net.ChanWriteAngle(m_clients[j]->angles.x);
				m_net.ChanWriteAngle(m_clients[j]->angles.y);
				m_net.ChanWriteAngle(m_clients[j]->angles.z);
				m_net.ChanFinishWrite();
			}
		}
	}

	//write to clients
	m_net.SendPackets();

	//Exec these last thing so changes don't screw up the current frame
	ExecServerCommands();
}


/*
================================================
Execute the buffer server commands
================================================
*/
void CServer::ExecServerCommands()
{
	if(m_svCmds.size())
	{
		for(StringList::iterator it = m_svCmds.begin(); it != m_svCmds.end(); it++)
			System::GetConsole()->ExecString(it->c_str());
		m_svCmds.clear();
	}		
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
		return;

	bool bRestarting = false;
	char mappath[COM_MAXPATH];
	char worldName[64];

	strcpy(worldName,mapname);
	strcpy(mappath, GAME_WORLDSDIR);
	strcat(mappath, mapname);
	Util::SetDefaultExtension(mappath,VOID_DEFAULTMAPEXT);

	//Restart if already active
	if(m_active)
	{	
		Restart();
		bRestarting = true;
	}
	else
	{
		if(!Init())
		{	
			ComPrintf("CServer::LoadWorld: Error initializing server\n");
			return;
		}
	}

	//Load World
	m_pWorld = CWorld::CreateWorld(mappath);
	if(!m_pWorld)
	{
		ComPrintf("CServer::LoadWorld: Could not load map %s\n", mappath);
		Shutdown();
		return;
	}
	//Load Entitiys in the world
	Util::RemoveExtension(m_svState.worldname,COM_MAXPATH, worldName);
	LoadEntities();

	//Write SignON data
	WriteSignOnBuffer();

	//update state
	m_svState.levelId ++;
	m_active = true;

	//if its not a dedicated server, then push "connect loopback" into the console
	if(!bRestarting)
		System::GetConsole()->ExecString("connect localhost");

	ComPrintf("CServer::LoadWorld : OK\n");
}


/*
======================================
Parse and read entities
======================================
*/
void CServer::LoadEntities()
{
	//create a spawnstring 
	int i=0, j=0;
	int classkey= -1;
	CBuffer entBuffer;

	for(i=0; i< m_pWorld->nentities; i++)
	{
		classkey = -1;
		entBuffer.Reset();

		//Look for classname
		for(j=0; j< m_pWorld->entities[i].num_keys; j++)
		{
			if(strcmp("classname",m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name) == 0)
			{
				classkey = m_pWorld->entities[i].first_key + j;
				entBuffer.WriteString(m_pWorld->keys[classkey].value);
			}
		}

		//Were we able to find the classname of the given key
		if(classkey == -1)
			continue;

		for(j=0; j< m_pWorld->entities[i].num_keys; j++)
		{
			if(m_pWorld->entities[i].first_key + j != classkey)
			{
				entBuffer.WriteString(m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].name);
				entBuffer.WriteString(m_pWorld->keys[(m_pWorld->entities[i].first_key + j)].value);
			}
		}
		m_pGame->SpawnEntity(entBuffer);
	}
	ComPrintf("%d entities, %d keys\n",m_pWorld->nentities, m_pWorld->nkeys);
}

/*
======================================
Get a handle to all the
======================================
*/
void CServer::WriteSignOnBuffer()
{
	CBuffer buffer;
	int numBufs, i;

	m_signOnBufs.Reset();

	//=================================
	//Write game info
	m_signOnBufs.gameInfo.WriteString(m_svState.gameName);
	m_signOnBufs.gameInfo.WriteString(m_svState.worldname);

	//==================================
	//Write	imagelist
	numBufs = 0;
	m_signOnBufs.imageList[0].WriteShort(m_numImages);
	for(i=0;i<m_numImages; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i); //m_imageList[i].id);
		buffer.WriteString(m_imageList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!m_signOnBufs.imageList[numBufs].HasSpace(buffer.GetSize()))
		{
			if(numBufs + 1 < NetSignOnBufs::MAX_MODEL_BUFS)
				numBufs ++;
			else
			{	
				//Error ran out of space to write entity info, FATAL ?
				ComPrintf("CServer::WriteSignOnBuffer: Out of space for modellist\n");
				return;
			}
		}
		m_signOnBufs.imageList[numBufs].WriteBuffer(buffer);
	}
	m_signOnBufs.numImageBufs = numBufs+1;
	for(i=0;i<m_signOnBufs.numImageBufs; i++)
		ComPrintf("SignOn ImageBuf %d : %d bytes\n", i, m_signOnBufs.imageList[i].GetSize()); 

	
	//==================================
	//Write	modellist
	numBufs = 0;
	m_signOnBufs.modelList[0].WriteShort(m_numModels);
	for(i=0;i<m_numModels; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i);
		buffer.WriteString(m_modelList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!m_signOnBufs.modelList[numBufs].HasSpace(buffer.GetSize()))
		{
			if(numBufs + 1 < NetSignOnBufs::MAX_MODEL_BUFS)
				numBufs ++;
			else
			{	
				//Error ran out of space to write entity info, FATAL ?
				ComPrintf("CServer::WriteSignOnBuffer: Out of space for modellist\n");
				return;
			}
		}
		m_signOnBufs.modelList[numBufs].WriteBuffer(buffer);
	}
	m_signOnBufs.numModelBufs= numBufs+1;
	for(i=0;i<m_signOnBufs.numModelBufs; i++)
		ComPrintf("SignOn ModelBuf %d : %d bytes\n", i, m_signOnBufs.modelList[i].GetSize()); 

	//==================================
	//Write Soundlist
	numBufs = 0;
	m_signOnBufs.soundList[0].WriteShort(m_numSounds);
	for(i=0;i<m_numSounds; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i);
		buffer.WriteString(m_soundList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!m_signOnBufs.soundList[numBufs].HasSpace(buffer.GetSize()))
		{
			if(numBufs + 1 < NetSignOnBufs::MAX_SOUND_BUFS)
				numBufs ++;
			else
			{	
				//Error ran out of space to write entity info, FATAL ?
				ComPrintf("CServer::WriteSignOnBuffer: Out of space for soundList\n");
				return;
			}
		}
		m_signOnBufs.soundList[numBufs].WriteBuffer(buffer);
	}
	m_signOnBufs.numSoundBufs= numBufs+1;
	for(i=0;i<m_signOnBufs.numSoundBufs; i++)
		ComPrintf("SignOn SoundBuf %d : %d bytes\n", i, m_signOnBufs.soundList[i].GetSize()); 

	
	//==================================
	//Write entity baselines
	numBufs = 0;
	for(i=0; i<m_pGame->numEnts; i++)
	{
		buffer.Reset();
		if(!m_entities[i] || !WriteEntBaseLine(m_entities[i],buffer))
			continue;

		//Check if the signOn buffer has space for this entity
		if(!m_signOnBufs.entityList[numBufs].HasSpace(buffer.GetSize()))
		{
			if(numBufs + 1 < NetSignOnBufs::MAX_ENTITY_BUFS)
				numBufs ++;
			else
			{	
				//Error ran out of space to write entity info, FATAL ?
				ComPrintf("CServer::ParseEntities: Out of space for Entities\n");
				return;
			}
		}
		m_signOnBufs.entityList[numBufs].WriteBuffer(buffer);
	}

	m_signOnBufs.numEntityBufs= numBufs+1;
	for(i=0;i<m_signOnBufs.numEntityBufs; i++)
		ComPrintf("SignOn EntBuf %d : %d bytes\n", i, m_signOnBufs.entityList[i].GetSize()); 
}

/*
======================================
Write baselines of entities
======================================
*/
bool CServer::WriteEntBaseLine(const Entity * ent, CBuffer &buf) const
{
	//we only write a baseline if the entity is using a model or soundIndex
	if(ent->mdlIndex >= 0 || ent->sndIndex >= 0)
	{
		buf.WriteShort(ent->num);
		buf.WriteByte(ent->moveType);
		buf.WriteCoord(ent->origin.x);
		buf.WriteCoord(ent->origin.y);
		buf.WriteCoord(ent->origin.z);
		buf.WriteAngle(ent->angles.x);
		buf.WriteAngle(ent->angles.y);
		buf.WriteAngle(ent->angles.z);

		if(ent->mdlIndex >=0)
		{
			buf.WriteChar('m');
			buf.WriteShort(ent->mdlIndex);
			buf.WriteShort(ent->skinNum);
			buf.WriteShort(ent->frameNum);
		}
		if(ent->sndIndex >=0)
		{
			buf.WriteChar('s');
			buf.WriteShort(ent->sndIndex);
			buf.WriteShort(ent->volume);
			buf.WriteShort(ent->attenuation);
		}
		//Set END OF MESSAGE
		buf.WriteChar(0);
		return true;
	}
	return false;
}


//==========================================================================
//==========================================================================

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

	ComPrintf("Local Addr : %s\n", m_svState.localAddr);
	ComPrintf("Map name   : %s\n", m_svState.worldname);
	ComPrintf("Map id     : %d\n", m_svState.levelId);

	for(int i=0; i<m_svState.maxClients; i++)
	{
		if(m_clients[i])
		{
			ComPrintf("%s:\n", m_clients[i]->name);

			const NetChanState & state = m_net.ChanGetState(i);
			ComPrintf("  Rate:%d\n  In:%d\n  Acked:%d\n  Out:%d\n", 
						m_net.ChanGetRate(i), state.inMsgId, state.inAckedId, state.outMsgId);
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
	if(parms.NumTokens() == 1)
		return false;

	if(cvar == reinterpret_cast<CVarBase *>(&m_cPort))
	{
		int port = parms.IntTok(1);
		if(port > 32767 || port < 1024)
		{
			ComPrintf("Port out of range. Should be with in 1024 to 32767\n");
			return false;
		}
		return true;
	}
	else if(cvar == reinterpret_cast<CVarBase *>(&m_cMaxClients))
	{
		int maxclients = parms.IntTok(1);
		if(maxclients < 1 || maxclients > GAME_MAXCLIENTS)
		{
			ComPrintf("Max Clients should be between 1 and %d\n", GAME_MAXCLIENTS);
			return false;
		}
		m_svState.maxClients = maxclients;
		return true;
	}
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
	case CMD_CHANGELEVEL:
		{
			break;
		}
	case CMD_KICK:
		{
			break;
		}
	}
}