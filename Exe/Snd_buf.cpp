#include "Snd_buf.h"

//======================================================================================
//======================================================================================

using namespace VoidSound;

//======================================================================================
//The Primary Sound buffer
//======================================================================================

CPrimaryBuffer::CPrimaryBuffer()
{
	m_pDSBuffer = 0;
	m_volume = 0;
}

CPrimaryBuffer::~CPrimaryBuffer()
{
	Destroy();
}
/*
==========================================
Initialize, set format and start mixing
==========================================
*/
IDirectSound3DListener * CPrimaryBuffer::Create(WAVEFORMATEX &pcmwf)
{
	//Set up DSBUFFERDESC structure. 
	DSBUFFERDESC dsbdesc; 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize  = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_CTRLVOLUME |DSBCAPS_PRIMARYBUFFER; 
	dsbdesc.guid3DAlgorithm = GUID_NULL;
    dsbdesc.dwBufferBytes = 0;
    dsbdesc.lpwfxFormat = 0; 
    
	//Create buffer. 
	HRESULT hr = GetDirectSound()->CreateSoundBuffer(&dsbdesc,&m_pDSBuffer,0);
    if(FAILED(hr))
    { 
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:");
		return 0;
        //return false;
    } 

	hr = m_pDSBuffer->SetFormat(&pcmwf);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Set Format:");
		Destroy();
		//return false;
		return 0;
	}

	hr = m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Can't start mixing:");
		Destroy();
		//return false;
		return 0;
	}

#if 1
	if(!SetVolume(m_volume))
	{
		ComPrintf("CPrimaryBuffer::Create: Unable to set init volume\n");
		Destroy();
		//return false;
		return 0;
	}
#endif

	IDirectSound3DListener * p3dlistener=0;
	hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&p3dlistener);
	if(FAILED(hr))
    {
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Get 3dlistener:");
		Destroy();
		//return false;
		return 0;
	}


	ComPrintf("CPrimaryBuffer::Create: OK\n");
	return p3dlistener;
}

/*
==========================================
Destroy the primary buffer
==========================================
*/
void CPrimaryBuffer::Destroy()
{
	if(m_pDSBuffer)
	{
		m_pDSBuffer->Release();
		m_pDSBuffer = 0;
	}
}

/*
==========================================
Print current format
==========================================
*/
void CPrimaryBuffer::PrintStats() const
{
	if(!m_pDSBuffer)
		return;

	WAVEFORMATEX wavFormat;
	memset(&wavFormat,0,sizeof(wavFormat));
	wavFormat.cbSize = sizeof(wavFormat);
	
	HRESULT hr = m_pDSBuffer->GetFormat(&wavFormat,sizeof(wavFormat),0);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::PrintStats: Unable to get format");
		return;
	}

	ComPrintf("Primary Buffer:\n");
	ComPrintf(" Bits per sample:%d\n",wavFormat.wBitsPerSample);
	ComPrintf(" Samples per sec:%d\n",wavFormat.nSamplesPerSec);

	DSBCAPS dsCaps;
	memset(&dsCaps,0,sizeof(DSBCAPS));
	dsCaps.dwSize = sizeof(DSBCAPS);
	
	hr = m_pDSBuffer->GetCaps(&dsCaps);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::PrintStats: Unable to get caps");
		return;
	}
	ComPrintf(" %d bytes\n",dsCaps.dwBufferBytes);
}

/*
==========================================
Master Volume
==========================================
*/
long CPrimaryBuffer::GetVolume()
{
	if(!m_pDSBuffer)
		return 0;

	long lvol=0;
	if(FAILED(m_pDSBuffer->GetVolume(&lvol)))
	{
		ComPrintf("CPrimaryBuffer:GetVolume: Failed to get volume\n");
		return 0;
	}
	return lvol;
}

bool CPrimaryBuffer::SetVolume(long vol)
{
	if(!m_pDSBuffer)
	{
		m_volume = vol;
		return true;
	}

	HRESULT hr = m_pDSBuffer->SetVolume(vol);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::SetVolume:");
		ComPrintf("Unable to set to %d db\n",vol);
		return false;
	}
	m_volume = vol;
	return true;
}


//======================================================================================
//A Generic Sound buffer with wave data
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CSoundBuffer::CSoundBuffer()
{
	m_pWaveData = 0;
	m_pDSBuffer = 0;
}

CSoundBuffer::~CSoundBuffer()
{
	if(InUse())
		Destroy();
	if(m_pWaveData)
		delete m_pWaveData;
}

/*
==========================================
Create the DSound Buffer from a wave file
==========================================
*/
bool CSoundBuffer::Create(const char * path)
{
	m_pWaveData = new CWaveFile(path);
	
	if(m_pWaveData->IsEmpty())
	{
		delete m_pWaveData;
		m_pWaveData = 0;
		return false;
	}


	WAVEFORMATEX waveFormat;
	// Set up wave format structure. 
    memset(&waveFormat, 0, sizeof(WAVEFORMATEX)); 
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	waveFormat.nChannels = 1;
	waveFormat.nBlockAlign = m_pWaveData->m_blockAlign;		
	waveFormat.wBitsPerSample = m_pWaveData->m_bitsPerSample;
	waveFormat.nSamplesPerSec = m_pWaveData->m_samplesPerSecond;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    
	//Set up DSBUFFERDESC structure. 
	DSBUFFERDESC dsbdesc; 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize  = sizeof(DSBUFFERDESC); 
	//Need default controls (pan, volume, frequency). 
    dsbdesc.dwFlags = DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS //|DSBCAPS_STATIC
						| DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;

	//DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME |

	// 5-second buffer. 
    dsbdesc.dwBufferBytes = m_pWaveData->m_size;
    dsbdesc.lpwfxFormat = &waveFormat; 
    
	// Create buffer. 
	HRESULT hr = GetDirectSound()->CreateSoundBuffer(&dsbdesc,&m_pDSBuffer,0);
    if(FAILED(hr))
    { 
		PrintDSErrorMessage(hr,"CSoundBuffer::Create:");
		m_pDSBuffer = 0;
        return false;
    } 
	return true;
}

/*
==========================================
has the buffer been created
==========================================
*/
bool CSoundBuffer::InUse() const
{
	if(m_pDSBuffer) return true;
	return false;
}


/*
==========================================
Print info about the Buffer
==========================================
*/
void CSoundBuffer::PrintStats() const
{
	if(m_pWaveData)
		ComPrintf("(%2d): %s : (%5d) %d bytes\n", m_pWaveData->m_bitsPerSample, m_pWaveData->m_filename,
												m_pWaveData->m_samplesPerSecond, m_pWaveData->m_size);
}

/*
==========================================
Release it
==========================================
*/
void CSoundBuffer::Destroy()
{
	if(m_pDSBuffer)
	{
		m_pDSBuffer->Release();
		m_pDSBuffer = 0;
	}
}

/*
==========================================
Access funcs
==========================================
*/
IDirectSoundBuffer * CSoundBuffer::GetDSBuffer() const { return m_pDSBuffer; }
CWaveFile		   * CSoundBuffer::GetWaveData() const { return m_pWaveData; }
const char         * CSoundBuffer::GetFilename() const 
{ if(m_pWaveData) 
	return m_pWaveData->m_filename; 
  return 0; 
}


//======================================================================================
//Channel
//======================================================================================

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

	//Get a 3d Interface if its a 3d sound
	if(porigin || pvelocity)
	{
		hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DBuffer,(LPVOID *)&m_pDS3dBuffer);
		if(FAILED(hr))
		{
ComPrintf("Unable to get 3d interface\n");
			Destroy();
			return false;
		}

		m_pDS3dBuffer->SetMinDistance(150, DS3D_IMMEDIATE);
		m_pDS3dBuffer->SetMaxDistance(600, DS3D_IMMEDIATE);

		if(porigin) 
			m_pDS3dBuffer->SetPosition(porigin->x, porigin->y, porigin->z, DS3D_IMMEDIATE); //DS3D_DEFERRED);
		if(pvelocity)
			m_pDS3dBuffer->SetVelocity(pvelocity->x, pvelocity->y, pvelocity->z, DS3D_IMMEDIATE);
		origin = porigin;
		velocity = pvelocity;
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

