#ifndef VOID_MUSIC_INTERFACE
#define VOID_MUSIC_INTERFACE

#include "Sys_hdr.h"

//======================================================================================
//Not doing anything special for music
//client code can just send console messages to play stuff
//======================================================================================

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
	class CDirectMusic;
}

//======================================================================================
//======================================================================================

class CMusic : public I_ConHandler
{
public:

	CMusic();
	~CMusic();
	
	bool Init(); 
	void Shutdown();

	void HandleMCIMsg(uint &wParam, long &lParam);

	void HandleCommand(int cmdId, const CParms &parms);
	bool HandleCVar(const CVarBase * cvar, const CStringVal &strVal);

private:

	VoidMusic::CMusCDAudio * m_pCDAudio;

	//playback volume
	CVar m_cVolume;		
	bool Volume(const CVar * var, const CParms &parms);

	//Command Handling
	void Play(const char* arg);
	void Pause();
	void Stop();
	void Resume();
	void PrintStats();
};

#endif