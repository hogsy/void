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

	IDirectSound	*	m_pDSound = 0;	// The global sound object.
}

using namespace VoidSound;

//======================================================================================
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CSoundManager::CSoundManager() : m_cVolume("s_vol", "9", CVar::CVAR_INT, CVar::CVAR_ARCHIVE),
								 m_cHighQuality("s_highquality", "1", CVar::CVAR_BOOL, CVar::CVAR_ARCHIVE)
{
	m_pPrimary = new CPrimaryBuffer;
	m_Buffers =  new CSoundBuffer[MAX_SOUNDS];
	m_Channels = new CSoundChannel[MAX_CHANNELS];
	
	m_numBuffers = 0;
	m_channelsInUse =0;

	m_bHQSupport=false;
	m_bStereoSupport= false;

	System::GetConsole()->RegisterCVar(&m_cVolume,this);
	System::GetConsole()->RegisterCVar(&m_cHighQuality,this);

	System::GetConsole()->RegisterCommand("splay",CMD_PLAY,this);
	System::GetConsole()->RegisterCommand("sstop",CMD_STOP,this);
	System::GetConsole()->RegisterCommand("sinfo",CMD_INFO,this);
	System::GetConsole()->RegisterCommand("slist",CMD_LIST,this);
}

CSoundManager::~CSoundManager()
{
	delete [] m_Buffers;
	delete [] m_Channels;
	delete m_pPrimary;
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
		m_pDSound->Release();

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

	m_pPrimary->Create(pcmwf);

	ComPrintf("CSound::Init OK\n");

//	RegisterSound("sounds/loop.wav");
//	RegisterSound("sounds/hum.wav");
	return true;
}


/*
==========================================
shutdown the system
==========================================
*/
void CSoundManager::Shutdown()
{
	UnregisterAll();
	
	m_pPrimary->Destroy();

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

/*
==========================================
Play sound at index blah, at this channel
==========================================
*/
void CSoundManager::Play(hSnd index, int channel)
{
	for(int i=0; i<MAX_CHANNELS; i++)
		if(!m_Channels[i].IsPlaying())
			break;

	if(i== MAX_CHANNELS)
	{
		ComPrintf("CSoundManager::Play: Unable to play %s, max sounds reached\n", m_Buffers[index].GetFilename());
		return;
	}

	m_Channels[i].Create(m_Buffers[index]);

	bool ret = false;
	if(channel == CHAN_LOOP)
		ret = m_Channels[i].Play(true);
	else
		ret = m_Channels[i].Play(false);
	
	if(ret)
	{
//TEMP, dont really need this info during gameplay
//		ComPrintf("Playing %s at channel %d\n", m_Buffers[index].GetFilename(),i);
		m_channelsInUse ++;
	}
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
		SPlay(parms.StringTok(1));
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
			if(!RegisterSound(wavfile))
				return;
		}
		//Play the sound
		Play(i);
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
			m_channelsInUse--;
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
	ComPrintf("Currently playing %d channels\n", m_channelsInUse);
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
bool CSoundManager::SVolume(const CParms &parms)
{
	float fvol = 0.0f;
	long  lvol = 0;
	int numParms = parms.NumTokens();
	if(numParms==1)
	{
		lvol = m_pPrimary->GetVolume();
		fvol = (5000 - lvol)/500;
		ComPrintf("Volume : %.2f (%d)\n", fvol,lvol);
		return false;
	}
	else if(numParms > 1)
	{
		fvol = parms.FloatTok(1);
		if(fvol < 0.0f || fvol > 10.0f)
		{
			ComPrintf("CSoundManager::SVolume: Valid range is 0.0f to 10.0f\n");
			return false;
		}
		lvol = -(5000 - fvol*500);
		if(m_pPrimary->SetVolume(lvol))
		{
			ComPrintf("CSoundManager::SVolume: Changed to %.2f (%d)\n", fvol, lvol);
			return true;
		}
		return false;
	}
	return false;
}


/*
==========================================
Handle CVar changes
==========================================
*/
bool CSoundManager::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == (CVarBase *)&m_cVolume)
		return SVolume(parms);
	return false;
}

//======================================================================================
//======================================================================================
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
