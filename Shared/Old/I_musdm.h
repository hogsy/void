#ifndef INC_VDIRECTMUSIC
#define INC_VDIRECTMUSIC

#include <dsound.h>

interface I_MusDMusic : IUnknown
{
	virtual bool pascal DMInit(DPRINT print,char *gamepath, IDirectSound *p_ds)=0;
	virtual bool pascal Shutdown()=0;

	virtual bool pascal Play(const char *file)=0;
	virtual bool pascal Stop()=0;
	virtual bool pascal Resume()=0;
	virtual bool pascal Pause()=0;
};

extern "C" const IID IID_IMUSDMUSIC;
extern "C" const CLSID CLSID_VDirectMusic;

#endif


