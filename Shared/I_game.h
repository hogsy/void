#ifndef VOID_GAME_INTERFACE
#define VOID_GAME_INTERFACE

#include "Game_defs.h"

struct I_Console;
class  CBuffer;


const int ENT_MAXCLASSNAME = 32;
const int ENT_MAXRESNAME = 64;
const int ENT_MAXSTRING = 128;
const int ENT_MAXMESSAGE = 256;

/*
=======================================
The Server-side game entitiy. 
=======================================
*/
struct Entity : public BaseEntity
{
	Entity(const char * cname)
	{	strcpy(classname,cname);
	}
	
	virtual ~Entity() {}

	char	classname[ENT_MAXCLASSNAME];
};

/*
======================================
Client Entity on the server
======================================
*/
struct EntClient : public Entity
{
	EntClient() :Entity("client")
	{	
		memset(name,0,ENT_MAXCLASSNAME); 
		memset(modelName, 0, ENT_MAXRESNAME);
		memset(skinName, 0, ENT_MAXRESNAME);
		inUse = false; 
		spawned = false;
	}

	bool inUse;
	bool spawned;

	//Server will update this as it gets updates from the client
	ClCmd clCmd;

	char name[ENT_MAXCLASSNAME];
	char modelName[ENT_MAXRESNAME];
	char skinName[ENT_MAXRESNAME];
};

/*
======================================
Multicast types
======================================
*/
enum MultiCastType
{
	MULTICAST_NONE,
	MULTICAST_ALL,
	MULTICAST_PVS,
	MULTICAST_PHS,
	
	//all except the given chanID
	MULTICAST_ALL_X,	
	MULTICAST_PVS_X,
	MULTICAST_PHS_X
};

/*
======================================
interface exported by the main server
class to game dlls
======================================
*/
struct I_GameHandler
{
	//Interface to World
	virtual I_World * GetWorld()=0;

	virtual void BroadcastPrintf(const char * msg,...)=0;
	virtual void ClientPrintf(int clNum, const char * msg,...)=0;

	virtual NetChanWriter & GetNetChanWriter() =0;
	virtual void GetMultiCastSet(MultiCastSet &set, MultiCastType type, int clId)=0;
	virtual void GetMultiCastSet(MultiCastSet &set, MultiCastType type, const vector_t &source)=0;

	virtual void DebugPrint(const char * msg)=0;
	virtual void FatalError(const char * msg)=0;

	virtual void PlaySnd(const Entity &ent, int index, int channel, float vol, float atten) =0;
	virtual void PlaySnd(vector_t &origin,  int index, int channel, float vol, float atten) =0;

	virtual void ExecCommand(const char * cmd)=0;

	virtual int  RegisterModel(const char * model)=0;
	virtual int  RegisterSound(const char * image)=0;
	virtual int  RegisterImage(const char * sound)=0;
};


/*
======================================
Interface exported by the game dll
======================================
*/
//implemented by the game dll
struct I_Game
{
	virtual bool InitGame()=0;
	virtual void ShutdownGame()=0;
	virtual int  GetVersion()=0;

	virtual void RunFrame(float curTime)=0;

	virtual bool SpawnEntity(CBuffer &buf)=0;

	//Client funcs
	virtual bool ClientConnect(int clNum, CBuffer &userInfo, bool reconnect)=0;
	virtual void ClientUpdateUserInfo(int clNum, CBuffer &userInfo)=0;
	virtual void ClientBegin(int clNum)=0;
	virtual void ClientDisconnect(int clNum)=0;
	virtual void ClientCommand(int clNum, CBuffer &command)=0;

	Entity    ** entities;
	EntClient ** clients;

	int maxEnts;
	int numEnts;
	int numClients;
};

/*
======================================
Exported funcs
======================================
*/
extern "C"
{
	I_Game * GAME_GetAPI(I_GameHandler * pImport, I_Console * pConsole);
	void GAME_Shutdown();
}

#endif
