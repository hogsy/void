#ifndef VOID_MUSIC_INTERFACE
#define VOID_MUSIC_INTERFACE

#include "Sys_hdr.h"

//======================================================================================
//======================================================================================

class CMusDriver
{
public:

	enum EMusState
	{
		M_INACTIVE =0,
		M_PLAYING  =1,
		M_PAUSED   =2,
		M_STOPPED  =4
	};

	CMusDriver() : m_eState(M_INACTIVE) { }
	virtual ~CMusDriver() { }

	virtual bool  Init()=0;
	virtual bool  Shutdown()=0;
	virtual bool  Play(char * trackname)=0;
	virtual bool  SetPause(bool pause)=0;
	virtual bool  Stop()=0;
	virtual void  PrintStats()=0;
	virtual void  SetVolume(float vol)=0;
	virtual float GetVolume() const =0;
	
	EMusState GetState() const { return m_eState; }
	const char * GetTrackName() const { return m_trackName; }

protected:

	char		m_trackName[COM_MAXPATH];
	EMusState	m_eState;
};

//======================================================================================
//======================================================================================

namespace VoidMusic
{
	extern const int MAXCHANNELS;

#ifdef INCLUDE_FMOD
	class CMusFMod;
#endif

	class CMusCDAudio;
//	class CMusDirectMusic;
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

	void HandleCommand(HCMD cmdId, int numArgs, char ** szArgs);
	bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);

	CMusDriver * GetMusicDriver();

private:

	CMusDriver	* m_curDriver;

#ifdef INCLUDE_FMOD
	VoidMusic::CMusFMod	* m_pFMod;
#endif
	
	CVar m_cVolume;		//playback volume
	CVar m_cDriver;

	//Cvar handlers
	bool Volume(const CVar * var, int argc, char** argv);
	bool Driver(const CVar * var, int argc, char** argv);

	//Command Handling
	void Play(int argc, char** argv);
	void Pause();
	void Stop();
	void Resume();
	void PrintStats();
};

#endif