#ifndef VOID_SOUND_INTERFACE
#define VOID_SOUND_INTERFACE

#include "Snd_defs.h"
#include "Sys_hdr.h"

//======================================================================================
//======================================================================================


namespace VoidSound
{
	class CPrimaryBuffer;	//The primary sound buffer, there can be only one
	class CSoundBuffer;		//A sound buffer wrapping up a wavefile
	class CSoundChannel;	//A sound buffer which actually gets played
}

//======================================================================================
//======================================================================================

//This is what is exposed to Sys_main
class CSoundManager : public I_SoundManager,
					  public I_CVarHandler,
					  public I_CmdHandler
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

	//hook this up with an entity, for speed and origin
	void Play(hSnd index, int channel= VoidSound::CHAN_AUTO);

	//CVar Handler
	bool HandleCVar(const CVarBase * cvar, const CParms &parms);

	//Cmd Handler
	void HandleCommand(HCMD cmdId, const CParms &parms);

private:

	VoidSound::CPrimaryBuffer*  m_pPrimary;
	VoidSound::CSoundBuffer  *	m_Buffers;	//All sounds are buffered when registered
	VoidSound::CSoundChannel *	m_Channels;	//Channels which are actually played

	int	 m_numBuffers;
//	int	 m_channelsInUse;

	bool m_bHQSupport;
	bool m_bStereoSupport;
	
	CVar m_cVolume;			//Master Volume 
	CVar m_cHighQuality;	//16bit buffer if on.

	bool SVolume(const CParms &parms);

	//==========================================
	//Temp debug funcs
	
	void SPlay(const char * arg);
	void SStop(int channel);
	void SListSounds();
	bool SPrintInfo();
};

#endif
