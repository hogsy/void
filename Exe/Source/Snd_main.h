#ifndef _V_DSOUND_
#define _V_DSOUND_

#include <dsound.h>
#include "Snd_wave.h"

#define SND_PATH "sounds\\"

class CSound
{
public:

	CSound();
	~CSound();

	static char				soundpath[MAX_PATH];
	static CWavemanager	*	m_pWavelist;
	
	// Create the IDirectSound object.
	static bool Init(); //int maxchannels=8); 
	
	// Release the IDirectSound object.
	static bool Shutdown(void);

	// Whether the IDirectSound object exists.
	static bool Exists(void) { return (m_pdSound != 0); }

	//Register Sound func to be visible to everyone else
	//load unload sound funcs

	//Game Frame
	static void RunSounds();

	//Return the Direct Sound Object
	static IDirectSound * GetDirectSound();

	// Creates a sound buffer.
	static HRESULT	MakeBuffer(IDirectSoundBuffer** buffer,
										  unsigned int bufferSize,
										  unsigned int sampleRate, 
										  unsigned int bitDepth, 
										  unsigned int channels);
	
	// Duplicates a sound buffer.
	static IDirectSoundBuffer* CopyBuffers(IDirectSoundBuffer*);

	//temp
	void Play(char *name,bool loop);

	static void SPlay(int argc,  char** argv);
	static void SPause(int argc,  char** argv);
	static void SStop(int argc,  char** argv);
	static void SResume(int argc,  char** argv);
	static bool SVolume(const CVar * var, int argc,  char** argv);	//validation func

private:
	static IDirectSound * m_pdSound;		// The IDirectSound object.
	
	static CVar 	    * m_pvolume;		// Master DirectSound Volume 
	static CVar			* m_pChannels;		// Sound channels

//	static CVar 	    m_pvolume;		// Master DirectSound Volume 
//	static CVar			m_pChannels;		// Sound channels
};

extern CSound * g_pSound;

void DSError(HRESULT hr);

#endif 
