#include "Game_hdr.h"
#include "Game_main.h"
#include "Game_spawn.h"
#include "Game_anims.h"
#include "Com_buffer.h"
#include "Com_trace.h"

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

	m_pWorld =	0;
	InitializeVars();
}

CGame::~CGame()
{
	I_Console::GetConsole()->UnregisterHandler(this);

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
{
	EntMove::SetWorld(pWorld);
	m_pWorld = pWorld;
	return true;
}

void CGame::UnloadWorld()
{
	//Free the the non-persistant entities
	if(entities)
	{
		for(int i=0;i<GAME_MAXENTITIES; i++)
		{
			if(entities[i]) 
				delete entities[i];
			entities[i] = 0;
		}
	}
	numEnts = 0;

	EntMove::SetWorld(0);
	m_pWorld =	0;
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
ComPrintf("GAME: %s : %s\n", ent->classname, ent->origin.ToString());
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
	EntClient * pClient = 0;

	EPlayerAnim clAnim= PLAYER_NONE;

	for(i=0; i<numClients; i++)
	{
		/*
		FIX ME: Server side movement needs to be redesigned.

		If the server will be running at a fixed frame-rate then the server
		will need to know how many times between each server frame did a client attempt
		to move in a given direction and the clients average frame rate.
		The current system will move a client in a big "GAME_FRAMETIME" chunk even if
		the client had pressed move for a single client frame.
		*/

#if 0

		if(clients[i]->clCmd.svFlags & ClCmd::UPDATED)
		{
			pClient = clients[i];

			forward.Set(0,0,0);
			right.Set(0,0,0);
			up.Set(0,0,0);
			desiredMove.Set(0,0,0);

			pClient->angles = pClient->clCmd.angles;
			pClient->angles.AngleToVector(&forward,&right,&up);
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
			
			if (pClient->clCmd.moveFlags & ClCmd::MOVEFORWARD)
				desiredMove.VectorMA(desiredMove, (pClient->maxSpeed), forward);
			if (pClient->clCmd.moveFlags & ClCmd::MOVEBACK)
				desiredMove.VectorMA(desiredMove, -(pClient->maxSpeed), forward);
			if (pClient->clCmd.moveFlags & ClCmd::MOVERIGHT)
				desiredMove.VectorMA(desiredMove, (pClient->maxSpeed), right);
			if (pClient->clCmd.moveFlags & ClCmd::MOVELEFT)
				desiredMove.VectorMA(desiredMove, -(pClient->maxSpeed), right);

			clAnim = PLAYER_RUN;
			
			//Check for touching ground
			TraceInfo traceInfo;
			vector_t  end(pClient->origin);
			end.z -= 0.1f;

			m_pWorld->Trace(traceInfo,pClient->origin, end, pClient->mins,pClient->maxs);
			if(traceInfo.fraction != 1)
			{
				clAnim = PLAYER_JUMP;

				if (pClient->clCmd.moveFlags & ClCmd::JUMP)
				{	desiredMove.z += 300;
				}
			}

			float moveTime = 10 * pClient->clCmd.time;
			
			// always add gravity
			desiredMove.z -= (pClient->gravity * moveTime);

			//gradually slow down existing velocity (friction)
			pClient->velocity.x *= (pClient->friction  * moveTime);
			pClient->velocity.y *= (pClient->friction  * moveTime);
		
			if (pClient->velocity.x < 0.01f)
				pClient->velocity.x = 0;
			if (pClient->velocity.y < 0.01f)
				pClient->velocity.y = 0;

			if(pClient->velocity.x == 0 &&
			   pClient->velocity.y == 0 &&
			   pClient->velocity.z == 0)
				clAnim = PLAYER_STAND;

			if(clAnim != pClient->animSeq)
			{
				pClient->animSeq = clAnim;
				pClient->sendFlags |= SVU_ANIMSEQ;
			}
					
			//Add velocity created by new input
			pClient->velocity += desiredMove;

			//Perform the actual move and update angles
			EntMove::ClientMove(pClient, moveTime);

			//Do we really want to do this ?
			pClient->clCmd.Reset();
		}
#endif
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
	clients[clNum]->bSpawned = false;
	clients[clNum]->num = numClients;
	strcpy(clients[clNum]->name, userInfo.ReadString());
	strcpy(clients[clNum]->charPath, userInfo.ReadString());

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
	
	clients[clNum]->mins = VEC_CLIENT_MINS;
	clients[clNum]->maxs = VEC_CLIENT_MAXS;

	clients[clNum]->origin.Set(0.0f,0.0f,48.0f);
	clients[clNum]->angles.Set(0.0f, 0.0f, 0.0f);
	clients[clNum]->velocity.Set(0.0f, 0.0f, 0.0f);

	//Set any custom speed/gravity/friction here
	
	//First time spawn
	clients[clNum]->maxSpeed = g_varMaxSpeed.fval;
	clients[clNum]->gravity = g_varGravity.fval;
	clients[clNum]->friction = g_varFriction.fval;
	clients[clNum]->sendFlags |= (SVU_GRAVITY|SVU_FRICTION|SVU_MAXSPEED);

	g_pImports->BroadcastPrintf("%s entered the game", clients[clNum]->name);
	clients[clNum]->bSpawned = true;
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