#ifndef VOID_MUS_CDAUDIO
#define VOID_MUS_CDAUDIO

#include "Mus_main.h"
#include <mmsystem.h>

namespace VoidMusic 
{

class CMusCDAudio : public I_ConHandler
{
public:

	CMusCDAudio();
	~CMusCDAudio();

	bool Init();
	bool Shutdown();

	void HandleMCIMsg(uint &wParam, long &lParam);
	
	void HandleCommand(int cmdId, const CParms &parms);
	bool HandleCVar(const CVar * cvar, const CStringVal &strVal) { return false; } 
	
	
private:

	void Play(const char * trackname);
	void SetPause(bool pause);
	void Stop();
	void OpenTray(bool open);
	void PrintStats();

	void PrintErrorMessage(DWORD err, const char * msg);
	bool SetTimeFormat(DWORD format);
	bool IsCDInserted();
	int  GetNumTracks();
	
	int		m_numTracks;
	int		m_curTrack;
	DWORD   m_curPos;
	bool	m_cdInserted;
	HWND	m_hwnd;

	//use this to hold proposed changes, which are set or 
	//ignored depending on what the notify callback returns
	EMusState		m_eWaitState;
	
	//Current state
	EMusState		m_eState;

	//CDrom id
	MCIDEVICEID		m_mciDevice;
};

}

#endif

