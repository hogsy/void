#ifndef VOID_CLIENT_GAME
#define VOID_CLIENT_GAME

#include "Cl_defs.h"
#include "Game_defs.h"

enum SndChannelType
{
	CHAN_AUTO   = 0,		//first free
	CHAN_UI		= 1,		//UI sound, no coordinates
	CHAN_WORLD  = 2,		//ambient, world sounds etc
	CHAN_CLIENT = 3,		//sounds from the player
	CHAN_MONSTER= 4,		//Monster and player share channels
	CHAN_PLAYER = 4,
	CHAN_ITEM   = 5,		//item noises, pickups etc
	CHAN_WEAPON	= 6,		//weapon noises
	
	CHAN_LOOPING = 128		//flagged
};

//A client side entitiy
struct ClEntity : public EntState
{
	ClEntity()
	{	Reset();
	}

	virtual void Reset()
	{
		num = -1;
		numSkins = numFrames = 0;
		mdlIndex = -1;
		skinNum = 0;
		frame = nextFrame = 0;
		
		inUse = false;

		Void3d::VectorSet(origin,0,0,0);
		Void3d::VectorSet(angle,0,0,0);
	}

	virtual ~ClEntity() {}
	
	int	 num;
	bool inUse;

	int			soundIndex;
	CacheType	sndCache;
	int			volume;
	int			attenuation;
};

//A client side Client
struct ClClient : public ClEntity
{
	ClClient() 
	{	
		memset(name,0,32);
		Void3d::VectorSet(mins,0,0,0);
		Void3d::VectorSet(maxs,0,0,0);
	}

	virtual void Reset()
	{
		ClEntity::Reset();
		memset(name,0,32);
		Void3d::VectorSet(mins,0,0,0);
		Void3d::VectorSet(maxs,0,0,0);
	}

	char name[32];

	vector_t mins;
	vector_t maxs;
};


#endif