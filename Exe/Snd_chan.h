#ifndef VOID_SOUND_CHANNEL
#define VOID_SOUND_CHANNEL

#include "Snd_buf.h"
#include "3dmath.h"

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
	bool Create(CSoundBuffer &buffer,
				const vector_t * porigin=0,
				const vector_t * pvelocity=0);	
	void Destroy();
	
	bool Play(bool looping = false);
	void Stop();
	bool IsPlaying() const;

	ulong GetVolume();
	void  SetVolume(ulong vol);

	const vector_t * origin;
	const vector_t * velocity;


	IDirectSound3DBuffer * m_pDS3dBuffer; 
	IDirectSoundBuffer   * m_pDSBuffer;	
};

}

#endif