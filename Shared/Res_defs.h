#ifndef VOID_RESOURCE_DEFS
#define VOID_RESOURCE_DEFS


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


#endif