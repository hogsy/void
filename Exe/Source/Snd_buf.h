#ifndef _V_DSOUND_BUF
#define _V_DSOUND_BUF

#include "Snd_main.h"
//#include "World.h"


#define S_LOOPING	1

class CDirectSoundBuffer
{
public:
	CDirectSoundBuffer();
	~CDirectSoundBuffer();

	// Whether the IDirectSoundBuffer has been created.
	operator bool(void) const { return m_pBuffer != 0; }
	bool Exists (){ return m_pBuffer != 0; }
	
	//Load the file into the buffer

	bool Play(int index, bool looping);
	
	bool Stop(void);			//Stop the playback
	
	int IsPlaying();			//Is it playing right now ?
	
	//Copy the file data from another buffer
	bool Copy(const CDirectSoundBuffer& buf);
	
	// Release the sound buffer.
	bool Release(void);
	
	bool SetVol(long val);	// Set the volume.
	bool SetPan(long val);	// Set the pan.
	
private:
		
	IDirectSoundBuffer	*	m_pBuffer;		//DirectSoundBuffer
	
	int						soundindex;
	int						type;			//looping 
	int						attentuation;	//how far can it be heard
	vector_t				origin;			//where the hell is it
};


#endif
