#ifndef VOID_GAME_INTERFACE
#define VOID_GAME_INTERFACE

#include "Com_defs.h"
#include "Com_buffer.h"
#include "Com_vector.h"
#include "Net_defs.h"


#if 0


enum
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


const int ENT_MAXCLASSNAME = 32



/*
======================================
interface exported by the Exe
======================================
*/
struct I_GameHandler
{
	virtual void BroadcastPrint(const char * msg)=0;
	virtual void ClientPrint(const char * msg)=0;

	virtual NetChanWriter & GetNetChanWriter() const;

	virtual void DebugPrint(const char * msg)=0;
	virtual void FatalError(const char * msg)=0;

	virtual void PlaySnd(const Entity &ent, int index, int channel, float vol, float atten);
	virtual void PlaySnd(vector_t &origin,  int index, int channel, float vol, float atten);

	virtual void ExecCommand(const char * cmd)=0;

	virtual int  RegisterModel(const char * model)=0;
	virtual int  RegisterSound(const char * image)=0;
	virtual int  RegisterImage(const char * sound)=0;
};


/*
======================================
The base game entitiy. 
======================================
*/
struct GameEnt
{
	int			num;
	char		classname[ENT_MAXCLASSNAME];
	vector_t	origin;
	vector_t	angles;
};

/*
======================================
Client Entity on the server
======================================
*/
struct GameClient : public GameEnt
{
	bool inUse;
	char name[32];

	vector_t mins;
	vector_t maxs;
};

struct GameEnts
{
	GameEnt ** gameEnts;
	int maxEnts;
	int numEnts;
};


/*
======================================
Interface exported by the game dll
======================================
*/
//implemented by the game dll
struct I_GameServer
{
	virtual void InitGame()=0;
	virtual void ShutdownGame()=0;

	virtual int  GetVersion()=0;
	virtual GameEnts * GetEnts()=0;

	virtual void RunFrame()=0;

	virtual bool SpawnEntity(CBuffer &buf)=0;

	//Client funcs
	virtual bool ClientConnect(CBuffer &userInfo, bool reconnect);
	virtual void ClientUpdateUserInfo(CBuffer &userInfo);
	virtual void ClientBegin();
	virtual void ClientDisconnect();
};


#endif

#endif