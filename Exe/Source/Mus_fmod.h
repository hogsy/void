#ifdef INCLUDE_FMOD


#ifndef VOID_MUSIC_FMOD
#define VOID_MUSIC_FMOD

#include "Mus_main.h"
#include "Fmod/fmod.h"

namespace VoidMusic {

class CMusFMod :public CMusDriver
{
public:

	CMusFMod();
	~CMusFMod();

	bool  Init();
	bool  Shutdown();

	bool  Play(char * trackname);
	bool  SetPause(bool pause);
	bool  Stop();
	void  PrintStats();
	void  SetVolume(float vol);
	float GetVolume() const;

private:

	const char * ErrorMessage(long err);

	HWND	m_hwnd;
	FSOUND_STREAM * m_pStream;
	int	  m_volume;
};


}

#endif


#endif