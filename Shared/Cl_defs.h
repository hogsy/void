#ifndef VOID_CLIENT_DEFS
#define VOID_CLIENT_DEFS
/*
============================================================================
Basic definitions shared by the client resource managers
include the Renderer, Sound system and any clientside dll
============================================================================
*/
#include "Com_vector.h"

typedef int hSnd;
typedef int hMdl;
typedef int hImg;

//Cache type,
enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};

//These should be updated, 
//if the games maxmodels/sounds/images vars change

const int CACHE_NUMMODELS = 256;
const int CACHE_NUMIMAGES = 256;
const int CACHE_NUMSOUNDS = 256;
const int CACHE_NUMCACHES = 3;

/*
======================================
Entity info the renderer needs to know to
renderer it in the game.
Visibility determination should be done client side ?
======================================
*/
struct EntState 
{
	EntState()
	{
		mdlCache = CACHE_LOCAL;
		mdlIndex = -1;
		numSkins = skinNum = 0;
		numFrames = frame = nextFrame = 0;
		frac = 0.0f;
	}

	CacheType	mdlCache;
	hMdl		mdlIndex;
	
	int			numSkins;
	int			skinNum;
	
	int			numFrames;
	int			frame;
	int			nextFrame;
	float		frac;

	vector_t	origin;
	vector_t	angle;
};

#endif