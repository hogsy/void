#ifndef VOID_GAME_DEFS
#define VOID_GAME_DEFS

/*
======================================
Basic "game" definitions.
======================================
*/

const int	GAME_MAXMODELS	= 256;
const int	GAME_MAXSOUNDS	= 256;
const int	GAME_MAXIMAGES	= 256;
const int	GAME_MAXENTITIES= 1024;
const int	GAME_MAXCLIENTS = 16;
const char	GAME_WORLDSDIR[]= "Worlds/";

const char  CL_MAXNAME		= 32;
const char  CL_MAXMODELNAME = 32;
const char  CL_MAXCHARNAME  = 64;

/*
================================================
Move types
================================================
*/
enum EMoveType
{
	MOVETYPE_NOCLIP,
	MOVETYPE_BBOX,		//just a static bbox
	MOVETYPE_MISSLE,	
	MOVETYPE_TRAJECTORY,
	MOVETYPE_STEP
};

/*
================================================
Basic client side definitions 
There are needed by the client 
resource managers like
the Renderer and Sound system etc
================================================
*/
enum CacheType
{
	CACHE_LOCAL = 0,	//Persistant through out the game, Client is reponsible for loading these.
	CACHE_GAME	= 1,	//Map specific, should be unloaded on map change
	CACHE_TEMP	= 2,	//Temp object,  should release once it has been used
};
const int CACHE_NUMCACHES = 3;

/*
================================================
================================================
*/
enum
{
	MODEL_SKIN_BOUND = 0,
	MODEL_SKIN_UNBOUND_GAME  = 0X80000000,
	MODEL_SKIN_UNBOUND_LOCAL = 0X40000000
};


/*
================================================
Different sound channel types
================================================
*/
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

#endif