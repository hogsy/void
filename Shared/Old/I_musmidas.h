#ifndef V_INTERFACE_MIDAS
#define V_INTERFACE_MIDAS

//DirectSound needs to be included before
#include <dsound.h>

interface I_MusMidas : IUnknown
{
	virtual bool pascal InitMidas(DPRINT print,const char *basepath, const HWND &hwnd, const IDirectSound *p_ds)=0;
	virtual bool pascal Shutdown()=0;

	virtual bool pascal Play(char *filename)=0;
	virtual bool pascal Pause()=0;
	virtual bool pascal Stop()=0;
	virtual bool pascal Resume()=0;

};

extern "C" const IID IID_IMUSMIDAS;
extern "C" const CLSID CLSID_VMidasMusic;

#endif


