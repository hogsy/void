#include "Game_hdr.h"
#include "Game_main.h"
#include "Game_spawn.h"
#include "Com_buffer.h"

/*
================================================
Constructor/Destructor
================================================
*/
CGame::CGame()
{
	entities = new Entity * [GAME_MAXENTITIES];
	memset(entities, 0, sizeof(Entity *) * GAME_MAXENTITIES);
	maxEnts = GAME_MAXENTITIES;
	numEnts = 0;

	clients = new EntClient * [GAME_MAXCLIENTS];
	memset(clients,0, sizeof(EntClient *) * GAME_MAXCLIENTS);
	numClients = 0;

	InitializeVars();
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

/*
================================================
Initalize/Destroy the Game
================================================
*/
bool CGame::InitGame()
{	EntSpawner::RegisterMakers();
	return true;
}

void CGame::ShutdownGame()
{	EntSpawner::DestroyMakers();
}

/*
================================================
Load/Unload World
================================================
*/
bool CGame::LoadWorld(I_World * pWorld)
{	EntMove::SetWorld(pWorld);
	return true;
}

void CGame::UnloadWorld()
{	EntMove::SetWorld(0);
}


/*
================================================
Spawn entities from parms
================================================
*/
bool CGame::SpawnEntity(CBuffer &buf)
{
	buf.BeginRead();
	char * classname = buf.ReadString();

	Entity * ent = EntSpawner::CreateEntity(classname,buf);
	if(ent)
	{
ComPrintf("GAME: %s at (%.2f,%.2f,%.2f)\n", ent->classname, ent->origin.x, ent->origin.y, ent->origin.z);
		entities[numEnts] = ent;
		entities[numEnts]->num = numEnts;
		numEnts++;
		return true;
	}
	return false;
}


//==========================================================================
//The Game frame
//==========================================================================

void CGame::RunFrame(float curTime)
{
	//Run entities
	for(int i=0; i<numEnts; i++)
	{
	}

	//Run client movement

	vector_t desiredMove;	
	vector_t forward, right, up;

	for(i=0; i<numClients; i++)
	{
		//Only move the client if we got new input from him  ?
		//So if the client is lagged, he will be stuck on the server, and
		//jump back to his original position when his connection unclogs
		if(clients[i]->clCmd.svFlags & ClCmd::UPDATED)
		{
			clients[i]->angles = clients[i]->clCmd.angles;
			clients[i]->angles.AngleToVector(&forward,&right,&up);
			up.Normalize();

			// find forward and right vectors that lie in the xy plane
			forward.z = 0;
			if(forward.Normalize() < 0.3f)
			{
				if(forward.z < 0)
					forward = up;
				else
					up.Inverse();
				forward.z = 0;
				forward.Normalize();
			}

			right.z = 0;
			if(right.Normalize() > 0.3f)
			{
				if(right.z > 0)
					right = up;
				else
					up.Inverse();
				right.z = 0;
				right.Normalize();
			}

			//Get desired move
			desiredMove.Set(0,0,0);

			if (clients[i]->clCmd.moveFlags & ClCmd::MOVEFORWARD)
				desiredMove.VectorMA(desiredMove,g_varMaxSpeed.fval, forward);
			if (clients[i]->clCmd.moveFlags & ClCmd::MOVEBACK)
				desiredMove.VectorMA(desiredMove,-g_varMaxSpeed.fval, forward);
			if (clients[i]->clCmd.moveFlags & ClCmd::MOVERIGHT)
				desiredMove.VectorMA(desiredMove,g_varMaxSpeed.fval, right);
			if (clients[i]->clCmd.moveFlags & ClCmd::MOVELEFT)
				desiredMove.VectorMA(desiredMove,-g_varMaxSpeed.fval, right);
			if (clients[i]->clCmd.moveFlags & ClCmd::JUMP)
				desiredMove.z += 400;

			// always add gravity
			desiredMove.z -= g_varGravity.fval * GAME_FRAMETIME;

			//gradually slow down existing velocity (friction)
			clients[i]->velocity.x *= g_varFriction.fval * GAME_FRAMETIME;
			clients[i]->velocity.y *= g_varFriction.fval * GAME_FRAMETIME;
			
			if (clients[i]->velocity.x < 0.01f)
				clients[i]->velocity.x = 0;
			if (clients[i]->velocity.y < 0.01f)
				clients[i]->velocity.y = 0;

			//Add velocity created by new input
			clients[i]->velocity += desiredMove;

			//Perform the actual move and update angles
			EntMove::ClientMove(clients[i], GAME_FRAMETIME);

			//Do we really want to do this ?
			clients[i]->clCmd.Reset();
		}
	}
}


//==========================================================================
//Client funcs
//==========================================================================


/*
================================================
Called on connection/disconnection
================================================
*/
bool CGame::ClientConnect(int clNum, CBuffer &userInfo, bool reconnect)
{
	//Make sure we have free slot
	if(clients[clNum] && !reconnect)
		return false;

	clients[clNum] = new EntClient();

	strcpy(clients[clNum]->name, userInfo.ReadString());
	strcpy(clients[clNum]->modelName,userInfo.ReadString());
	strcpy(clients[clNum]->skinName,userInfo.ReadString());

	clients[clNum]->mdlIndex = g_pImports->RegisterModel(clients[clNum]->modelName);
	clients[clNum]->skinNum = g_pImports->RegisterImage(clients[clNum]->skinName);

	clients[clNum]->inUse = true;
	clients[clNum]->spawned = false;
	clients[clNum]->clCmd.Reset();

	clients[clNum]->num = numClients;

	if(!reconnect)
		numClients++;

	g_pImports->BroadcastPrintf("%s connected", clients[clNum]->name);
	return true;
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

/*
================================================
Put a client into a game for the first time
called on initial connection and level changes
================================================
*/
void CGame::ClientBegin(int clNum)
{
	clients[clNum]->clCmd.Reset();
	VectorSet(&clients[clNum]->origin, 0.0f,0.0f,48.0f);
	VectorSet(&clients[clNum]->mins, -10.0f, -10.0f, -40.0f);
	VectorSet(&clients[clNum]->maxs, 10.0f, 10.0f, 10.0f);
	VectorSet(&clients[clNum]->angles, 0.0f, 0.0f, 0.0f);
	VectorSet(&clients[clNum]->velocity, 0.0f, 0.0f, 0.0f);

	g_pImports->BroadcastPrintf("%s entered the game", clients[clNum]->name);
	clients[clNum]->spawned = true;
}

/*
================================================
Client wants to change userInfo
================================================
*/
void CGame::ClientUpdateUserInfo(int clNum, CBuffer &userInfo)
{
}

/*
================================================
Process a client string command
================================================
*/
void CGame::ClientCommand(int clNum, CBuffer &command)
{
}