#include "Snd_main.h"
#include "Snd_hdr.h"
#include "Snd_wave.h"
#include "Snd_buf.h"
#include "Com_util.h"

namespace
{
	enum
	{
		CMD_PLAY  = 1,
		CMD_STOP  = 2,
		CMD_INFO  = 5,
		CMD_LIST  = 6
	};
	//Direct sound object.
	IDirectSound	*	m_pDSound = 0;	
}

using namespace VoidSound;

//======================================================================================
/*
==========================================
Constructor/Destructor
==========================================
*/
CSoundManager::CSoundManager() : m_cVolume("s_vol", "9", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cHighQuality("s_highquality", "1", CVAR_BOOL, CVAR_ARCHIVE),
								 m_cRollOffFactor("s_rolloff", "1.0", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cDopplerFactor("s_doppler", "1.0", CVAR_FLOAT, CVAR_ARCHIVE),
								 m_cDistanceFactor("s_distance", "15.0", CVAR_FLOAT, CVAR_ARCHIVE)
{
	m_pListener = 0;	
	m_pPrimary = new CPrimaryBuffer;
	m_Buffers =  new CSoundBuffer[MAX_SOUNDS];
	m_Channels = new CSoundChannel[MAX_CHANNELS];
	
	m_numBuffers = 0;

	m_bHQSupport=false;
	m_bStereoSupport= false;

	System::GetConsole()->RegisterCVar(&m_cVolume,this);
	System::GetConsole()->RegisterCVar(&m_cHighQuality,this);
	System::GetConsole()->RegisterCVar(&m_cRollOffFactor,this);
	System::GetConsole()->RegisterCVar(&m_cDopplerFactor,this);
	System::GetConsole()->RegisterCVar(&m_cDistanceFactor,this);
	
	System::GetConsole()->RegisterCommand("splay",CMD_PLAY,this);
	System::GetConsole()->RegisterCommand("sstop",CMD_STOP,this);
	System::GetConsole()->RegisterCommand("sinfo",CMD_INFO,this);
	System::GetConsole()->RegisterCommand("slist",CMD_LIST,this);
}

CSoundManager::~CSoundManager()
{	Shutdown();
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

	//Check sound drivers avaiblable
/*	hr = DirectSoundEnumerate((LPDSENUMCALLBACK)EnumSoundDevices, 0);
	if(FAILED(hr))
	{
		ComPrintf("CSound::Init Failed to enumerate DirectSound Interface\n");
		return false;
	}
*/
	// Initialize the DirectSound object.
	// Defaulting to Primary Sound Driver right now
	// FIX-ME
	hr = m_pDSound->Initialize(0);
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed Initialize Directsound\n");
		m_pDSound->Release();
		return false; 
	}

	// Set the cooperative level.
	hr = m_pDSound->SetCooperativeLevel(System::GetHwnd(),
										DSSCL_PRIORITY);
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed SetCoopLevel\n");
		Shutdown(); 
		return false; 
	}

	if(!SPrintInfo())
	{
		Shutdown();
		return false;
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

	IDirectSound3DListener * lpd3dlistener = m_pPrimary->Create(pcmwf);
	if(!lpd3dlistener)
	{
		Shutdown();
		return false;
	}

//	lpd3dlistener->Release();


	m_pListener = new C3DListener(lpd3dlistener);
/*	m_pListener->m_pDS3dListener->SetDistanceFactor(m_cDistanceFactor.fval,DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetRolloffFactor(m_cRollOffFactor.fval,DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetDopplerFactor(m_cDopplerFactor.fval,DS3D_DEFERRED);
	hr = m_pListener->m_pDS3dListener->CommitDeferredSettings();
	if(FAILED(hr))
	{
		PrintDSErrorMessage(hr,"CSoundManager::Init:Setting Listener parms:");
		Shutdown();
		return false;
	}
*/
	ComPrintf("CSound::Init OK\n");
	return true;
}

/*
======================================
Shutdown
======================================
*/
void CSoundManager::Shutdown()
{
	if(m_pListener)
	{
		delete m_pListener;
		m_pListener=0;
	}

	if(m_Buffers)
	{
		delete [] m_Buffers;
		m_Buffers= 0;
	}
	
	if(m_Channels)
	{
		delete [] m_Channels;
		m_Channels = 0;
	}
	
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
}

/*
==========================================
Nothing to do here rignt now
might need to check for streameable data
once wave streams are supported
==========================================
*/
void CSoundManager::RunFrame()
{
//	m_pListener->m_pDS3dListener->CommitDeferredSettings();
}

/*
======================================

======================================
*/
void CSoundManager::UpdateListener(const vector_t &pos,
								   const vector_t &velocity,
								   const vector_t &forward,
								   const vector_t &up)
{	
	m_pListener->m_pDS3dListener->SetPosition(pos.x, pos.y, pos.z, DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetVelocity(velocity.x,velocity.y, velocity.z, DS3D_DEFERRED);
	m_pListener->m_pDS3dListener->SetOrientation(forward.x, forward.y, forward.z, 
												 up.x,up.y, up.z, DS3D_DEFERRED);
}

/*
======================================
Update Sound position
The SoundManager needs to automatically stop sounds out of range
======================================
*/
void CSoundManager::UpdateSnd(int index, vector_t * pos, vector_t * velocity)
{
}

/*
==========================================
Play sound at index blah, at this channel
hook this up with an entity, for speed and origin
==========================================
*/
void CSoundManager::PlaySnd(int index, int channel,
							  const vector_t * origin,
							  const vector_t * velocity,
							  bool looping)
{
	//Find an unused channel
	for(int i=0; i<MAX_CHANNELS; i++)
		if(!m_Channels[i].IsPlaying())
			break;
	if(i== MAX_CHANNELS)
	{
		ComPrintf("CSoundManager::Play: Unable to play %s, max sounds reached\n", m_Buffers[index].GetFilename());
		return;
	}

	m_Channels[i].Create(m_Buffers[index],origin,velocity);
	bool ret = false;
	if(looping == true)
		ret = m_Channels[i].Play(true);
	else
		ret = m_Channels[i].Play(false);
	
	if(ret)
	{
//TEMP, dont really need this info during gameplay
//		ComPrintf("Playing %s at channel %d\n", m_Buffers[index].GetFilename(),i);
	}
}


/*
==========================================
Register a sound
==========================================
*/
int  CSoundManager::RegisterSound(const char * path)
{
	if(m_Buffers[m_numBuffers].Create(path))
		return (m_numBuffers++);
	return 0;
}

/*
==========================================
Destroy all Buffers and free all channels
==========================================
*/
void CSoundManager::UnregisterAll()
{
	int i;
	for(i=0; i< m_numBuffers; i++)
		m_Channels[i].Destroy();
	for(i=0;i<MAX_SOUNDS;i++)
		m_Buffers[i].Destroy();
}

//======================================================================================
//Console commands and CVars
//======================================================================================
/*
==========================================
Play a sound
==========================================
*/
void CSoundManager::SPlay(const char * arg)
{
	if(arg)
	{
		char wavfile[COM_MAXPATH];

		strcpy(wavfile,arg);
		Util::SetDefaultExtension(wavfile,"wav");

		//Run through the buffers to check if it has been registered
		for(int i=0; i<m_numBuffers;i++)
		{
			if(strcmp(wavfile, m_Buffers[i].GetFilename()) == 0)
				break;
		}

		//Create a new buffer
		if(i == m_numBuffers)
		{
			i = RegisterSound(wavfile);
			if(!i)
				return;
		}
		//Play the sound
		PlaySnd(i);
		return;
	}
	ComPrintf("Usage : splay <wavepath>\n");
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
	for(int i=0;i<m_numBuffers;i++)
		m_Buffers[i].PrintStats();

	//Currently playing
//	ComPrintf("Currently playing %d channels\n", m_channelsInUse);
}

/*
==========================================
Print current sounds info
==========================================
*/
bool CSoundManager::SPrintInfo()
{
	DSCAPS	dsCaps;
	memset(&dsCaps,0,sizeof(DSCAPS));
	dsCaps.dwSize = sizeof(DSCAPS);

	//Set caps
	HRESULT hr = m_pDSound->GetCaps(&dsCaps);
	if(FAILED(hr))
	{
		ComPrintf("CSound::Init Failed to Get Capabilities\n");
		Shutdown();
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

	ComPrintf("Min Sample Rate: %d\n",dsCaps.dwMinSecondarySampleRate);
	ComPrintf("Max Sample Rate: %d\n",dsCaps.dwMaxSecondarySampleRate);

	m_pPrimary->PrintStats();
	return true;
}


/*
==========================================
Set volume
==========================================
*/
bool CSoundManager::SetVolume(const CParms &parms)
{
	long lvol = 0;
	float fvol = 0.0f;

	if(parms.NumTokens() < 2)
	{
		lvol = m_pPrimary->GetVolume();
		fvol = (5000.0 - lvol)/500.0;
		ComPrintf("Volume : %.2f (%d)\n", fvol,lvol);
		return false;
	}

	fvol = parms.FloatTok(1);
	if(fvol < 0.0f || fvol > 10.0f)
	{
		ComPrintf("CSoundManager::SVolume: Valid range is 0.0f to 10.0f\n");
		return false;
	}
	lvol = -(5000 - fvol*500);
	if(m_pPrimary->SetVolume(lvol))
	{
		ComPrintf("Volume changed to %.2f (%d)\n", fvol, lvol);
		return true;
	}
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
	{
		ComPrintf("%s = \"%s\"\n", m_cRollOffFactor.name, m_cRollOffFactor.string);
		return false;
	}

	float factor = parms.FloatTok(1);
	if(factor < DS3D_MINROLLOFFFACTOR ||
	   factor > DS3D_MAXROLLOFFFACTOR)
	{
		ComPrintf("Valid range for Rolloff factor is %0.2f to %0.2f\n",
			DS3D_MINROLLOFFFACTOR,DS3D_MAXROLLOFFFACTOR);
		return false;
	}

/*	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetRolloffFactor(factor,DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetRollOffFactor:");
			return false;
		}
	}
*/
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
	{
		ComPrintf("%s = \"%s\"\n", m_cDistanceFactor.name, m_cDistanceFactor.string);
		return false;
	}

	float factor = parms.FloatTok(1);
	if(factor <= 0.0f)
	{
		ComPrintf("Distance factor should be greater than 0.0\n");
		return false;
	}

/*	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetDistanceFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetDistance:");
			return false;
		}
	}
*/
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
	{
		ComPrintf("%s = \"%s\"\n", m_cDopplerFactor.name, m_cDopplerFactor.string);
		return false;
	}

	float factor = parms.FloatTok(1);
	if(factor < DS3D_MINDOPPLERFACTOR ||
	   factor > DS3D_MAXDOPPLERFACTOR)
	{
		ComPrintf("Valid range for Doppler factor is %0.2f to %0.2f\n",
			DS3D_MINDOPPLERFACTOR, DS3D_MAXDOPPLERFACTOR);
		return false;
	}

/*	if(m_pListener)
	{
		HRESULT hr = m_pListener->m_pDS3dListener->SetDopplerFactor(factor, DS3D_DEFERRED);
		if(FAILED(hr))
		{
			PrintDSErrorMessage(hr,"CSoundManager::SetDopplerFactor:");
			return false;
		}
	}
*/
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
		SPlay(parms.UnsafeStringTok(1));
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
//======================================================================================

namespace VoidSound
{
	IDirectSound	*   GetDirectSound() { return m_pDSound; }

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
}

/*
======================================
Device Enumeration. Can't find any use yet
======================================
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
