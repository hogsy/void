#ifndef VOID_CLIENT_DEFS
#define VOID_CLIENT_DEFS

#include "Game_defs.h"

/*
============================================================================
Basic client side definitions 
There are needed by the client resource managers like
the Renderer and Sound system etc
============================================================================
*/
enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};

const int CACHE_NUMCACHES = 3;

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

/*
======================================
Entity info the renderer needs to know to renderer it in the game.
the Soundsystem needs to know this to figure out positions etc
Visibility determination should be done client side ?
======================================
*/
struct ClEntity : public BaseEntity
{
	ClEntity()
	{	Reset();
	}

	virtual void Reset()
	{
		mdlCache = CACHE_LOCAL;
		sndCache = CACHE_LOCAL;
		frac = 0.0f;
		numSkins = numFrames = 0;
		inUse = false;

		Void3d::VectorSet(origin,0,0,0);
		Void3d::VectorSet(angles,0,0,0);
		Void3d::VectorSet(velocity,0,0,0);
		Void3d::VectorSet(mins,0,0,0);
		Void3d::VectorSet(maxs,0,0,0);

		mdlIndex = sndIndex = -1;
		num = -1;
		frameNum = nextFrame = skinNum = 0;
		volume = attenuation = 0;
	}

	virtual ~ClEntity() {}

	bool inUse;

	CacheType	sndCache;
	CacheType	mdlCache;

	int			numSkins;
	int			numFrames;
	float		frac;
};

//A client side Client
struct ClClient : public ClEntity
{
	ClClient() 
	{	name[0] = 0;
	}

	virtual void Reset()
	{
		ClEntity::Reset();
		name[0] = 0;
	}
	char name[32];
};

#endif