#ifndef VOID_SOUND_PRIVHDR
#define VOID_SOUND_PRIVHDR

#include <windows.h>
#include <mmsystem.h>
#include <dsound.h>

#include "Snd_wave.h"

namespace VoidSound {

//class CWaveFile;		//A Wave file
//class CWaveManager;

LPDIRECTSOUND  GetDirectSound();
CWaveManager * GetWaveManager();

float GetMuteDist(float volume, int attenuation);

void PrintDSErrorMessage(HRESULT hr, char * prefix);

}


#endif


