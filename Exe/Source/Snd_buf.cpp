#include "Sys_hdr.h"
#include "Snd_buf.h"
#include "Snd_wave.h"



/*
=======================================
Constructor
=======================================
*/
CDirectSoundBuffer::CDirectSoundBuffer()
{	
	m_pBuffer=0;
	soundindex=0;
	type=0;
	attentuation=0;
	origin.x=0;
	origin.y=0;
	origin.z=0;
	return;
}


/*
=======================================
Destructor
=======================================
*/

CDirectSoundBuffer::~CDirectSoundBuffer()
{
	Release();
	return;
}


/*
=======================================
Copy the buffer from another buffer
- relies on CDirectSound Object
  to copy
=======================================
*/


bool CDirectSoundBuffer::Copy(const CDirectSoundBuffer& buf)
{
	// Release our sound first.
	if (!Release())
	{
		ComPrintf("CDirectSoundBuffer::Copy - Could not release\n");
		return false;
	}

	// Copy the sound buffer.

	m_pBuffer = CSound::CopyBuffers(buf.m_pBuffer);


	// Return whether the copy worked.
	return (m_pBuffer != 0);
}


/*
=======================================
Release the Direct Sound Buffer
- relies on CDirectSound Object
  to check if it exists or not
=======================================
*/
bool CDirectSoundBuffer::Release(void)
{
	if (!m_pBuffer) 
	{ return true; 
	}

	//Check if DirectSound Exists anymore or not
	bool result = !CSound::Exists() || m_pBuffer->Release() == 0;
//	ComPrintf("CDirectSoundBuffer::Release() - Released.\n");

	// Set the pointer to null.
	m_pBuffer = 0;
	soundindex=0;
	type=0;
	attentuation=0;
	origin.x=0;
	origin.y=0;
	origin.z=0;
	return result;
}

/*
=======================================
Play our buffer
=======================================
*/


bool CDirectSoundBuffer::Play(int index, bool looping)
{
	if(!Release())
	{
		ComPrintf("CDirectSoundBuffer::Load - Release Failed\n");
		return false;
	}

	CWavefile *wave;
	wave = CSound::m_pWavelist->WaveIndex(index);
	if(!wave)
	{
		ComPrintf("CDirectSoundBuffer::Load - couldnt load soundindex %d\n", index);
		return false;
	}

	HRESULT hr;
	void *lockPtr1, *lockPtr2;
	unsigned long lockSize1, lockSize2;

	hr= CSound::MakeBuffer(&m_pBuffer,
						   wave->length, 
						   wave->samplesPerSecond, 
						   (wave->blockAlign / wave->numChannels)*8, 
						   wave->numChannels);

	if(FAILED(hr))
	{
		DSError(hr);
		return false;
	}
	
	// lock the buffer
	if ((hr = m_pBuffer->Lock(0, wave->length, 
							&lockPtr1, &lockSize1, &lockPtr2, &lockSize2, 0)) != DS_OK)
	{
		if (hr != DSERR_BUFFERLOST)
		{
			ComPrintf("CDirectSoundBuffer::Load - could not lock buffer -%s", wave->filename);
			return false;
		}
		m_pBuffer->Restore();
		if (m_pBuffer->Lock(0, wave->length, &lockPtr1, &lockSize1, &lockPtr2, &lockSize2, 0) != DS_OK)
		{
			ComPrintf("CDirectSoundBuffer::Load - could not lock buffer 2-%s", wave->filename);
			return false;
		}
	}

	// write the data
	if (lockSize1)
		memcpy(lockPtr1, wave->data, lockSize1);
	if (lockSize2)
		memcpy(lockPtr2, wave->data+lockSize1, lockSize2);

	// unlock it
	m_pBuffer->Unlock(lockPtr1, lockSize1, lockPtr2, lockSize2);


	// Make sure we have a buffer.
	if (!m_pBuffer) 
	{
		ComPrintf("CDirectSoundBuffer::Play - No buffer\n");
		return false;
	}

	// Play the sound.
	unsigned long flags = looping ? DSBPLAY_LOOPING : 0;
	hr = m_pBuffer->Play(0, 0, flags);
	
	//Worked - update other info about the sound here
	if (SUCCEEDED(hr)) 
	{	
		soundindex=index;
		type=(int)looping;
		attentuation=0;
		origin.x=0;
		origin.y=0;
		origin.z=0;
		return true; 
	}
	
	// See if the buffer was lost.
	if (hr == DSERR_BUFFERLOST) 
	{	
		hr = m_pBuffer->Restore(); 	
		//try playing again
		if (SUCCEEDED(hr)) 
		{	
			hr = m_pBuffer->Play(0, 0, flags); 
			ComPrintf("DirectSoundBuffer::Play tried playing twice\n");
			if(SUCCEEDED(hr))
			{
				soundindex=index;
				type=(int)looping;
				attentuation=0;
				origin.x=0;
				origin.y=0;
				origin.z=0;
				return false;
			}
		}
	}
	DSError(hr);
	return false;
}


/*
=======================================
Stop Playback
=======================================
*/

bool CDirectSoundBuffer::Stop(void)
{
	// Make sure we have a buffer.
	if (!m_pBuffer) 
	{
		ComPrintf("DirectSoundBuffer::Stop() - No buffer\n");
		return false;
	}

	// Stop the buffer.
	HRESULT hr = m_pBuffer->Stop();

	if (FAILED(hr)) 
	{
		ComPrintf("DirectSoundBuffer::Stop() - failed\n");
		return false;
	}

	// Rewind the buffer.
	hr = m_pBuffer->SetCurrentPosition(0);

	if (FAILED(hr)) 
	{
		ComPrintf("DirectSoundBuffer::Stop() Failed SetCurrentPos()\n");
		return false;
	}
	return true;
}


/*
=======================================
Sets the Volume of the Buffer
=======================================
*/

bool CDirectSoundBuffer::SetVol(long val)
{
	if (!m_pBuffer) 
	{
		ComPrintf("DirectSoundBuffer::SetVol() - No buffer\n");
		return false;
	}

	HRESULT hr = m_pBuffer->SetVolume(val);

	if (FAILED(hr)) 
	{
		ComPrintf("DirectSoundBuffer::SetVol() - SetVolume() failed\n");
		return false;
	}

	return true;
}

/*
=======================================
Sets the Panning of the buffer
=======================================
*/

bool CDirectSoundBuffer::SetPan(long val)
{
	if (!m_pBuffer) 
	{
		ComPrintf("DirectSoundBuffer::SetPan() - No buffer\n");
		return false;
	}

	HRESULT hr = m_pBuffer->SetPan(val);

	if (FAILED(hr))
	{
		ComPrintf("DirectSoundBuffer::SetPan() - SetPan() failed\n");
		return false;
	}
	return true;
}



/*
======================================
Is it playing
return soundindex if yes
======================================
*/

int CDirectSoundBuffer::IsPlaying()
{
	if (!m_pBuffer) 
	{
//		ComPrintf("DirectSoundBuffer::IsPlaying() - No buffer\n");
		return 0;
	}
	
	DWORD status;
	HRESULT	hr = m_pBuffer->GetStatus(&status);
	if(!FAILED(hr))
	{
		if(status & DSBSTATUS_PLAYING)
			return soundindex;
		else if(status & DSBSTATUS_LOOPING)
			return soundindex;
	}
	return 0;
}


