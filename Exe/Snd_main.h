#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

#include "Sys_hdr.h"
#include "3dmath.h"
#include "Res_defs.h"

//What channel to play a sound in
enum SndChannelType
{
	CHAN_AUTO   = 0,		//first free
	CHAN_WORLD  = 1,		//ambient, world sounds etc
	CHAN_ITEM   = 2,		//item noises, pickups etc
	CHAN_WEAPON	= 3,		//weapon noises
	CHAN_PLAYER = 4			//player sounds
};

//======================================================================================
//Private stuff
//======================================================================================
namespace VoidSound
{
	class CPrimaryBuffer;	//The primary sound buffer, there can be only one
	class CSoundBuffer;		//A sound buffer wrapping up a wavefile
	class CSoundChannel;	//A sound buffer which actually gets played
	class C3DListener;		//The 3d Sound listener
}

/*
======================================================================================
Main Sound manager
play a sound at a given channel and pos, looping or not looping
======================================================================================
*/
class CSoundManager : public I_ConHandler 
{
public:

	enum
	{
		MAX_SOUNDS   = 256,
		MAX_CHANNELS = 16
	};

	CSoundManager();
	~CSoundManager();

	bool Init();
	void Shutdown();

	void RunFrame();

	int RegisterSound(const char * path);
	void UnregisterAll();

	//update pos
	void UpdateListener(const vector_t &pos,
						const vector_t &velocity,
						const vector_t &forward,
						const vector_t &up);

	//hook this up with an entity, for speed and origin
	void PlaySnd(int index, int channel= CHAN_AUTO,
							   const vector_t * origin=0,
							   const vector_t * velocity=0,
							   bool looping = false);
	//Update Sound position
	//The SoundManager needs to automatically stop sounds out of range
	void UpdateSnd(int index, vector_t * pos, vector_t * velocity);

	//Console handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::C3DListener   *  m_pListener;
	
	VoidSound::CSoundBuffer  *	m_Buffers;	//All sounds are buffered when registered
	VoidSound::CSoundChannel *	m_Channels;	//Channels which are actually played
	
	int	 m_numBuffers;
	bool m_bHQSupport;
	bool m_bStereoSupport;
	
	CVar m_cVolume;			//Master Volume 
	CVar m_cHighQuality;	//16bit buffer if on.
	CVar m_cRollOffFactor;
	CVar m_cDopplerFactor;
	CVar m_cDistanceFactor;

	bool SetVolume(const CParms &parms);
	bool SetRollOffFactor(const CParms &parms);
	bool SetDistanceFactor(const CParms &parms);
	bool SetDopplerFactor(const CParms &parms);

	//==========================================
	//Temp debug funcs
	void SPlay(const char * arg);
	void SStop(int channel);
	void SListSounds();
	bool SPrintInfo();

};

#endif