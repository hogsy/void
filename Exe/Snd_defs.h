#ifndef VOID_SOUND_DEFS
#define VOID_SOUND_DEFS


#include "Com_defs.h"
#include "3dmath.h"

//This is all the game code needs to see
namespace VoidSound
{
	//What channel to play a sound in
	enum SndChannelType
	{
		CHAN_AUTO   = 0,		//first free
		CHAN_WORLD  = 1,		//ambient, world sounds etc
		CHAN_ITEM   = 2,		//item noises, pickups etc
		CHAN_WEAPON	= 3,		//weapon noises
		CHAN_PLAYER = 4			//player sounds
	};

	//Cache type,
	//not using this right now. should be the same for all resource managers ?
	enum CacheType
	{
		CACHE_EMPTY = 0,	//Default
		CACHE_LOCAL	= 1,	//Persistant through out the game, Client is reponsible for loading these.

		//Server is responsible for giving an index starting containing all files here

		CACHE_GAME	= 2,	//Map specific, should be unloaded on map change
		CACHE_TEMP	= 4,	//Temp object,  should release once it has been used
	};
}

//Handle to sound
typedef int hSnd;

struct  I_SoundManager
{
	virtual hSnd RegisterSound(const char * path) =0;
	virtual void UnregisterAll() =0;

	//Setup listener

	//hook this up with an entity, for speed and origin
	virtual void PlaySnd(hSnd index, int channel= VoidSound::CHAN_AUTO,
							   const vector_t * origin=0,
							   const vector_t * velocity=0,
							   bool looping = false)=0;
};

#endif