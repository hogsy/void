#include "Snd_chan.h"

using namespace VoidSound;

/*
======================================================================================
Sound channel
======================================================================================
*/
/*
==========================================
Constructor/Destructor
==========================================
*/

CSoundChannel::CSoundChannel()
{
	m_pDSBuffer= 0;
	m_pDS3dBuffer = 0;
}

CSoundChannel::~CSoundChannel()
{	Destroy();
}

/*
==========================================
Release it
==========================================
*/
void CSoundChannel::Destroy()
{
	if(m_pDSBuffer)
	{
		m_pDSBuffer->Release();
		m_pDSBuffer = 0;
	}
	if(m_pDS3dBuffer)
	{
		m_pDS3dBuffer->Release();
		m_pDS3dBuffer = 0;
	}
	origin = 0;
	velocity = 0;
}

/*
==========================================
from another buffer
==========================================
*/
bool CSoundChannel::Create(CSoundBuffer &buffer,       //Create a duplicate buffer
						   const vector_t * porigin,
						   const vector_t * pvelocity)	
{
	//Destry current buffer if active
	Destroy();

	HRESULT hr = GetDirectSound()->DuplicateSoundBuffer(buffer.GetDSBuffer(), &m_pDSBuffer);
	if(FAILED(hr)) 
	{
		PrintDSErrorMessage(hr, "CSoundChannel::Create:Could not duplicate buffers:");
		return false;
	}

	//Copy the wave file
	void *lockPtr1=0; 
	ulong lockSize1=0;

	// lock the buffer
	hr = m_pDSBuffer->Lock(0, 0, &lockPtr1, &lockSize1, 0,0,DSBLOCK_ENTIREBUFFER);
	if(FAILED(hr))
	{
		if (hr != DSERR_BUFFERLOST)
		{
			ComPrintf("CSoundChannel::Create: Could not lock buffer: %s", buffer.GetWaveData()->m_filename);
			return false;
		}
		m_pDSBuffer->Restore();
		if(m_pDSBuffer->Lock(0, 0, &lockPtr1, &lockSize1, 0,0,DSBLOCK_ENTIREBUFFER) != DS_OK)
		{
			ComPrintf("CSoundChannel::Create: Could not lock buffer 2: %s", buffer.GetWaveData()->m_filename);
			return false;
		}
	}

	// write the data
	if (lockSize1)
		memcpy(lockPtr1, buffer.GetWaveData()->m_data, lockSize1);
	m_pDSBuffer->Unlock(lockPtr1, lockSize1, 0, 0);

		hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,(LPVOID *)&m_pDS3dBuffer);
		if(FAILED(hr))
		{
ComPrintf("Unable to get 3d interface\n");
			Destroy();
			return false;
		}

	//Get a 3d Interface if its a 3d sound
	if(porigin || pvelocity)
	{
		m_pDS3dBuffer->SetMinDistance(150, DS3D_IMMEDIATE);
//		m_pDS3dBuffer->SetMaxDistance(600, DS3D_IMMEDIATE);

		if(porigin) 
			m_pDS3dBuffer->SetPosition(porigin->x, porigin->y, porigin->z, DS3D_IMMEDIATE); //DS3D_DEFERRED);
		if(pvelocity)
			m_pDS3dBuffer->SetVelocity(pvelocity->x, pvelocity->y, pvelocity->z, DS3D_IMMEDIATE);
		origin = porigin;
		velocity = pvelocity;
	}
	else
	{
		//hr = m_pDS3dBuffer->SetMode(DS3DMODE_DISABLE,DS3D_IMMEDIATE);
		hr = m_pDS3dBuffer->SetMode(DS3DMODE_DISABLE,DS3D_DEFERRED);
		if(FAILED(hr))
		{
ComPrintf("Unable to  3d interface\n");
			Destroy();
			return false;
		}

	}

	return true;
}

/*
==========================================
Play the Channel
==========================================
*/
bool CSoundChannel::Play(bool looping)
{
	ulong flags;
	looping ? flags = DSBPLAY_LOOPING: flags = 0;
	
	HRESULT hr = m_pDSBuffer->Play(0, 0, flags);
	
	//Failed
	if (FAILED(hr)) 
	{	
		// See if the buffer was lost.
		if (hr == DSERR_BUFFERLOST) 
		{	
			hr = m_pDSBuffer->Restore(); 	
			//try playing again
			if (SUCCEEDED(hr)) 
			{	
				hr = m_pDSBuffer->Play(0, 0, flags); 
				if(FAILED(hr))
				{
					PrintDSErrorMessage(hr,"CSoundChannel::Play: Failed to play twice");
					return false;
				}
				return true;
			}
		}
		return false;
	}
	return true;
}

/*
==========================================
Stop playback
==========================================
*/
void CSoundChannel::Stop()
{	
	m_pDSBuffer->Stop();
}

/*
==========================================
Is the buffer playing
==========================================
*/
bool CSoundChannel::IsPlaying() const
{
	if (!m_pDSBuffer) 
		return false;
	
	DWORD status;
	HRESULT	hr = m_pDSBuffer->GetStatus(&status);
	if(!FAILED(hr))
	{
		if(status & DSBSTATUS_PLAYING)
			return true;
		else if(status & DSBSTATUS_LOOPING)
			return true;
	}
	return false;
}
