#include "Sv_main.h"
#include "World.h"
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

CServer * g_pServer=0;

/*
======================================
Constructor/Destructor
======================================
*/
CServer::CServer() : m_cPort("sv_port", "20010", CVAR_INT, CVAR_LATCH|CVAR_ARCHIVE),
					 m_cHostname("sv_hostname", "Void Server", CVAR_STRING, CVAR_LATCH|CVAR_ARCHIVE),
					 m_cMaxClients("sv_maxclients", "4", CVAR_INT, CVAR_ARCHIVE),
					 m_cGame("sv_game", "Game", CVAR_STRING, CVAR_LATCH|CVAR_ARCHIVE),
					 m_chanWriter(m_net)
{
	g_pServer = this;

	m_entities = 0;
	m_clients = 0;

	//Initialize Network Server
	m_net.Create(this, &m_svState);

	m_numModels = 0;
	m_numImages = 0;
	m_numSounds = 0;

	m_pWorld = 0;
	m_active = false;
	
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
	g_pServer = 0;

	Shutdown();

	if(m_entities)
	{
		for(int i=0;i<GAME_MAXENTITES; i++)
		{
			if(m_entities[i]) 
				delete m_entities[i];
		}
		delete [] m_entities;
	}
	if(m_clients)
	{
		for(int i=0;i<GAME_MAXCLIENTS; i++)
		{
			if(m_clients[i]) 
				delete m_clients[i];
		}
		delete [] m_clients;
	}
}

/*
======================================
Initialize the Server from dead state
======================================
*/
bool CServer::Init()
{
	m_cGame.Unlatch();
	m_cHostname.Unlatch();
	m_cPort.Unlatch();

	//Initialize State values
	strcpy(m_svState.gameName, m_cGame.string);
	strcpy(m_svState.hostName, m_cHostname.string);
	m_svState.maxClients = m_cMaxClients.ival;
	m_svState.port = m_cPort.ival;
	m_svState.worldname[0] = 0;
	m_svState.levelId = 0;

	if(!m_net.Init())
		return false;
	
	strcpy(m_svState.localAddr, m_net.GetLocalAddr());
	m_active = true;

	//Initialize Game
	m_maxEntities=0;
	m_numEntities= 0;

	m_entities = new Entity * [GAME_MAXENTITES];
	memset(m_entities,0,(sizeof(Entity *) * GAME_MAXENTITES));

	m_clients = new EntClient * [GAME_MAXCLIENTS];
	memset(m_clients,0,(sizeof(EntClient*) * GAME_MAXCLIENTS));

	InitGame();
	
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

	m_cGame.Unlatch();
	m_cHostname.Unlatch();
	m_cPort.Unlatch();

	strcpy(m_svState.gameName, m_cGame.string);
	strcpy(m_svState.hostName, m_cHostname.string);
	m_svState.maxClients = m_cMaxClients.ival;
	m_svState.port = m_cPort.ival;
	m_svState.worldname[0] = 0;
	m_svState.levelId = 0;

	m_svState.numClients = 0;
	m_svState.levelId = 0;
	memset(m_svState.worldname,0,sizeof(m_svState.worldname));
	m_active = false;

	//Destroy nonpersistant entities

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

//	m_active = false;

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
}

//======================================================================================
//======================================================================================

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

		//found classkey, see if we want info about this entity
		//then copy over all the parms
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
		SpawnEntity(entBuffer);
	}
	ComPrintf("%d entities, %d keys\n",m_pWorld->nentities, m_pWorld->nkeys);
}


/*
======================================
Get a handle to all the
======================================
*/
void CServer::WriteSignOnBuffer(NetSignOnBufs &signOnBuf)
{
	CBuffer buffer;
	int numBufs, i;

	//=================================
	//Write game info
	signOnBuf.gameInfo.WriteString(m_svState.gameName);
	signOnBuf.gameInfo.WriteString(m_svState.worldname);

	//==================================
	//Write	imagelist
	numBufs = 0;
	signOnBuf.imageList[0].WriteShort(m_numImages);
	for(i=0;i<m_numImages; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i); //m_imageList[i].id);
		buffer.WriteString(m_imageList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!signOnBuf.imageList[numBufs].HasSpace(buffer.GetSize()))
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
		signOnBuf.imageList[numBufs].WriteBuffer(buffer);
	}
	signOnBuf.numImageBufs = numBufs+1;
	for(i=0;i<signOnBuf.numImageBufs; i++)
		ComPrintf("SignOn ImageBuf %d : %d bytes\n", i, signOnBuf.imageList[i].GetSize()); 

	
	//==================================
	//Write	modellist
	numBufs = 0;
	signOnBuf.modelList[0].WriteShort(m_numModels);
	for(i=0;i<m_numModels; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i);
		buffer.WriteString(m_modelList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!signOnBuf.modelList[numBufs].HasSpace(buffer.GetSize()))
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
		signOnBuf.modelList[numBufs].WriteBuffer(buffer);
	}
	signOnBuf.numModelBufs= numBufs+1;
	for(i=0;i<signOnBuf.numModelBufs; i++)
		ComPrintf("SignOn ModelBuf %d : %d bytes\n", i, signOnBuf.modelList[i].GetSize()); 

	//==================================
	//Write Soundlist
	numBufs = 0;
	signOnBuf.soundList[0].WriteShort(m_numSounds);
	for(i=0;i<m_numSounds; i++)
	{
		buffer.Reset();
		buffer.WriteShort(i);
		buffer.WriteString(m_soundList[i].name);

		//Check if the signOn buffer has space for this entity
		if(!signOnBuf.soundList[numBufs].HasSpace(buffer.GetSize()))
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
		signOnBuf.soundList[numBufs].WriteBuffer(buffer);
	}
	signOnBuf.numSoundBufs= numBufs+1;
	for(i=0;i<signOnBuf.numSoundBufs; i++)
		ComPrintf("SignOn SoundBuf %d : %d bytes\n", i, signOnBuf.soundList[i].GetSize()); 

	
	//==================================
	//Write entity baselines
	numBufs = 0;
	for(i=0; i<m_numEntities; i++)
	{
		buffer.Reset();
		if(!m_entities[i] || !m_entities[i]->WriteBaseline(buffer))
			continue;

		//Check if the signOn buffer has space for this entity
		if(!signOnBuf.entityList[numBufs].HasSpace(buffer.GetSize()))
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
		signOnBuf.entityList[numBufs].WriteBuffer(buffer);
	}

	signOnBuf.numEntityBufs= numBufs+1;
	for(i=0;i<signOnBuf.numEntityBufs; i++)
		ComPrintf("SignOn EntBuf %d : %d bytes\n", i, signOnBuf.entityList[i].GetSize()); 
}

/*
==========================================
Load the World
==========================================
*/
void CServer::LoadWorld(const char * mapname)
{
	//Get Map name
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
	Util::SetDefaultExtension(mappath,VOID_DEFAULTMAPEXT);

	//Shutdown if currently active
	if(m_active)
	{
		Restart();
		bRestarting = true;
	}
	else
	{
		 if(!Init())
			return;
	}

	//Load World
	m_pWorld = world_create(mappath);
	if(!m_pWorld)
	{
		ComPrintf("CServer::LoadWorld: Could not load map %s\n", mappath);
		Shutdown();
		return;
	}

	//Set worldname
	Util::RemoveExtension(m_svState.worldname,COM_MAXPATH, worldname);

	//Create Sigon-message. includes static entity baselines
	//=======================
	//all we need is the map name right now

	LoadEntities();


	NetSignOnBufs & signOnBuf = m_net.GetSignOnBufs();

	//first reset all the signON buffers
	signOnBuf.gameInfo.Reset();
	signOnBuf.numImageBufs = 0;
	for(int i=0; i< NetSignOnBufs::MAX_IMAGE_BUFS; i++)
		signOnBuf.imageList[i].Reset();
	signOnBuf.numSoundBufs = 0;
	for(i=0; i< NetSignOnBufs::MAX_SOUND_BUFS; i++)
		signOnBuf.soundList[i].Reset();
	signOnBuf.numModelBufs = 0;
	for(i=0; i< NetSignOnBufs::MAX_MODEL_BUFS; i++)
		signOnBuf.modelList[i].Reset();
	signOnBuf.numEntityBufs = 0;
	for(i=0; i< NetSignOnBufs::MAX_ENTITY_BUFS; i++)
		signOnBuf.entityList[i].Reset();

	WriteSignOnBuffer(signOnBuf);

	//update state
	m_svState.levelId ++;
	m_active = true;

	//if its not a dedicated server, then push "connect loopback" into the console
	if(!bRestarting)
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

	ComPrintf("Local Addr : %s\n", m_svState.localAddr);
	ComPrintf("Map name   : %s\n", m_svState.worldname);
	ComPrintf("Map id     : %d\n", m_svState.levelId);

//	EntClient * client = 0;
	for(int i=0; i<m_svState.maxClients; i++)
	{
		if(m_clients[i])
		{
//			client = reinterpret_cast<EntClient *>(m_entities[i]);
			ComPrintf("%s:\n", m_clients[i]->name);

			const NetChanState & state = m_net.ChanGetState(i);
			ComPrintf("  Rate:%d\n  In:%d\n  Acked:%d\n  Out:%d\n", 
						m_net.ChanGetRate(i), state.inMsgId, state.inAckedId, state.outMsgId);  //1.0/state.rate
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
	}
}

//======================================================================================
//======================================================================================

/*
======================================
Return an id for the given model
======================================
*/
int CServer::RegisterModel(const char * model)
{
	if(m_numModels == GAME_MAXMODELS)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXMODELS; i++)
	{
		//we reached the end, no more models after this
		if(!m_modelList[i].name)
			break;
		if(strcmp(m_modelList[i].name, model) == 0)
		    return i;
	}

	m_numModels ++;
	m_modelList[i].name = new char[strlen(model)+1];
	strcpy(m_modelList[i].name,model);
	return i;
}

/*
======================================
Return an id for the given sound
======================================
*/
int CServer::RegisterSound(const char * sound)
{
	if(m_numSounds == GAME_MAXSOUNDS)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXSOUNDS; i++)
	{
		//we reached the end, no more models after this
		if(!m_soundList[i].name)
			break;
		if(strcmp(m_soundList[i].name, sound) == 0)
		    return i;
	}

	m_numSounds ++;
	m_soundList[i].name = new char[strlen(sound)+1];
	strcpy(m_soundList[i].name,sound);
	return i;
}

/*
======================================
Return an id for the given image
======================================
*/
int CServer::RegisterImage(const char * image)
{
	if(m_numImages == GAME_MAXIMAGES)
		return -1;

	//Check to see whether we already have a model Id by that name
	for(int i=0; i<GAME_MAXIMAGES; i++)
	{
		//we reached the end, no more models after this
		if(!m_imageList[i].name)
			break;
		if(strcmp(m_imageList[i].name, image) == 0)
		    return i;
	}

	m_numImages ++;
	m_imageList[i].name = new char[strlen(image)+1];
	strcpy(m_imageList[i].name,image);
	return i;
}



//======================================================================================
//======================================================================================


/*
======================================
Check in an entity should be spawned
this will be implemented by teh game dll
======================================
*/
bool CServer::SpawnEntity(CBuffer &buf)
{
	buf.BeginRead();
	char * classname = buf.ReadString();

	Entity * ent = CEntityMaker::CreateEnt(classname,buf);
	if(ent)
	{
		m_entities[m_numEntities] = ent;
		m_entities[m_numEntities]->num = m_numEntities;
		m_numEntities++;
		return true;
	}
	return false;
}

















