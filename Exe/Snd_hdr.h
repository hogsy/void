#ifndef VOID_SOUND_PRIVHDR
#define VOID_SOUND_PRIVHDR

#include <mmsystem.h>
#include <dsound.h>


namespace VoidSound
{
	class CWaveFile;		//A Wave file

	LPDIRECTSOUND GetDirectSound();
	
	void PrintDSErrorMessage(HRESULT hr, char * prefix);
}


#endif


