#ifndef VOID_CLIENT_PRIVATE_HEADER
#define VOID_CLIENT_PRIVATE_HEADER

#include "Com_defs.h"
#include "Com_vector.h"

#include "Cl_game.h"
#include "Net_defs.h"
#include "Net_protocol.h"

/*
================================================
This interface is exported to the Client side 
game code. It contains everything the client game
should ever need from the exe

This will have to be updated as new funcs are exported
================================================
*/

struct NetChanState;
struct ClEntity;
class  CBuffer;


enum EClState
{
	CLIENT_DISCONNECTED =0,
	CLIENT_RECONNECTING =1,
	CLIENT_INGAME =2
};


struct I_ClientGame
{
	//Models
	virtual void DrawModel(const ClEntity &state)=0;
	virtual int  RegisterModel(const char *model, CacheType cache, int index=-1)=0;
	virtual void UnregisterModel(CacheType cache,int index)=0;
	virtual void UnregisterModelCache(CacheType cache)=0;

	//Images
	virtual int  RegisterImage(const char *image, CacheType cache, int index=-1)=0;
	virtual void UnregisterImage(CacheType cache, int index)=0;
	virtual void UnregisterImageCache(CacheType cache)=0;

	//Hud
	virtual void HudPrintf(int x, int y, float time, const char *msg,...)=0;

	//Sound
	virtual int  RegisterSound(const char *path, CacheType cache, int index = -1)=0;
	virtual void UnregisterSound(CacheType cache,int index)=0;
	virtual void UnregisterSoundCache(CacheType cache)=0;

	virtual void AddSoundSource(const ClEntity * ent)=0;
	virtual void RemoveSoundSource(const ClEntity * ent)=0;

	virtual void PlaySnd3d(const ClEntity * ent,
				   int index, CacheType cache,int volume = 10, 
				   int attenuation =5,
				   int chantype = CHAN_AUTO)=0;
	
	virtual void PlaySnd2d(int index, CacheType cache,int volume = 10,
				   int chantype = CHAN_AUTO)=0;

	//Client
	virtual void SetClientState(EClState state)=0;
	virtual void SetNetworkRate(int rate)=0;
	
	virtual CWorld * LoadWorld(const char * worldName)=0;
	virtual void UnloadWorld()=0;

	//Networking
	virtual CBuffer & GetSendBuffer()=0;
	virtual CBuffer & GetReliableSendBuffer()=0;

};

#endif