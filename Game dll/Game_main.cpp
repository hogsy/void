#include "Game_main.h"
#include "Game_ents.h"
#include "Game_defs.h"

//CMemManager g_memManager("mem_game.log");
const char MEM_SZLOGFILE[] = "mem_game.log";

I_GameHandler * g_pImports=0;
I_Console * g_pCons=0;

CGame *		g_pGame=0;


I_Game * GAME_GetAPI(I_GameHandler * pImports, I_Console * pConsole)
{
	g_pImports = pImports;
	g_pCons = pConsole;
	if(!g_pGame)
		g_pGame = new CGame();
	return g_pGame;
}

void GAME_Shutdown()
{
	if(g_pGame)
	{	delete g_pGame;
		g_pGame = 0;
	}
	g_pImports = 0;
	g_pCons = 0;
}

//======================================================================================
//======================================================================================


CGame::CGame()
{
	entities = new Entity * [GAME_MAXENTITIES];
	memset(entities, 0, sizeof(Entity *) * GAME_MAXENTITIES);
	maxEnts = GAME_MAXENTITIES;
	numEnts = 0;

	clients = new EntClient * [GAME_MAXCLIENTS];
	memset(clients,0, sizeof(EntClient *) * GAME_MAXCLIENTS);
	numClients = 0;
}

CGame::~CGame()
{
	if(entities)
	{
		for(int i=0;i<GAME_MAXENTITIES; i++)
		{
			if(entities[i]) 
				delete entities[i];
		}
		delete [] entities;
	}
	if(clients)
	{
		for(int i=0;i<GAME_MAXCLIENTS; i++)
		{
			if(clients[i]) 
				delete clients[i];
		}
		delete [] clients;
	}
}


bool CGame::InitGame()
{
	EntSpawner::RegisterMakers();
	return true;
}

void CGame::ShutdownGame()
{
	EntSpawner::DestroyMakers();
}

void CGame::RunFrame(float curTime)
{
	//Run entities
	for(int i=0; i<numEnts; i++)
	{
	}

	vector_t desiredDir;
	//Run through clients
	for(i=0; i<numClients; i++)
	{
		if(clients[i]->inUse && clients[i]->spawned)
		{
			//clients[i]->clCmd;
			//CMoveType::ClientMove(clients[clNum]
		}
	}
}

int  CGame::GetVersion()
{
	return 1;
}


//Client funcs
bool CGame::ClientConnect(int clNum, CBuffer &userInfo, bool reconnect)
{
	if(clients[clNum] && !reconnect)
		return false;

	clients[clNum] = new EntClient();

	strcpy(clients[clNum]->name, userInfo.ReadString());
	strcpy(clients[clNum]->modelName,userInfo.ReadString());
	clients[clNum]->mdlIndex = g_pImports->RegisterModel(clients[clNum]->modelName);

	strcpy(clients[clNum]->skinName,userInfo.ReadString());
	clients[clNum]->skinNum = g_pImports->RegisterImage(clients[clNum]->skinName);

	clients[clNum]->inUse = true;
	clients[clNum]->spawned = false;

	clients[clNum]->num = numClients;

	if(!reconnect)
		numClients++;

	g_pImports->BroadcastPrintf("%s connected", clients[clNum]->name);
	return true;
}

void CGame::ClientUpdateUserInfo(int clNum, CBuffer &userInfo)
{
}

void CGame::ClientBegin(int clNum)
{
	g_pImports->BroadcastPrintf("%s entered the game", clients[clNum]->name);
	clients[clNum]->spawned = true;
}

void CGame::ClientDisconnect(int clNum)
{
	if(clients[clNum])
	{
		delete clients[clNum];
		clients[clNum] = 0;
		numClients --;
	}
}

void CGame::ClientCommand(int clNum, CBuffer &command)
{
}



//======================================================================================
//======================================================================================

void ComPrintf(const char * text, ...)
{
	static char buffer[1024];
	
	va_list args;
	va_start(args, text);
	vsprintf(buffer, text, args);
	va_end(args);
	g_pCons->ComPrint(buffer);
}

int HandleOutOfMemory(size_t size)
{	g_pImports->FatalError("Game Dll is out of memory");
	return 0;
}




