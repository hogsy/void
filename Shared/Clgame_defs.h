#ifndef VOID_CLGAME_DEFS
#define VOID_CLGAME_DEFS

#include "3dmath.h"
#include "I_renderer.h"

//A client side entitiy
struct ClEntity : public R_EntState
{
	ClEntity()
	{	Reset();
	}

	virtual void Reset()
	{
		num_skins = num_frames = 0;
		index = -1;
		skinnum = 0;
		frame = nextframe = 0;
		
		inUse = false;

		Void3d::VectorSet(origin,0,0,0);
		Void3d::VectorSet(angle,0,0,0);
	}

	virtual ~ClEntity() {}

	bool inUse;

	int			soundIndex;
	CacheType	sndCache;

	int	volume;
	int	attenuation;
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