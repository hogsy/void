#ifndef VOID_SOUND_BUFFER
#define VOID_SOUND_BUFFER

#include "Snd_hdr.h"

//======================================================================================
//======================================================================================

namespace VoidSound {
/*
======================================
3d listener
======================================
*/
class C3DListener
{
public:
	IDirectSound3DListener * m_pDS3dListener;

	C3DListener(IDirectSound3DListener * listener) : 
		m_pDS3dListener(listener)	{}
	
	~C3DListener()
	{ 
		if(m_pDS3dListener) 
		{	
			m_pDS3dListener->Release(); 
			m_pDS3dListener =0;
		}	
	}
};


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

	IDirectSound3DListener  * Create(WAVEFORMATEX &pcmwf, float vol);
	void Destroy();

	float GetVolume();
	bool  SetVolume(float vol);

	void PrintStats() const;

private:

//	float m_volume;
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

	//Create a new buffer
	bool Create(const char * path);		
	void Destroy();

	bool  InUse()	   const;
	void  PrintStats() const;
	IDirectSoundBuffer * GetDSBuffer() const;
	CWaveFile		   * GetWaveData() const;
	const char         * GetFilename() const;
	
private:

	IDirectSoundBuffer * m_pDSBuffer;
	CWaveFile		   * m_pWaveFile;	
};

}

#endif