#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

#include "Sys_hdr.h"
#include "3dmath.h"
#include "Res_defs.h"

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

	CSoundManager();
	~CSoundManager();

	bool Init();
	void Shutdown();

	void RunFrame();

	//finds id, or creates new one, if index = -1, otherwise loads sound at give index
	hSnd RegisterSound(const char *path, CacheType cache, hSnd index = -1);
	void UnregisterSound(hSnd index, CacheType cache);
	void UnregisterCache(CacheType cache);
	void UnregisterAll();
	
	//update pos
	void UpdateListener(const vector_t &pos,
						const vector_t &velocity,
						const vector_t &forward,
						const vector_t &up);

	//hook this up with an entity, for speed and origin
	void PlaySnd(int index, CacheType cache,
				 int channel= CHAN_AUTO,
				 const vector_t * origin=0,
				 const vector_t * velocity=0,
				 bool looping = false);

	//Update Sound position
	//The SoundManager needs to automatically stop sounds out of range
	void UpdateGameSound(int index, vector_t * pos, vector_t * velocity);

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
	void SPlay(const char * arg);
	void SStop(int channel);
	void SListSounds();
	void SPrintInfo();
};

#endif