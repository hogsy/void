#ifndef VOID_SOUND_BUFFER
#define VOID_SOUND_BUFFER

#include "Snd_main.h"
#include "Snd_hdr.h"
#include "Snd_wave.h"

//======================================================================================
//======================================================================================
//Use all static buffers for now

namespace VoidSound {

/*
==========================================
The Primary sound Buffer
==========================================
*/
class CPrimaryBuffer
{
public:
	CPrimaryBuffer();
	~CPrimaryBuffer();

	IDirectSound3DListener  * Create(WAVEFORMATEX &pcmwf);
	void Destroy();

	//Master Volume,	range 0 (max) to 5000 (min)
	long GetVolume();
	bool SetVolume(long vol);

	void PrintStats() const;

private:

	long m_volume;

	IDirectSoundBuffer * m_pDSBuffer;	//DirectSoundBuffer
};

/*
==========================================
A sound buffer with access to wave data
==========================================
*/
class CSoundBuffer
{
public:
	CSoundBuffer();
	~CSoundBuffer();

	//Update these to keep cache type later on
	bool Create(const char * path);		//Create a new buffer
	
	void Destroy();

	bool  InUse()	   const;
	void  PrintStats() const;
	IDirectSoundBuffer * GetDSBuffer() const;
	CWaveFile		   * GetWaveData() const;
	const char         * GetFilename() const;
	
private:
	IDirectSoundBuffer * m_pDSBuffer;	//DirectSoundBuffer
	CWaveFile		   * m_pWaveData;	//Wave Data, will be 0 if duplicate buffer
};


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

	bool Create(CSoundBuffer &buffer);	//Create a duplicate buffer
	void Destroy();
	
	bool Play(bool looping = false);
	void Stop();
	bool IsPlaying() const;

	ulong GetVolume();
	void  SetVolume(ulong vol);

private:
	IDirectSoundBuffer * m_pDSBuffer;	//DirectSoundBuffer
};


}

#endif