#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

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
	void UnregisterSound(int index, CacheType cache);
	void UnregisterCache(CacheType cache);
	void UnregisterAll();

	//Add a static soundSource which will be automatically played when the listener
	//gets in range, and stopped when out of range.. Update func 
	//just tells the system to recalculate vars cause the ent has changed.
	//NOTE: there can only be ONE static Source per entity
	void AddStaticSource(const ClEntity * ent);
	void RemoveStaticSource(const ClEntity * ent);
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
				   int volume = DEFAULT_VOLUME, 
				   int attenuation =DEFAULT_ATTENUATION,
				   int chantype = CHAN_AUTO);
	
	//Play a 2d-UI sound at given volume
	void PlaySnd2d(int index, CacheType cache,
				 int volume = DEFAULT_VOLUME,
				 int chantype = CHAN_AUTO);

	//Console handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::C3DListener   *  m_pListener;
	
	//All sounds are buffered when registered
	VoidSound::CSoundBuffer  *	m_bufferCache[CACHE_NUMCACHES];	
	
	//Channels which are actually played
	VoidSound::CSoundChannel *	m_Channels;			

	/*
	======================================
	A Sound Source
	======================================
	*/
	struct SndSource
	{
		SndSource() { Reset(); }
		~SndSource() { Reset(); }

		void Reset() { bStatic = false; 
					   channel = 0; 
					   ent = 0; 
					   flags = 0;
					   muteDist =0.0f; }
		int   flags;
		float muteDist;
		const ClEntity * ent;
		bool  bStatic;
		VoidSound::CSoundChannel * channel;
	};
	
	SndSource m_sndSources[MAX_SOUNDSOURCES];

	void PlaySoundSource(SndSource &source);
	
	bool m_bHQSupport;
	bool m_bStereoSupport;

	float	 m_fLastFrame;
	vector_t m_listenerPos;		//HACK ?

	CVar m_cVolume;			//Master Volume 
	CVar m_cHighQuality;	//16bit buffer if on.
	CVar m_cRollOffFactor;
	CVar m_cDopplerFactor;
	CVar m_cDistanceFactor;
	CVar m_cSndFps;

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