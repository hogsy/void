#ifndef VOID_SOUND_SYSTEM
#define VOID_SOUND_SYSTEM

#include "Com_vector.h"
#include "Game_defs.h"

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

class  CCamera;
struct ClEntity;

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
		DEFAULT_ATTENUATION = 5,
		MIN_VOLUME = 0,
		MAX_VOLUME = 10,
		MAX_SOUNDSOURCES = 128,
		MAX_WAVEFILES = 512,
		MAX_CHANNELS = 16
	};

	CSoundManager();
	~CSoundManager();

	bool Init();
	void RunFrame();

	//finds id, or creates new one, if index = -1, otherwise loads sound at give index
	int  RegisterSound(const char *path, CacheType cache, int  index = -1);
	void UnregisterSound(CacheType cache, int index);
	void UnregisterCache(CacheType cache);
	void UnregisterAll();

	//Add a static soundSource which will be automatically played when the listener
	//gets in range, and stopped when out of range.. Update func 
	//just tells the system to recalculate vars cause the ent has changed.
	//NOTE: there can only be ONE static Source per entity
	void AddStaticSource(const ClEntity * ent);
	void RemoveStaticSource(const ClEntity * ent);

	//update pos CCamera
	void UpdateListener(const CCamera * pCamera);

	//Play a sound originating from an entity
	//If volume and attenuation are 0 then it uses the ones set to the entitiy
	void PlaySnd3d(const ClEntity * ent,
				   int index, CacheType cache,
				   int volume = DEFAULT_VOLUME, 
				   int attenuation =DEFAULT_ATTENUATION,
				   int chantype = CHAN_AUTO);
	
	//Play a 2d-UI sound at given volume
	void PlaySnd2d(int index, CacheType cache,
				 int volume = DEFAULT_VOLUME,
				 int chantype = CHAN_AUTO);

	//Console handler
	bool HandleCVar(const CVarBase * cvar, const CStringVal &strVal);
	void HandleCommand(int cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::C3DListener   *  m_pListener;
	
	//All sounds are buffered when registered
	VoidSound::CSoundBuffer  *	m_bufferCache[CACHE_NUMCACHES];	
	
	//Channels which are actually played
	VoidSound::CSoundChannel *	m_Channels;			

	struct SndSource;
	SndSource * m_sndSources;

	bool m_bHQSupport;
	bool m_bStereoSupport;

	float	 m_fLastFrame;
	vector_t m_listenerPos;

	CVar m_cVolume;			//Master Volume 
	CVar m_cHighQuality;	//16bit buffer if on.
	CVar m_cRollOffFactor;
	CVar m_cDopplerFactor;
	CVar m_cDistanceFactor;
	CVar m_cSndFps;

	void PlaySoundSource(SndSource &source);


	bool SetVolume(const CStringVal &strVal);
	bool SetRollOffFactor(const CStringVal &strVal);
	bool SetDistanceFactor(const CStringVal &strVal);
	bool SetDopplerFactor(const CStringVal &strVal);

	//==========================================
	//console funcs
	void SPlay(const CParms &parms);
	void SStop(int channel);
	void SListSounds();
	void SPrintInfo();
};

#endif