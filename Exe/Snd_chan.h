#ifndef VOID_SOUND_CHANNEL
#define VOID_SOUND_CHANNEL

#include "Snd_buf.h"
#include "3dmath.h"
#include "Clgame_defs.h"

namespace VoidSound {

/*
==========================================
A sound channel which actually gets played
==========================================
*/
class CSoundChannel
{
public:
	CSoundChannel();
	~CSoundChannel();

	//Create a duplicate buffer. then get a 3dinterface from it
	bool Create(const CSoundBuffer &buffer,
				int volume);

	bool Create(const CSoundBuffer &buffer,
				const ClEntity * ent, 
				int volume,
				int attenuation);	
	
	void Destroy();
	
	bool Play(bool looping = false);
	void Stop();
	bool IsPlaying() const;

	float GetVolume();
	void  SetVolume(float vol);

//	hSnd		soundIndex;
//	CacheType	soundCache;
	const ClEntity * m_pEntity;

private:

	bool m_bInUse;

	bool CreateBuffer(const CSoundBuffer &buffer);

	IDirectSound3DBuffer * m_pDS3dBuffer; 
	IDirectSoundBuffer   * m_pDSBuffer;	
};

}

#endif