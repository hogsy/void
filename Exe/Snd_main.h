#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

#include "Sys_hdr.h"
#include "3dmath.h"
#include "Game_defs.h"
#include "Clgame_defs.h"

/*
======================================
Private stuff
======================================
*/
namespace VoidSound
{
	class CPrimaryBuffer;	//The primary sound buffer, there can be only one
	class CSoundBuffer;		//A sound buffer wrapping up a wavefile
	class CSoundChannel;	//A sound buffer which actually gets played
	class C3DListener;		//The 3d Sound listener
}

/*
======================================
Main Sound manager
======================================
*/

class CSoundManager : public I_ConHandler 
{
public:

	enum
	{
		DEFAULT_VOLUME = 10,
		MIN_VOLUME = 0,
		MAX_VOLUME = 10,
		MAX_STATICSOURCES = 64,
		MAX_WAVEFILES = 512,
		MAX_CHANNELS = 16
	};

	CSoundManager();
	~CSoundManager();

	bool Init();

	//finds id, or creates new one, if index = -1, otherwise loads sound at give index
	hSnd RegisterSound(const char *path, CacheType cache, hSnd index = -1);
	void UnregisterSound(hSnd index, CacheType cache);
	void UnregisterCache(CacheType cache);
	void UnregisterAll();

	//Run a Sound Frame
	void RunFrame();

	//Add a static soundSource which will be automatically played
	//when the client gets in range, and stopped when out of range.
	void AddStaticSource(const ClEntity * ent);
	void RemoveStaticSource(const ClEntity * ent);
	//just tell system to recalculate vars cause the ent has changed.
	void UpdateStaticSource(const ClEntity * ent);

	//update pos CCamera
	void UpdateListener(const vector_t &pos,
						const vector_t &velocity,
						const vector_t &forward,
						const vector_t &up);

	//Play a sound originating from an entity
	//If volume and attenuation are 0 then it uses the ones set to the entitiy
	void PlaySnd3d(const ClEntity * ent,
				 int index, CacheType cache,
				 int volume = 10, int attenuation =0,
				 int chantype = CHAN_AUTO);
	
	//Play a 2d-UI sound at given volume
	void PlaySnd2d(int index, CacheType cache,
				 int volume = 10,
				 int chantype = CHAN_AUTO);

	//Console handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::C3DListener   *  m_pListener;
	
	//All sounds are buffered when registered
	VoidSound::CSoundBuffer  *	m_bufferCache[RES_NUMCACHES];	
	
	//Channels which are actually played
	VoidSound::CSoundChannel *	m_Channels;			

	//Keep track of static sources


	struct SndSource
	{
		SndSource() { Reset(); }
		void Reset() { channel = -1; ent = 0; }  //muteDist =0.0f; 
		~SndSource() { Reset(); }
		
		int   channel;
		const ClEntity * ent;
	};
	SndSource m_sndSources[MAX_STATICSOURCES];

	void PlayStaticSound(SndSource &source);
	
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
	//console funcs
	void SPlay(const CParms &parms);
	void SStop(int channel);
	void SListSounds();
	void SPrintInfo();
};

#endif