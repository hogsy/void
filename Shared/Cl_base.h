#ifndef VOID_CLIENT_BASE_ENT
#define VOID_CLIENT_BASE_ENT

#include "Game_base.h"

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

		origin.Set(0,0,0);
		angles.Set(0,0,0);
		velocity.Set(0,0,0);
		mins.Set(0,0,0);
		maxs.Set(0,0,0);
	
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
	{	Reset();
	}

	virtual void Reset()
	{
		ClEntity::Reset();
		memset(name,0,32);
		gravity = friction = maxSpeed = 0.0f;
	}
	
	char name[CL_MAXNAME];

	//for local prediction
	float gravity;
	float friction;
	float maxSpeed;
};


#endif