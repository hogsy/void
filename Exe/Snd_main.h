#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

#include "Snd_defs.h"
#include "Sys_hdr.h"

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

//======================================================================================
//Main Sound manager
//======================================================================================

class CSoundManager : public I_SoundManager,
					  public I_ConHandler 
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
	void Shutdown();	//Unload all sounds
	
	void RunFrame();

	hSnd RegisterSound(const char * path);
	void UnregisterAll();

	//update pos
	void UpdateListener(const vector_t &pos,
						const vector_t &velocity,
						const vector_t &forward,
						const vector_t &up);

	//hook this up with an entity, for speed and origin
	void Play(hSnd index, int channel= VoidSound::CHAN_AUTO);

	//Console handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::C3DListener   *  m_pListener;
	
	VoidSound::CSoundBuffer  *	m_Buffers;	//All sounds are buffered when registered
	VoidSound::CSoundChannel *	m_Channels;	//Channels which are actually played
	
	int	 m_numBuffers;
//	int	 m_channelsInUse;

	bool m_bHQSupport;
	bool m_bStereoSupport;
	
	CVar m_cVolume;			//Master Volume 
	CVar m_cHighQuality;	//16bit buffer if on.
	CVar m_cRollOffFactor;
	CVar m_cDopplerFactor;
	CVar m_cDistanceFactor;

	bool SVolume(const CParms &parms);

	//==========================================
	//Temp debug funcs
	void SPlay(const char * arg);
	void SStop(int channel);
	void SListSounds();
	bool SPrintInfo();

};

#endif