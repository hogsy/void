#ifndef VOID_CLIENT_BASE_ENT
#define VOID_CLIENT_BASE_ENT

#include "Game_defs.h"

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
		inUse = false;

		origin.Set(0,0,0);
		angles.Set(0,0,0);
		velocity.Set(0,0,0);
		mins.Set(0,0,0);
		maxs.Set(0,0,0);

		animInfo.Set(0,0);
	
		mdlIndex = -1;
		sndIndex = -1;
		skinNum = 0;
		num = -1;
		volume = attenuation = 0;
	}

	virtual ~ClEntity() {}

	bool inUse;

	AnimState	animInfo;
	
	CacheType	sndCache;
	CacheType	mdlCache;
};

/*
================================================
A client side Client
================================================
*/
struct ClClient : public ClEntity
{
	ClClient() 
	{	Reset();
	}

	virtual void Reset()
	{
		ClEntity::Reset();

		clAnim = 0;

		memset(name,0,CL_MAXNAME);
		memset(model,0,CL_MAXMODELNAME);
		gravity = friction = maxSpeed = 0.0f;
	}
	
	char name[CL_MAXNAME];
	//fix me, changed to pointer to list of loaded char resources
	char model[CL_MAXMODELNAME];
	
	int   clAnim;

	//for local prediction
	float gravity;
	float friction;
	float maxSpeed;
};

#endif