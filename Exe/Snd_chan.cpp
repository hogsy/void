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
	
	m_pEntity  = 0;
	m_muteDist = 0.0f;
	m_bInUse  = false;
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
//Stop ?
		m_pDSBuffer->Release();
		m_pDSBuffer = 0;
	}
	if(m_pDS3dBuffer)
	{
		m_pDS3dBuffer->Release();
		m_pDS3dBuffer = 0;
	}
	m_pEntity =0;
	m_muteDist = 0.0f;
	m_bInUse = false;
}

/*
======================================
Duplicate buffer info
======================================
*/
bool CSoundChannel::CreateBuffer(const CSoundBuffer &buffer)
{
	//Destry current buffer if active
	if(m_bInUse)
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
			ComPrintf("CSoundChannel::Create: Could not lock buffer: %s", buffer.GetFilename());
			return false;
		}
		m_pDSBuffer->Restore();
		if(m_pDSBuffer->Lock(0, 0, &lockPtr1, &lockSize1, 0,0,DSBLOCK_ENTIREBUFFER) != DS_OK)
		{
			ComPrintf("CSoundChannel::Create: Could not lock buffer 2: %s", buffer.GetFilename());
			return false;
		}
	}

	// write the data
	if (lockSize1)
		memcpy(lockPtr1, buffer.GetWaveData()->GetData(), lockSize1);
	m_pDSBuffer->Unlock(lockPtr1, lockSize1, 0, 0);

	hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,(LPVOID *)&m_pDS3dBuffer);
	if(FAILED(hr))
	{
ComPrintf("Unable to get 3d interface\n");
		Destroy();
		return false;
	}
	return true;
}

/*
attenuation, 0 to 10
min =  attention * 50
max =  min * volume;
*/

/*
==========================================
Create a sound sourced from an entitiy
==========================================
*/
bool CSoundChannel::Create(const CSoundBuffer &buffer,
						   const ClEntity * ent,
						   int volume,
						   int attenuation)
{

	if(!CreateBuffer(buffer))
		return false;

	//HRESULT hr;

	//Get a 3d Interface if its a 3d sound
	m_pEntity = ent;

	m_pDS3dBuffer->SetMinDistance(500, DS3D_DEFERRED);
//		m_pDS3dBuffer->SetMaxDistance(600, DS3D_IMMEDIATE);
	m_pDS3dBuffer->SetPosition(m_pEntity->origin.x, 
							   m_pEntity->origin.y, 
							   m_pEntity->origin.z, 
							   DS3D_DEFERRED); //DS3D_DEFERRED);
//	m_pDS3dBuffer->SetVelocity(0, 0, 0, DS3D_IMMEDIATE);

	//Calculate mute distance

	m_bInUse = true;
	return true;
}

/*
======================================
Create a 2d sound
======================================
*/
bool CSoundChannel::Create(const CSoundBuffer &buffer,
							int volume)
{
	if(!CreateBuffer(buffer))
		return false;

	HRESULT	hr = m_pDS3dBuffer->SetMode(DS3DMODE_DISABLE,DS3D_DEFERRED);
	if(FAILED(hr))
	{
ComPrintf("Unable to disable 3d interface\n");
		Destroy();
		return false;
	}
	m_bInUse = true;
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
	ComPrintf("Chan : Volume : %d\n", GetVolume());
	return true;
}

/*
==========================================
Stop playback
==========================================
*/
void CSoundChannel::Stop()
{	m_pDSBuffer->Stop();
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




float CSoundChannel::GetVolume()
{
	if(!m_pDSBuffer)
		return 0;

	long lvol=0;
	if(FAILED(m_pDSBuffer->GetVolume(&lvol)))
	{
		ComPrintf("CSoundChannel:GetVolume: Failed to get volume\n");
		return 0;
	}
	return ((5000.0 - lvol)/500.0);
}

void CSoundChannel::SetVolume(float vol)
{
	if(!m_pDSBuffer)
		return;

	long lvol = -(5000 - vol*500);

	HRESULT hr = m_pDSBuffer->SetVolume(lvol);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CSoundChannel::SetVolume:");
		ComPrintf("Unable to set to %d(%f) db\n",lvol, vol);
	}
}
