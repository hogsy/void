#ifndef VOID_MUSIC_INTERFACE
#define VOID_MUSIC_INTERFACE

#include "Sys_hdr.h"

/*
======================================================================================
The music code is VERY messy and bare bones right now.
Not doing anything special
client code can just send console messages to play stuff
======================================================================================
*/

namespace VoidMusic
{
	enum EMusState
	{
		M_INACTIVE = 0,
		M_STOPPED  = 1,
		M_PAUSED   = 2,
		M_PLAYING  = 3
	};

	class CMusCDAudio;
//	class CDirectMusic;
}

struct FSOUND_STREAM;

//======================================================================================
//======================================================================================

class CMusic : public I_ConHandler
{
public:

	CMusic();
	~CMusic();
	
	bool Init(); 
	void Shutdown();

	void PlayMp3(const char * szFile);
	void StopMp3();

	void HandleMCIMsg(uint &wParam, long &lParam);

	void HandleCommand(int cmdId, const CParms &parms);
	bool HandleCVar(const CVar * cvar, const CStringVal &strVal);

private:

	VoidMusic::CMusCDAudio * m_pCDAudio;

	bool m_bFMod;
	int	 m_mp3Chan;
	FSOUND_STREAM * m_pStream;

	//playback volume
	CVar * m_cVolume;		
	bool Volume(const CVar * var, const CParms &parms);

	//Command Handling
	void Play(const char* arg);
	void Pause();
	void Stop();
	void Resume();
	void PrintStats();
};

#endif