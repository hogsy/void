/*

#ifndef _V_DIRECT_MUSIC
#define _V_DIRECT_MUSIC

#include "I_musdm.h"
#include <dmusicc.h>
#include <dmusici.h>


class CDirectMusic:public I_MusDMusic
{
public:
	// IUnknown
	virtual HRESULT __stdcall QueryInterface(const IID& iid, void** ppv) ;
	virtual ULONG __stdcall AddRef() ;
	virtual ULONG __stdcall Release() ;

	virtual bool __stdcall DMInit(DPRINT print,char *gamepath, IDirectSound *p_ds);
	virtual bool __stdcall Shutdown();
	virtual bool __stdcall Play(const char *file);
	virtual bool __stdcall Stop();
	virtual bool __stdcall Resume();
	virtual bool __stdcall Pause();

	CDirectMusic();
	~CDirectMusic();

private:

	static IDirectMusic	*			m_pDirect;
	static IDirectMusicLoader*      m_pLoader;
	static IDirectMusicPerformance* m_pPerformance;
	static IDirectMusicSegment*     m_pSegment;
	static IDirectMusicPort*		m_pPort;
	static IDirectMusicComposer* 	m_pComposer;
	static IDirectMusicStyle*       m_pStyle;


	long	m_refcount;
	char	muspath[_MAX_PATH];
	DPRINT dprintf;
	
	HRESULT PlayMotif( WCHAR* pwszMotifName );
	HRESULT InitSynth( IDirectSound *p_ds);
	HRESULT InitPort(DWORD minChannelGroups);

};

#endif
*/







