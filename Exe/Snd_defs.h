#ifndef VOID_SOUND_DEFS
#define VOID_SOUND_DEFS


#include "Com_defs.h"

//This is all the game code needs to see

namespace VoidSound
{

	//What channel to play a sound in
	enum SndChannelType
	{
		CHAN_STREAM = 0,
		CHAN_WORLD  = 1,		//ambient, world sounds etc
		CHAN_ITEM   = 2,		//item noises, pickups etc
		CHAN_WEAPON	= 3,		//weapon noises
		CHAN_PLAYER = 4,		//player sounds
		CHAN_AUTO	= 5			//first freely available
	};


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
	virtual void Play(hSnd index, int channel=VoidSound::CHAN_AUTO)=0;
};


#endif