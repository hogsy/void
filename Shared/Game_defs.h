#ifndef VOID_GAME_DEFS
#define VOID_GAME_DEFS

#include "Com_vector.h"

/*
======================================
Basic "game" definitions
======================================
*/
const int	GAME_MAXMODELS	= 256;
const int	GAME_MAXSOUNDS	= 256;
const int	GAME_MAXIMAGES	= 256;
const int	GAME_MAXENTITIES= 1024;
const int	GAME_MAXCLIENTS = 16;
const char	GAME_WORLDSDIR[]= "Worlds/";

/*
======================================
Only contains data needed by routines 
shared between client and server code
Both client and server subclass this 
adding stuff they need

This data will be propagated to all 
connected clients
======================================
*/
struct BaseEntity
{
	BaseEntity()
	{
		num = -1;
		mdlIndex = -1;
		frameNum = nextFrame = skinNum = 0;
		sndIndex = -1;
		volume = attenuation = 0;
		origin.x = origin.y = origin.z = 0.0f;
		angles.x = angles.y = angles.z = 0.0f;
		velocity.x = velocity.y = velocity.z = 0.0f;
		mins.x = mins.y = mins.z = 0.0f;
		maxs.x = maxs.y = maxs.z = 0.0f;
	}

	virtual ~BaseEntity() =0 {}

	int		num;
	int		mdlIndex, 
			frameNum,	
			nextFrame, 
			skinNum;
	int		sndIndex, 
			volume,	
			attenuation;
	
	vector_t origin;
	vector_t angles;
	vector_t velocity;
	vector_t mins;
	vector_t maxs;
};

/*
======================================
Client sends this to the server as frequently as possible
======================================
*/
struct ClCmd
{
	float	time;
	int		angles[3];
	short	forwardmove, 
			rightmove, 
			upmove;
};

#endif