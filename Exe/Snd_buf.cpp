#include "Sys_hdr.h"
#include "Snd_hdr.h"
#include "Snd_buf.h"

using namespace VoidSound;

//======================================================================================
//The Primary Sound buffer
//======================================================================================

CPrimaryBuffer::CPrimaryBuffer()
{
	m_pDSBuffer = 0;
//	m_volume = 0;
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
IDirectSound3DListener * CPrimaryBuffer::Create(WAVEFORMATEX &pcmwf, float vol)
{
	//Set up DSBUFFERDESC structure. 
	DSBUFFERDESC dsbdesc; 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize  = sizeof(DSBUFFERDESC); 
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;
					//| DSBCAPS_CTRLPAN |DSBCAPS_CTRLFREQUENCY; 

  	//Create buffer. 
	HRESULT hr = GetDirectSound()->CreateSoundBuffer(&dsbdesc,&m_pDSBuffer,0);
    if(FAILED(hr))
    { 
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:");
		return 0;
    } 

	IDirectSound3DListener * p3dlistener=0;
	hr = m_pDSBuffer->QueryInterface(IID_IDirectSound3DListener, (LPVOID *)&p3dlistener);
	if(FAILED(hr))
    {
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Get 3dlistener:");
		Destroy();
		return 0;
	}

	hr = m_pDSBuffer->SetFormat(&pcmwf);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Set Format:");
		Destroy();
		return 0;
	}

#if 0
	if(!SetVolume(vol))
	{
		ComPrintf("CPrimaryBuffer::Create: Unable to set init volume\n");
		Destroy();
		return 0;
	}
#endif

	hr = m_pDSBuffer->Play(0,0,DSBPLAY_LOOPING);
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CPrimaryBuffer::Create:Can't start mixing:");
		Destroy();
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
float CPrimaryBuffer::GetVolume()
{
	if(!m_pDSBuffer)
		return 0;

	long lvol=0;
	if(FAILED(m_pDSBuffer->GetVolume(&lvol)))
	{
		ComPrintf("CPrimaryBuffer:GetVolume: Failed to get volume\n");
		return 0;
	}
	ComPrintf("CPrimaryBuffer::GetVolume: %ddB\n", lvol);
	return ((5000.0 - lvol)/500.0);
}

bool CPrimaryBuffer::SetVolume(float fvol)
{
	if(m_pDSBuffer)
	{
		long lvol = -(5000 - fvol*500);
		HRESULT hr = m_pDSBuffer->SetVolume(lvol);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CPrimaryBuffer::SetVolume:");
			ComPrintf("Unable to set to %d(%f) db\n", lvol, fvol);
			return false;
		}
		ComPrintf("CPrimaryBuffer::SetVolume: Set to %ddB. %f\n", lvol, fvol);
	}
//	m_volume = fvol;
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
	m_pWaveFile = 0;
	m_pDSBuffer = 0;
}

CSoundBuffer::~CSoundBuffer()
{
	if(InUse())
		Destroy();
}

/*
==========================================
Create the DSound Buffer from a wave file
==========================================
*/
bool CSoundBuffer::Create(const char * path)
{
	if(InUse())
		Destroy();


	m_pWaveFile = GetWaveManager()->Create(path);
	if(!m_pWaveFile)
		return false;

	WAVEFORMATEX waveFormat;
	// Set up wave format structure. 
    memset(&waveFormat, 0, sizeof(WAVEFORMATEX)); 
	waveFormat.cbSize = sizeof(WAVEFORMATEX);
	waveFormat.nChannels = 1;
	waveFormat.nBlockAlign = m_pWaveFile->m_blockAlign;		
	waveFormat.wBitsPerSample = m_pWaveFile->m_bitsPerSample;
	waveFormat.nSamplesPerSec = m_pWaveFile->m_samplesPerSecond;
	waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;
	waveFormat.wFormatTag = WAVE_FORMAT_PCM;
    
	//Set up DSBUFFERDESC structure. 
	DSBUFFERDESC dsbdesc; 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));
	dsbdesc.dwSize  = sizeof(DSBUFFERDESC); 
	dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | 
					  DSBCAPS_GLOBALFOCUS |DSBCAPS_STATIC |
					  DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
//	dsbdesc.guid3DAlgorithm = DS3DALG_DEFAULT;
    dsbdesc.dwBufferBytes = m_pWaveFile->m_size;
    dsbdesc.lpwfxFormat = &waveFormat; 
    
	// Create buffer. 
	HRESULT hr = GetDirectSound()->CreateSoundBuffer(&dsbdesc,&m_pDSBuffer,0);
    if(FAILED(hr))
    { 
		PrintDSErrorMessage(hr,"CSoundBuffer::Create:");
		Destroy();
        return false;
    } 
	return true;
}

/*
==========================================
Print info about the Buffer
==========================================
*/
void CSoundBuffer::PrintStats() const
{
	if(m_pWaveFile)
		ComPrintf("%2dbit:%5dkhz:%6d bytes: %s\n", m_pWaveFile->m_bitsPerSample, 
												  m_pWaveFile->m_samplesPerSecond, 
												  m_pWaveFile->m_size,
												  m_pWaveFile->GetFileName());
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
		GetWaveManager()->Release(m_pWaveFile);
		m_pWaveFile = 0;
	}
}

/*
==========================================
Util funcs 
==========================================
*/
bool CSoundBuffer::InUse() const 
{ 
	if(m_pDSBuffer)  
		return true;	
	return false; 
}

const char * CSoundBuffer::GetFilename() const 
{ 
	if(m_pWaveFile) 
		return m_pWaveFile->GetFileName(); 
  return 0; 
}

IDirectSoundBuffer * CSoundBuffer::GetDSBuffer() const { return m_pDSBuffer; }
const CWaveFile	   * CSoundBuffer::GetWaveData() const { return m_pWaveFile; }