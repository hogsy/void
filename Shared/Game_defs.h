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
	CHAN_UI		= 1,		//UI sound, no coordinates
	CHAN_WORLD  = 2,		//ambient, world sounds etc
	CHAN_CLIENT = 3,		//sounds from the player
	CHAN_MONSTER= 4,		//Monster and player share channels
	CHAN_PLAYER = 4,
	CHAN_ITEM   = 5,		//item noises, pickups etc
	CHAN_WEAPON	= 6,		//weapon noises
	
	CHAN_LOOPING = 128		//flagged
};


//Sound attenuation



//Cache type,
enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};


#endif