#ifndef VOID_SOUND_CHANNEL
#define VOID_SOUND_CHANNEL

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
	bool Create2d(CSoundBuffer &buffer,
				  int volume);

	bool Create3d(CSoundBuffer &buffer,
				  const vector_t &origin, 
				  float muteDist);	
	void Destroy();
	
	bool Play(bool looping = false);
	void Stop();
	bool IsPlaying() const;

	float GetVolume();
	void  SetVolume(float vol);

private:

	bool m_bInUse;

	bool CreateBuffer(CSoundBuffer &buffer);

	IDirectSound3DBuffer * m_pDS3dBuffer; 
	IDirectSoundBuffer   * m_pDSBuffer;	
};

}

#endif