#ifndef VOID_MUS_CDAUDIO
#define VOID_MUS_CDAUDIO

#if 0

#include "Mus_main.h"
#include <mmsystem.h>


class CMusCDAudio : public CMusDriver
{
public:
	CMusCDAudio();
	~CMusCDAudio();

	bool  Init();
	bool  Shutdown();
	bool  Play(char * trackname);
	bool  SetPause(bool pause);
	bool  Stop();
	void  PrintStats();
	void  SetVolume(float vol);
	float GetVolume() const;

	void  PrintErrorMessage(DWORD err, const char * msg);

private:

	HWND			m_hwnd;
	MCIDEVICEID		m_mciDevice;	//mci device

	int				m_numTracks;
	int				m_curTrack;
};


#endif



#if 0
class CCDMusic : public I_MusCD
{
public:

	virtual bool __stdcall CDInit(DPRINT print, HWND	hwnd);
	virtual bool __stdcall CDShutdown();
	virtual bool __stdcall CDPlay(int track);
	virtual bool __stdcall CDResume();
	virtual bool __stdcall CDStop() ;
	virtual bool __stdcall CDPause();

	virtual bool __stdcall CDEject();
	virtual bool __stdcall CDClose();
	
	virtual void __stdcall CDTrackListing();

	CCDMusic();
	~CCDMusic();

private:

	DWORD SendCommand(UINT uMsg, DWORD fdwCommand, DWORD dwParam);
	DWORD GetStatus(DWORD dwItem);
	DWORD Set(DWORD dwWhat);

	DWORD Play(DWORD dwFrom /*=0L*/, DWORD dwTo /*=0L*/, BOOL bAsync /*=TRUE*/);
	DWORD Seek(DWORD dwTo, BOOL bAsync /*=FALSE*/);
	DWORD SeekToStart(BOOL bAsync /*=FALSE*/);
	DWORD SeekToEnd(BOOL bAsync /*=FALSE*/);
	DWORD Seek(DWORD dwTo, DWORD dwFlags, BOOL bAsync);
	
	DWORD GetTrackPos(DWORD dwTrack) ;
	DWORD GetTrackType(DWORD dwTrack) ;
	DWORD GetTrackLength(DWORD dwTrack); 
	DWORD GetTrackInfo(DWORD dwTrack, DWORD dwItem) ;

	DWORD GetMediaLength(DWORD dwTrack);
	DWORD GetCurrentTrack(); 
	DWORD GetCurrentPos() ;
	DWORD GetStartPos();
	DWORD GetNumberOfTracks() ;
	BOOL  IsReady();


	DWORD SetTimeFormat(DWORD dwTimeFormat);
	DWORD GetTimeFormat();


	
	void  PrintError(DWORD err);
	void  Print(char *s,...);
	
	
	HWND		m_hwnd;
	MCIDEVICEID m_mciDevice;	//mci device
	DWORD		m_lasterror;
	
	CDStates m_cdstate;				
	int m_totracks;
	int m_curtrack;	
};

#endif

#endif