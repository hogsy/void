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

/*
	min dis/ max dis
	1 =	1/1 
    0.5 = 1/2
*/

	//Cache type,
	//not using this right now. should be the same for all resource managers ?
	enum SndCacheType
	{
		CACHE_EMPTY = 0,		//not loaded, default
		CACHE_TEMP	= 1,		//temp object, manager should release once it has been used
		CACHE_LEVEL	= 2,		//map specifc, should be unloaded after
		CACHE_GAME	= 3,		//persist throughout the game
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