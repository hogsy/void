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

class CMusic : public I_CmdHandler,
			   public I_CVarHandler
{
public:

	CMusic();
	~CMusic();
	
	bool Init(); 
	void Shutdown();

	void HandleMCIMsg(uint &wParam, long &lParam);

	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);
	bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);

private:

	VoidMusic::CMusCDAudio * m_pCDAudio;

	//playback volume
	CVar m_cVolume;		
	bool Volume(const CVar * var, int argc, char** argv);

	//Command Handling
	void Play(int argc, char** argv);
	void Pause();
	void Stop();
	void Resume();
	void PrintStats();
};

#endif