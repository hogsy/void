#include "Snd_main.h"
#include "Snd_hdr.h"
#include "Snd_buf.h"
#include "Snd_chan.h"
#include "Com_util.h"

using namespace VoidSound;

namespace
{
	enum
	{
		CMD_PLAY  = 1,
		CMD_STOP  = 2,
		CMD_INFO  = 5,
		CMD_LIST  = 6,
	};
	//Direct sound object.
	IDirectSound	*	m_pDSound = 0;	
	CWaveManager	*	m_pWaveManager = 0;
}

/*
==========================================
Constructor
==========================================
*/
CSoundManager::CSoundManager() : m_cVolume("snd_vol", "9", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cHighQuality("snd_highquality", "1", CVAR_BOOL, CVAR_ARCHIVE),
								 m_cRollOffFactor("snd_rolloff", "1.0", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cDopplerFactor("snd_doppler", "1.0", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cDistanceFactor("snd_distance", "15.0", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cSndFps("snd_maxfps", "30", CVAR_FLOAT, CVAR_ARCHIVE)
{
	m_pListener = 0;	
	m_pPrimary = new CPrimaryBuffer;

	m_pWaveManager = new CWaveManager(MAX_WAVEFILES);

	//Create buffer caches
	for(int i=0; i< CACHE_NUMCACHES; i++)
		m_bufferCache[i] =  new CSoundBuffer[GAME_MAXSOUNDS];
	
	//Create sound channels
	m_Channels = new CSoundChannel[MAX_CHANNELS];

	m_bHQSupport=false;
	m_bStereoSupport= false;
	m_fLastFrame = 0.0f;

	System::GetConsole()->RegisterCVar(&m_cSndFps);
	System::GetConsole()->RegisterCVar(&m_cVolume,this);
	System::GetConsole()->RegisterCVar(&m_cHighQuality,this);
	System::GetConsole()->RegisterCVar(&m_cRollOffFactor,this);
	System::GetConsole()->RegisterCVar(&m_cDopplerFactor,this);
	System::GetConsole()->RegisterCVar(&m_cDistanceFactor,this);
	
	System::GetConsole()->RegisterCommand("sndplay",CMD_PLAY,this);
	System::GetConsole()->RegisterCommand("sndstop",CMD_STOP,this);
	System::GetConsole()->RegisterCommand("sndinfo",CMD_INFO,this);
	System::GetConsole()->RegisterCommand("sndlist",CMD_LIST,this);
}


/*
======================================
Destructor
======================================
*/
CSoundManager::~CSoundManager()
{	
	if(m_pListener)
	{
		delete m_pListener;
		m_pListener=0;
	}

	if(m_Channels)
	{
		delete [] m_Channels;
		m_Channels = 0;
	}

	for(int i=0; i<CACHE_NUMCACHES; i++)
		delete [] m_bufferCache[i];

	if(m_pPrimary)
	{
		delete m_pPrimary;
		m_pPrimary = 0;
	}

	if(m_pDSound)
	{
		m_pDSound->Release();
		m_pDSound = 0;
		ComPrintf("CSound::Shutdown : Released DirectSound\n");
	}
	
	if(m_pWaveManager)
	{
		delete m_pWaveManager;
		m_pWaveManager = 0;
	}
}

/*
==========================================
Initialize
==========================================
*/
bool CSoundManager::Init()
{
	// Nothing to do if already created.
	if (m_pDSound) 
		return true;

	// Create the DirectSound object.
	HRESULT hr = CoCreateInstance(CLSID_DirectSound, 
						  0, 
						  CLSCTX_ALL,
						  IID_IDirectSound, 
						  (void**)&m_pDSound);

	if (FAILED(hr))
	{ 
		ComPrintf("CSound::Init Failed to get DirectSound Interface\n");
		return false; 
	}

	//Initialize the DirectSound object.
	//Defaults to Primary Sound Driver right now
	hr = m_pDSound->Initialize(0);
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed Initialize Directsound\n");
		m_pDSound->Release();
		return false; 
	}

	//Set the cooperative level.
	hr = m_pDSound->SetCooperativeLevel(System::GetHwnd(),
										DSSCL_PRIORITY);
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed SetCoopLevel\n");
		return false; 
	}

	//Get DirectSound caps
	DSCAPS	dsCaps;
	memset(&dsCaps,0,sizeof(DSCAPS));
	dsCaps.dwSize = sizeof(DSCAPS);

	//Set caps
	hr = m_pDSound->GetCaps(&dsCaps);
	if(FAILED(hr))
	{
		ComPrintf("CSound::Init Failed to Get Capabilities\n");
		return false;
	}

	//Print Caps
	ComPrintf("Direct Sound Capabilities:\n");
	ComPrintf("Primary Buffer:\n");
	if(dsCaps.dwFlags & DSCAPS_PRIMARY16BIT)
	{
		ComPrintf(" supports 16bit\n");
		m_bHQSupport = true;
	}
	else
	{
		ComPrintf(" does NOT support 16bit\n");
		m_bHQSupport = false;
	}

	if(dsCaps.dwFlags & DSCAPS_PRIMARYSTEREO)
	{
		ComPrintf(" supports stereo\n");
		m_bStereoSupport = true;
	}
	else
	{
		ComPrintf(" does NOT supports stereo\n");
		m_bStereoSupport = false;
	}


	//Create the primary buffer
	WAVEFORMATEX pcmwf; 
	
	// Set up wave format structure. 
    memset(&pcmwf, 0, sizeof(WAVEFORMATEX)); 
	pcmwf.cbSize = sizeof(WAVEFORMATEX);
	
	if(m_bHQSupport)
		pcmwf.wBitsPerSample = 16;
	else
		pcmwf.wBitsPerSample = 8; 
	
	if(m_bStereoSupport)
		pcmwf.nChannels = 2;
	else
		pcmwf.nChannels = 1;
	
	//linked with samples per sec ?
	pcmwf.nBlockAlign = 4;		

	//Should this be user definable ?
	pcmwf.nSamplesPerSec = 22050;
	pcmwf.nAvgBytesPerSec = pcmwf.nSamplesPerSec * pcmwf.nBlockAlign;
	pcmwf.wFormatTag = WAVE_FORMAT_PCM;

	//Get 3dListener Interface
	IDirectSound3DListener * lpd3dlistener = m_pPrimary->Create(pcmwf, m_cVolume.fval);
	if(!lpd3dlistener)
	{
		ComPrintf("CSound::Init Failed to create listener\n");
		return false;
	}

	//Create Listener object
	m_pListener = new C3DListener(lpd3dlistener);
	m_pListener->m_pDS3dListener->SetDistanceFactor(m_cDistanceFactor.fval,DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetRolloffFactor(m_cRollOffFactor.fval,DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetDopplerFactor(m_cDopplerFactor.fval,DS3D_DEFERRED);
	hr = m_pListener->m_pDS3dListener->CommitDeferredSettings();
	
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CSoundManager::Init:Setting Listener parms:");
		return false;
	}

	//Print SoundSystem Info
	SPrintInfo();
	ComPrintf("CSound::Init OK\n");
	return true;
}

//======================================================================================
//======================================================================================

/*
======================================
Update Listener pos
======================================
*/
void CSoundManager::UpdateListener(const vector_t &pos,
								   const vector_t &velocity,
								   const vector_t &forward,
								   const vector_t &up)
{	
	Void3d::VectorSet(m_listenerPos,pos);
	m_pListener->m_pDS3dListener->SetPosition(pos.x, pos.y, pos.z, DS3D_DEFERRED);
//	m_pListener->m_pDS3dListener->SetVelocity(velocity.x,velocity.y, velocity.z, DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetOrientation(forward.x, forward.y, forward.z, 
												 up.x,up.y, up.z, DS3D_DEFERRED);
}


/*
==========================================
check for streameable data ?
Check playing channels to see which ones are out
of range now, and silence them
==========================================
*/
void CSoundManager::RunFrame()
{
	if(m_fLastFrame > System::g_fcurTime)
		return;
	m_fLastFrame = System::g_fcurTime + 1/m_cSndFps.fval;


	m_pListener->m_pDS3dListener->CommitDeferredSettings();

	//Go through all the sound sources to play the ones in range,
	//and stop the ones out of range. out of range nonStatic sources are removed
	float listenerDist = 0.0f;
	for(int i=0;i< MAX_SOUNDSOURCES; i++)
	{
		//Play sounds now in Range
		if(m_sndSources[i].ent)
		{
			listenerDist = Void3d::VectorDistance(m_listenerPos, m_sndSources[i].ent->origin);
			
			//is it/ was it being played by a channel, and
			//is out of range now, then stop it
			if((m_sndSources[i].channel) && 
			   (listenerDist > m_sndSources[i].muteDist))
			{
//ComPrintf("SND: STOPPING %d\n", i);
				m_sndSources[i].channel->Stop();
				m_sndSources[i].channel->Destroy();
				m_sndSources[i].channel = 0;
				if(!m_sndSources[i].bStatic)
					m_sndSources[i].Reset();
			}
			else if((!m_sndSources[i].channel) && 
					(listenerDist < m_sndSources[i].muteDist))
			{
//ComPrintf("SND: PLAYING %d\n", i);
				//Start playing it
				PlaySoundSource(m_sndSources[i]);
			}
		}
	}
}

/*
======================================
Play sound at given cache and index on given channel
======================================
*/
void CSoundManager::PlaySnd3d(const ClEntity * ent,
							  int index, CacheType cache,
							  int volume, int attenuation,
							  int chantype)
{
	if(!m_bufferCache[cache][index].InUse())
	{
		ComPrintf("CSoundManager::PlaySnd3d: No sound at index %d, cache %d\n", index, cache);
		return;
	}

	//Look for a free sound source
	for(int i=0; i< MAX_SOUNDSOURCES; i++)
	{
		if(!m_sndSources[i].ent)
			break;
	}
	if(i == MAX_SOUNDSOURCES)
	{
		ComPrintf("CSoundManager::PlaySnd3d: Unable to play %s, max sounds reached\n", 
			m_bufferCache[cache][index].GetFilename());
	}

	//Look for a free channel
	for(int j=0; j<MAX_CHANNELS; j++)
	{
		if(!m_Channels[j].IsPlaying())
			break;
	}
	if(j== MAX_CHANNELS)
	{
		ComPrintf("CSoundManager::PlaySnd3d: Unable to play %s, max sounds reached\n", 
			m_bufferCache[cache][index].GetFilename());
		return;
	}

	//We found a source and a Channel object
	m_sndSources[i].channel = &m_Channels[j];
	m_sndSources[i].muteDist = GetMuteDist(volume,attenuation);
	m_sndSources[i].ent = ent;
	m_sndSources[i].flags = chantype;
	m_Channels[j].Create3d(m_bufferCache[cache][index],
						   ent->origin,
						   m_sndSources[i].muteDist);
	//Play the sound now
/*	bool loop = false;
	chantype & CHAN_LOOPING ? loop = true : loop = false;
	if(!m_Channels[i].Play(loop))
	{
		ComPrintf("SoundManager::PlaySnd3d:Error playing sound %s at index %d\n", 
				index, m_bufferCache[cache][index].GetFilename());
		m_sndSources[i].Reset();
		return;
	}
*/
}

/*
======================================
Play a Static Sound source
======================================
*/
void CSoundManager::PlaySoundSource(SndSource &source)
{
	//Find a free SoundChannel
	for(int i=0; i<MAX_CHANNELS; i++)
		if(!m_Channels[i].IsPlaying())
			break;
	if(i== MAX_CHANNELS)
	{
		ComPrintf("CSoundManager::PlayStaticSound: Unable to play %s, max sounds reached\n", 
			m_bufferCache[source.ent->sndCache][source.ent->soundIndex].GetFilename());
		return;
	}

	source.channel = &m_Channels[i];
	m_Channels[i].Create3d(m_bufferCache[source.ent->sndCache][source.ent->soundIndex],
						   source.ent->origin,
						   source.muteDist);

	bool loop = false;
	source.flags & CHAN_LOOPING ? loop = true : loop = false;
	if(!m_Channels[i].Play(loop))
	{
		ComPrintf("CSoundManager::PlayStaticSound: Error playing sound %s at index %d\n", 
					source.ent->soundIndex, 
					m_bufferCache[source.ent->sndCache][source.ent->soundIndex].GetFilename());
		source.Reset();
		return;
	}
}

/*
======================================
Play a one dimensional sound.
======================================
*/
void CSoundManager::PlaySnd2d(int index, CacheType cache,
							  int volume,
							  int chantype)
{
	if(!m_bufferCache[cache][index].InUse())
	{
		ComPrintf("CSoundManager::PlaySnd2d: no sound at index %d, cache %d\n", index, cache);
		return;
	}

	for(int i=0; i<MAX_CHANNELS; i++)
		if(!m_Channels[i].IsPlaying())
			break;
	if(i== MAX_CHANNELS)
	{
		ComPrintf("CSoundManager::PlaySnd2d: Unable to play %s, max sounds reached\n", 
			m_bufferCache[cache][index].GetFilename());
		return;
	}

	m_Channels[i].Create2d(m_bufferCache[cache][index],volume);
	bool loop = false;
	chantype & CHAN_LOOPING ? loop = true : loop = false;
	if(!m_Channels[i].Play(loop))
		ComPrintf("CSoundManager::PlaySnd2d: Error playing sound %s at index %d\n", 
					index, m_bufferCache[cache][index].GetFilename());
}

//======================================================================================
//======================================================================================

/*
======================================

======================================
*/
void CSoundManager::AddStaticSource(const ClEntity * ent)
{
	if(!m_bufferCache[ent->sndCache][ent->soundIndex].InUse())
	{
		ComPrintf("CSoundManager::AddStaticSource: no sound at index %d, cache %d\n", 
			ent->soundIndex, ent->sndCache);
		return;
	}

	//make sure we dont add duplicate sources
	int freeIndex = -1;
	for(int i=0; i< MAX_SOUNDSOURCES; i++)
	{
		if(freeIndex == -1 && !m_sndSources[i].ent)
			freeIndex = 0;
		else if(m_sndSources[i].ent == ent)
			return;
	}
	m_sndSources[freeIndex].Reset();
	m_sndSources[freeIndex].bStatic = true;
	m_sndSources[freeIndex].ent = ent;
	m_sndSources[freeIndex].flags = CHAN_LOOPING;
	m_sndSources[freeIndex].muteDist = GetMuteDist(ent->volume, ent->attenuation);
}

/*
======================================

======================================
*/
void CSoundManager::RemoveStaticSource(const ClEntity * ent)
{
	for(int i=0; i< MAX_SOUNDSOURCES; i++)
	{
		if(m_sndSources[i].ent == ent &&
		   m_sndSources[i].bStatic == true)
			break;
	}
	//didnt find source
	if(i == MAX_SOUNDSOURCES)
		return;

	//First make sure that source isn't being played
	if(m_sndSources[i].channel)
		m_sndSources[i].channel->Destroy();
	m_sndSources[i].Reset();
}


/*
======================================
Register a new sound at given index,
or load and return new index if not specified
======================================
*/
hSnd CSoundManager::RegisterSound(const char *path, CacheType cache, hSnd index)
{
	//we have to register a new sound, at the given index
	if(index != -1)
	{
		if(m_bufferCache[cache][index].InUse())
		{
			m_bufferCache[cache][index].Destroy();
			if(m_bufferCache[cache][index].Create(path))
				return index;
			return -1;
		}
	}

	//Load at first avaiable slot, or just return index if its already loaded
	int unusedSlot= -1;
	for(int i=0; i<GAME_MAXSOUNDS; i++)
	{
		if(m_bufferCache[cache][i].InUse())
		{
		    //if we found an index to the loaded sound, then just return
		   if(strcmp(path, m_bufferCache[cache][i].GetFilename()) == 0)
				return i;
		}
		else if(unusedSlot == -1)
			unusedSlot = i;
	}

	//Didnt find any space to load the wav
	if(unusedSlot == -1)
	{
		ComPrintf("CSoundManager::RegisterSound:: No slot for sound \"%s\" in cache %d\n", path, cache);
		return -1;
	}

	//found an empty space, load sound here and return index
	if(m_bufferCache[cache][unusedSlot].Create(path))
		return unusedSlot;
	return -1;
}


/*
======================================
Unregister the given sound
======================================
*/
void CSoundManager::UnregisterSound(hSnd index, CacheType cache)
{
	if(m_bufferCache[cache][index].InUse())
		m_bufferCache[cache][index].Destroy();
}

/*
======================================
Unregister all sounds in the given cache
======================================
*/
void CSoundManager::UnregisterCache(CacheType cache)
{
	for(int i=0; i< GAME_MAXSOUNDS; i++)
		UnregisterSound(i, cache);
}

/*
======================================
Destroy all files
======================================
*/
void CSoundManager::UnregisterAll()
{
	for(int i=0; i< 3; i++)
		UnregisterCache((CacheType)i);
}

//======================================================================================
//Console commands
//======================================================================================
/*
==========================================
Play a sound
==========================================
*/
void CSoundManager::SPlay(const CParms &parms)
{
	if(parms.NumTokens() < 2)
	{
		ComPrintf("Usage : sndplay <wavefile>\n");
		return;
	}

	char wavefile[COM_MAXPATH];
	parms.StringTok(1,wavefile,COM_MAXPATH);

	Util::SetDefaultExtension(wavefile,"wav");

	//Run through buffers to see if it has been registered
	hSnd index = RegisterSound(wavefile, CACHE_LOCAL);
	if(index == -1)
	{
		ComPrintf("Unable to find wavefile %s\n", wavefile);
		return;
	}

	PlaySnd2d(index,CACHE_LOCAL);
}

/*
==========================================
Stop the given channel
==========================================
*/
void CSoundManager::SStop(int channel)
{
	if(channel >= 0)
	{
		if(m_Channels[channel].IsPlaying())
		{
			m_Channels[channel].Stop();
			m_Channels[channel].Destroy();
			return;
		}
	}
	ComPrintf("Usage : sstop <channel num>\n");
}

/*
==========================================
List all the loaded sounds
==========================================
*/
void CSoundManager::SListSounds()
{
	int j;
	ComPrintf("Local Cache\n=======================\n");
	for(j=0; j<GAME_MAXSOUNDS; j++)
		if(m_bufferCache[0][j].InUse())
			m_bufferCache[0][j].PrintStats();
	ComPrintf("Game Cache\n=======================\n");
	for(j=0; j<GAME_MAXSOUNDS; j++)
		if(m_bufferCache[1][j].InUse())
			m_bufferCache[1][j].PrintStats();
	ComPrintf("Temp Cache\n=======================\n");
	for(j=0; j<GAME_MAXSOUNDS; j++)
		if(m_bufferCache[2][j].InUse())
			m_bufferCache[2][j].PrintStats();
}

/*
==========================================
Print current sounds info
==========================================
*/
void CSoundManager::SPrintInfo()
{
	DSCAPS	dsCaps;
	memset(&dsCaps,0,sizeof(DSCAPS));
	dsCaps.dwSize = sizeof(DSCAPS);

	//Set caps
	HRESULT hr = m_pDSound->GetCaps(&dsCaps);
	if(FAILED(hr))
		ComPrintf("CSound::Init Failed to Get Capabilities\n");
	else
	{
		//Print Caps
		ComPrintf("Direct Sound Capabilities:\n");
		ComPrintf("Primary Buffer:\n");
		if(dsCaps.dwFlags & DSCAPS_PRIMARY16BIT)
			ComPrintf(" supports 16bit\n");
		else
			ComPrintf(" does NOT support 16bit\n");

		if(dsCaps.dwFlags & DSCAPS_PRIMARYSTEREO)
			ComPrintf(" supports stereo\n");
		else
			ComPrintf(" does NOT supports stereo\n");

		ComPrintf("Min Sample Rate: %d\n",dsCaps.dwMinSecondarySampleRate);
		ComPrintf("Max Sample Rate: %d\n",dsCaps.dwMaxSecondarySampleRate);

	}

	m_pPrimary->PrintStats();

	ComPrintf("==== 3D Listener ====\n");

	DS3DLISTENER l3dparms;
	memset(&l3dparms,0,  sizeof(DS3DLISTENER));
	l3dparms.dwSize = sizeof(DS3DLISTENER);

	m_pListener->m_pDS3dListener->GetAllParameters(&l3dparms);

	ComPrintf("Distance Factor : %.2f\nRolloff Factor : %.2f\nDoppler Factor : %.2f\n",
				l3dparms.flDistanceFactor,l3dparms.flRolloffFactor, l3dparms.flDopplerFactor);
	ComPrintf("Listener pos : %.2f %.2f %.2f\n", l3dparms.vPosition.x, 
						l3dparms.vPosition.y, l3dparms.vPosition.x);
}

/*
==========================================
Handle Commands
==========================================
*/
void CSoundManager::HandleCommand(HCMD cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case CMD_PLAY:
		SPlay(parms);
		break;
	case CMD_STOP:
		SStop(parms.IntTok(1));
		break;
	case CMD_INFO:
		SPrintInfo();
		break;
	case CMD_LIST:
		SListSounds();
		break;
	}
}

//======================================================================================
//Cvar Handler
//======================================================================================
/*
==========================================
Set volume
==========================================
*/
bool CSoundManager::SetVolume(const CParms &parms)
{
	if(parms.NumTokens() < 2)
	{
		ComPrintf("Volume : %.2f\n", m_pPrimary->GetVolume());
		return false;
	}

	float fvol = parms.FloatTok(1);
	if(fvol < 0.0f || fvol > 10.0f)
	{
		ComPrintf("CSoundManager::SVolume: Valid range is 0.0f to 10.0f\n");
		return false;
	}
	if(m_pPrimary->SetVolume(fvol))
		return true;
	return false;
}

/*
======================================
Set the RollOFF factor
Just validate values if the listener
hasn't been created yet
======================================
*/
bool CSoundManager::SetRollOffFactor(const CParms &parms)
{
	//Just print value and return
	if(parms.NumTokens() < 2)
		return false;

	float factor = parms.FloatTok(1);
	if(factor < DS3D_MINROLLOFFFACTOR ||   factor > DS3D_MAXROLLOFFFACTOR)
	{
		ComPrintf("Valid range for Rolloff factor is %0.2f to %0.2f\n",
			DS3D_MINROLLOFFFACTOR,DS3D_MAXROLLOFFFACTOR);
		return false;
	}

	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetRolloffFactor(factor,DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetRollOffFactor:");
			return false;
		}
	}
	ComPrintf("RollOffFactor changed to = \"%f\"\n", factor);
	return true;
}

/*
======================================
Set the distance Factor
======================================
*/
bool CSoundManager::SetDistanceFactor(const CParms &parms)
{
	//Just print value and return
	if(parms.NumTokens() < 2)
		return false;

	float factor = parms.FloatTok(1);
	if(factor <= 0.0f)
	{
		ComPrintf("Distance factor should be greater than 0.0\n");
		return false;
	}

	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetDistanceFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetDistance:");
			return false;
		}
	}
	ComPrintf("DistanceFactor changed to = \"%f\"\n", factor);
	return true;
}

/*
======================================
Set the doppler factor
======================================
*/
bool CSoundManager::SetDopplerFactor(const CParms &parms)
{
	//Just print value and return
	if(parms.NumTokens() < 2)
		return false;
	
	float factor = parms.FloatTok(1);
	if(factor < DS3D_MINDOPPLERFACTOR ||  factor > DS3D_MAXDOPPLERFACTOR)
	{
		ComPrintf("Valid range for Doppler factor is %0.2f to %0.2f\n",
					DS3D_MINDOPPLERFACTOR, DS3D_MAXDOPPLERFACTOR);
		return false;
	}

	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetDopplerFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetDopplerFactor:");
			return false;
		}
	}
	ComPrintf("DopplerFactor changed to = \"%f\"\n", factor);
	return true;
}


/*
==========================================
Handle CVar changes
==========================================
*/
bool CSoundManager::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == reinterpret_cast<CVarBase*>(&m_cVolume))
		return SetVolume(parms);
	if(cvar == reinterpret_cast<CVarBase*>(&m_cRollOffFactor))
		return SetRollOffFactor(parms);
	if(cvar == reinterpret_cast<CVarBase*>(&m_cDopplerFactor))
		return SetDopplerFactor(parms);
	if(cvar == reinterpret_cast<CVarBase*>(&m_cDistanceFactor))
		return SetDistanceFactor(parms);
	return false;
}

//======================================================================================
//======================================================================================

namespace VoidSound
{
	IDirectSound	*   GetDirectSound() { return m_pDSound; }
	CWaveManager	*	GetWaveManager() { return m_pWaveManager; }

	void PrintDSErrorMessage(HRESULT hr, char * prefix)
	{
		char error[256];
		strcpy(error,prefix);
		switch(hr)
		{
		case DSERR_ALLOCATED:
			strcat(error,"The request failed because resources, such as a priority level, were already in use by another caller"); 
			break;
		case DSERR_ALREADYINITIALIZED:
			strcat(error,"The object is already initialized.");
			break;
		case DSERR_BADFORMAT:
			strcat(error,"The specified wave format is not supported.");
			break;
		case DSERR_BUFFERLOST:
			strcat(error,"The buffer memory has been lost and must be restored.");
			break;
		case DSERR_CONTROLUNAVAIL:
			strcat(error,"The buffer control (volume, pan, and so on) requested by the caller is not available.");
			break;
		case DSERR_GENERIC:
			strcat(error,"An undetermined error occurred inside the DirectSound subsystem.");
			break;
		case DSERR_INVALIDCALL:
			strcat(error,"This function is not valid for the current state of this object.");
			break;
		case DSERR_INVALIDPARAM: 
			strcat(error,"An invalid parameter was passed to the returning function.");
			break;
		case DSERR_NOAGGREGATION:
			strcat(error,"The object does not support aggregation.");
			break;
		case DSERR_NODRIVER:
			strcat(error,"No sound driver is available for use.");
			break;
		case DSERR_NOINTERFACE:
			strcat(error,"The requested COM interface is not available.");
			break;
		case DSERR_OTHERAPPHASPRIO: 
			strcat(error,"Another application has a higher priority level, preventing this call from succeeding");
			break;
		case DSERR_OUTOFMEMORY: 
			strcat(error,"The DirectSound subsystem could not allocate sufficient memory to complete the caller's request.");
			break;
		case DSERR_PRIOLEVELNEEDED:
			strcat(error,"The caller does not have the priority level required for the function to succeed.");
			break;
		case DSERR_UNINITIALIZED: 
			strcat(error,"The IDirectSound::Initialize method has not been called or has not been called successfully before other methods were called.");
			break;
		case DSERR_UNSUPPORTED:
			strcat(error,"The function called is not supported at this time.");
			break;
		default:
			strcat(error,"Unknown Error");
			break;
		}
		ComPrintf("%s\n",error);
	}

	//Volume = 10, atten = 10 . range = 10000
	float GetMuteDist(float volume, int attenuation)
	{	return ((volume * 25) * (attenuation));
	}
}



/*
======================================
Device Enumeration. Can't find any use yet
======================================
*/
	//Check sound drivers avaiblable
/*	hr = DirectSoundEnumerate((LPDSENUMCALLBACK)EnumSoundDevices, 0);
	if(FAILED(hr))
	{
		ComPrintf("CSound::Init Failed to enumerate DirectSound Interface\n");
		return false;
	}
*/
/*
BOOL CALLBACK CSoundManager::EnumSoundDevices(LPGUID lpGuid,            
									  const char * szDesc,
									  const char * szModule,
									  void * pContext)
{
	if(szDesc)
		ComPrintf("Desc : %s\n", szDesc);
	if(szModule)
		ComPrintf("Module : %s\n", szModule);
	return true;
}
*/
