#include "Mus_main.h"
#include "I_file.h"
#include "Mus_fmod.h"



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
	
	const char DRIVER_FMOD[] = "fmod";
	const char DRIVER_CD[] = "cdaudio";
	const char DRIVER_DMUS[] = "dmusic";

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
CMusic::CMusic() : m_cVolume("mus_vol","8", CVar::CVAR_INT,CVar::CVAR_ARCHIVE),
				   m_cDriver("mus_driver","fmod", CVar::CVAR_STRING,CVar::CVAR_ARCHIVE)
{
	m_curDriver = 0;

#ifdef INCLUDE_FMOD
	m_pFMod = new CMusFMod();
#endif

	System::GetConsole()->RegisterCVar(&m_cVolume,this);
	System::GetConsole()->RegisterCVar(&m_cDriver,this);

	System::GetConsole()->RegisterCommand("musplay", MUS_PLAY, this);
	System::GetConsole()->RegisterCommand("muspause", MUS_PAUSE, this);
	System::GetConsole()->RegisterCommand("musstop", MUS_STOP, this);
	System::GetConsole()->RegisterCommand("musresume", MUS_RESUME, this);
	System::GetConsole()->RegisterCommand("musstats", MUS_STATS, this);
}

CMusic::~CMusic()
{	
	Shutdown();
	m_curDriver = 0;

#ifdef INCLUDE_FMOD
	delete m_pFMod;
	m_pFMod = 0;
#endif
}
	
/*
==========================================
Initialize Music driver
==========================================
*/
bool CMusic::Init()
{
	if(!strcmp(m_cDriver.string,DRIVER_FMOD))
	{
#ifdef INCLUDE_FMOD
		m_curDriver = m_pFMod;
		return m_curDriver->Init();
#endif
	}
	else if(!strcmp(m_cDriver.string,DRIVER_CD))
	{
	}
	else if(!strcmp(m_cDriver.string,DRIVER_DMUS))
	{
	}
	return true;
}

/*
==========================================
Shutdown the music driver
==========================================
*/
void CMusic::Shutdown()
{	
	if(m_curDriver)
	{	m_curDriver->Shutdown();
		m_curDriver =0;
	}
}

/*
==========================================
Handle one of the registered commands
==========================================
*/
void CMusic::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case MUS_PLAY:
		Play(numArgs,szArgs);
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
bool CMusic::HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)
{
	if(cvar == &m_cVolume)
		return Volume(&m_cVolume,numArgs,szArgs);
    else if(cvar == &m_cDriver)
		return Driver(&m_cDriver,numArgs,szArgs);
	return false;
}

/*
==========================================

==========================================
*/
CMusDriver * CMusic::GetMusicDriver()
{	return m_curDriver;
}

//======================================================================================
//======================================================================================

void CMusic::Play(int argc, char** argv)
{
	if(!m_curDriver)
		return;
	if(argc > 1 && argv[1])
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
}

void CMusic::Pause()
{
	if(m_curDriver)
		m_curDriver->SetPause(true);
}

void CMusic::Stop()
{
	if(m_curDriver)
		m_curDriver->Stop();
}

void CMusic::Resume()
{
	if(m_curDriver)
		m_curDriver->SetPause(false);
}

void CMusic::PrintStats()
{
	if(m_curDriver)
	{
	}
}

//======================================================================================
//======================================================================================

bool CMusic::Volume(const CVar * var, int argc, char** argv)
{	
	if(m_curDriver)
	{
	}
	return true;
}

bool CMusic::Driver(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		if(!strcmp(argv[1], DRIVER_FMOD))
		{
		}
		else if(!strcmp(argv[1], DRIVER_CD))
		{
		}
		else if(!strcmp(argv[1], DRIVER_DMUS))
		{
		}
	}
	else
	{
		ComPrintf("Valid Music drivers are :\n");
		ComPrintf("%s, %s, %s\n", DRIVER_FMOD, DRIVER_CD, DRIVER_DMUS);
	}
	return false;
}
