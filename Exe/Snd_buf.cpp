#include "Snd_buf.h"

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
    dsbdesc.dwFlags = DSBCAPS_CTRL3D | DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME; 

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

#if 1
	if(!SetVolume(m_volume))
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
	dsbdesc.dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_GETCURRENTPOSITION2 | 
					  DSBCAPS_GLOBALFOCUS |DSBCAPS_STATIC |
					  DSBCAPS_CTRL3D | DSBCAPS_MUTE3DATMAXDISTANCE;
	dsbdesc.guid3DAlgorithm = DS3DALG_DEFAULT;
    dsbdesc.dwBufferBytes = m_pWaveData->m_size;
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
		delete m_pWaveData;
		m_pWaveData = 0;
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
	if(m_pWaveData) 
	return m_pWaveData->m_filename; 
  return 0; 
}

IDirectSoundBuffer * CSoundBuffer::GetDSBuffer() const { return m_pDSBuffer; }
CWaveFile		   * CSoundBuffer::GetWaveData() const { return m_pWaveData; }