#include "Mus_main.h"
#include "Mus_cd.h"
#include "I_file.h"

#include "fmod/fmod.h"
#include "fmod/fmod_errors.h"


//======================================================================================
//======================================================================================

namespace VoidMusic
{
	const int MAXCHANNELS = 16;
}

namespace
{
	const char MUSICPATH[] = "music/";

	const float MAX_VOLUME = 10.0f;
	const float MIN_VOLUME = 0.0f;
	
	enum
	{
		MUS_PLAY  = 1,
		MUS_STOP  = 2,
		MUS_PAUSE = 3,
		MUS_RESUME= 4,
		MUS_STATS = 5
	};
}

using namespace VoidMusic;

//======================================================================================
//======================================================================================

/*
==========================================
Constructor/Destructor
==========================================
*/
CMusic::CMusic() : m_cVolume("mus_vol","8", CVAR_INT,CVAR_ARCHIVE)
{
	m_bFMod  = false;
	m_pStream = 0;
	m_mp3Chan= -1;

	m_pCDAudio = new CMusCDAudio();

	System::GetConsole()->RegisterCVar(&m_cVolume,this);

	System::GetConsole()->RegisterCommand("mus_play", MUS_PLAY, this);
	System::GetConsole()->RegisterCommand("mus_pause", MUS_PAUSE, this);
	System::GetConsole()->RegisterCommand("mus_stop", MUS_STOP, this);
	System::GetConsole()->RegisterCommand("mus_resume", MUS_RESUME, this);
	System::GetConsole()->RegisterCommand("mus_info", MUS_STATS, this);
}

CMusic::~CMusic()
{	
	Shutdown();
	
	delete m_pCDAudio;
	m_pCDAudio= 0;
}
	
/*
==========================================
Initialize Music drivers
==========================================
*/
bool CMusic::Init()
{
	//Initialize FMOD

	if (FSOUND_GetVersion() != FMOD_VERSION)
	{
		ComPrintf("CMusic::Init:: Incorrect DLL version for FMOD: Need %.2f\n",FMOD_VERSION);
		return false;
	}

	FSOUND_SetOutput(FSOUND_OUTPUT_DSOUND);
	FSOUND_SetBufferSize(200);
	FSOUND_SetDriver(0);

	if (!FSOUND_Init(44100, 1, 0))
	{
		ComPrintf("CMusic::Init:: Error in FSOUND_Init(): %s\n", FMOD_ErrorString(FSOUND_GetError()));
		FSOUND_Close();
		return false;
	}
	m_bFMod = true;
	return m_pCDAudio->Init();
}

/*
==========================================
Shutdown the music driver
==========================================
*/
void CMusic::Shutdown()
{	
	if(m_bFMod)
	{
		StopMp3();
		FSOUND_Close();
		m_bFMod = false;
	}
	m_pCDAudio->Shutdown();
}

void CMusic::PlayMp3(const char * szFile)
{
	if(!m_bFMod)
	{
		ComPrintf("CMusic::PlayMp3: Error. Fmod inactive\n");
		return;
	}
	
	StopMp3();

	char path[COM_MAXPATH];
	sprintf(path,"%s/Music/%s", System::GetCurGamePath(), szFile);

	m_pStream = FSOUND_Stream_OpenMpeg(path, FSOUND_NORMAL | FSOUND_LOOP_NORMAL);	
	if (!m_pStream)
	{
		ComPrintf("CMusic::PlayMp3:Open: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return;
	}

	m_mp3Chan = FSOUND_Stream_Play(FSOUND_FREE, m_pStream);
	if(m_mp3Chan == -1)
	{
		ComPrintf("CMusic::PlayMp3:Play: %s\n", FMOD_ErrorString(FSOUND_GetError()));
		return;
	}

	FSOUND_SetVolume(m_mp3Chan,255);
	FSOUND_SetSFXMasterVolume(255);
	ComPrintf("CMUSIC: Playing %s\n", szFile);
}

void CMusic::StopMp3()
{
	if(m_mp3Chan != -1)
	{
		m_mp3Chan = -1;
		FSOUND_Stream_Stop(m_pStream);
		FSOUND_Stream_Close(m_pStream);

ComPrintf("CMusic::Stopped MP3\n");
	}
}

/*
==========================================
Handle one of the registered commands
==========================================
*/
void CMusic::HandleCommand(int cmdId, const CParms &parms)
{
	switch(cmdId)
	{
	case MUS_PLAY:
//		Play(parms.UnsafeStringTok(1));
		break;
	case MUS_STOP:
		Stop();
		break;
	case MUS_PAUSE:
		Pause();
		break;
	case MUS_RESUME:
		Resume();
		break;
	case MUS_STATS:
		PrintStats();
		break;
	}
}

/*
==========================================
Handle Chantes to CVars
==========================================
*/
bool CMusic::HandleCVar(const CVarBase * cvar, const CStringVal &strVal)
{
//	if(cvar == &m_cVolume)
//		return Volume(&m_cVolume,parms);
	return false;
}

//======================================================================================
//======================================================================================

void CMusic::Play(const char* arg)
{
/*	if(argc > 1 && argv[1])
	{
		char	filename[COM_MAXPATH];
		char	ext[4];		//extension

		memset(filename,0,COM_MAXPATH);
		memset(ext,0,4);

		sprintf(filename,"%s%s",MUSICPATH,argv[1]);

		Util::ParseExtension(ext,4,filename);

		//no extension entered, lets see if we can findit
		if(!strlen(ext))	
		{	
			if(!FileUtil::FindFileExtension(ext,4,filename))
			{	
				ComPrintf("CMusic::Play: Unable to play %s, file doesnt exist\n", filename);
				return;
			}
			strcat(filename,".");
			strcat(filename,ext);
		}

#ifdef INCLUDE_FMOD		
		if(m_curDriver == m_pFMod)
		{
			if(strcmp(ext,"mp3"))
			{
				ComPrintf("CMusic::Play: Unable to play %s, Driver only plays Mp3 files\n", filename);
				return;
			}
			m_curDriver->Play(filename);
		}
#endif
	}
*/
}

void CMusic::Pause()
{
}

void CMusic::Stop()
{
}

void CMusic::Resume()
{
}

void CMusic::PrintStats()
{
}

//======================================================================================
//======================================================================================

bool CMusic::Volume(const CVar * var, const CParms &parms)
{	return true;
}

void CMusic::HandleMCIMsg(uint &wParam, long &lParam)
{
	if(m_pCDAudio)
		HandleMCIMsg(wParam,lParam);
}

