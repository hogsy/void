#ifndef VOID_GAME_DEFS
#define VOID_GAME_DEFS

/*
======================================
Common definitions shared by the resource managers
Header will also be included by the game dlls
======================================
*/

typedef int hSnd;
typedef int hMdl;
typedef int hImg;

const int GAME_MAXMODELS	= 256;
const int GAME_MAXSOUNDS	= 256;
const int GAME_MAXIMAGES	= 256;
const int GAME_MAXENTITIES	= 1024;

const int RES_NUMCACHES = 3;

enum SndChannelType
{
	CHAN_AUTO   = 0,		//first free
	CHAN_WORLD  = 1,		//ambient, world sounds etc
	CHAN_ITEM   = 2,		//item noises, pickups etc
	CHAN_WEAPON	= 3,		//weapon noises
	CHAN_PLAYER = 4			//player sounds
};

//Cache type,
enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};


#endif