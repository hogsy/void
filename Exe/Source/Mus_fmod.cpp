#include "Mus_fmod.h"

using namespace VoidMusic;

//	FSOUND_OUTPUT_DSOUND

//======================================================================================
//======================================================================================

/*
==========================================

==========================================
*/
CMusFMod::CMusFMod()
{
	m_pStream = 0;
	m_volume = 0;
}

CMusFMod::~CMusFMod()
{
}

/*
==========================================

==========================================
*/
bool CMusFMod::Init()
{
	FSOUND_SetOutput(FSOUND_OUTPUT_WINMM);
	FSOUND_SetDriver(0);
	FSOUND_SetHWND(System::GetHwnd());
	
	if(!FSOUND_Init(44100, MAXCHANNELS, 0))
	{
		ComPrintf("CMusFMod::Init, Couldnt Init FSOUND: %s\n", ErrorMessage(FSOUND_GetError()));
		return false;
	}
	ComPrintf("CMusFMod::Init OK\n");
	return true;
}

/*
==========================================

==========================================
*/
bool CMusFMod::Shutdown()
{
	Stop();
	FSOUND_Close();
	ComPrintf("CMusFMod::Shutdown: OK\n");
	return true;
}

//======================================================================================
//======================================================================================

/*
==========================================

==========================================
*/
bool CMusFMod::Play(char * trackname)
{
	Stop();

	ComPrintf("CMusFMod:: Attempting to play %s\n",trackname);

	m_pStream = FSOUND_Stream_OpenMpeg(trackname, 
									 FSOUND_2D|FSOUND_LOOP_NORMAL);	
	if(!m_pStream)
	{
		ComPrintf("CMusFMod:: Couldnt open %s, error : %s\n", trackname, ErrorMessage(FSOUND_GetError()));
		memset(m_trackName,0,COM_MAXPATH);
		return false;
	}
	
	if(FSOUND_Stream_Play(FSOUND_FREE, m_pStream) == -1)
	{
		ComPrintf("CMusFMod, Couldnt play %s, error : %s\n", trackname, ErrorMessage(FSOUND_GetError()));
		memset(m_trackName,0,COM_MAXPATH);
		return false;
	}
	m_eState = M_PLAYING;
	strcpy(m_trackName, trackname);
	return true;
}

/*
==========================================

==========================================
*/
bool CMusFMod::SetPause(bool pause)
{
	if(pause && (m_eState == M_PLAYING))
	{
		FSOUND_Stream_SetPaused(m_pStream,TRUE);
		m_eState = M_PAUSED;
		ComPrintf("CMusFMod: Paused %s\n",m_trackName);
		return true;
	}
	else if(!pause && (m_eState == M_PAUSED))
	{
		FSOUND_Stream_SetPaused(m_pStream,FALSE);
		m_eState = M_PLAYING;
		ComPrintf("CMusFMod: Resumed %s\n",m_trackName);
		return true;
	}
	return false;
}

/*
==========================================

==========================================
*/
bool CMusFMod::Stop()
{
	if(m_eState != M_INACTIVE && m_pStream)
	{
		FSOUND_Stream_Close(m_pStream);
		m_eState = M_INACTIVE;
		ComPrintf("CMusFMod: Stopped %s\n",	m_trackName);
		m_pStream = 0;
		return true;
	}
	return false;
}

/*
==========================================

==========================================
*/
void CMusFMod::PrintStats()
{
}


/*
==========================================

==========================================
*/
void CMusFMod::SetVolume(float vol)
{
}

/*
==========================================

==========================================
*/
float CMusFMod::GetVolume() const
{	return (float)m_volume;
}

/*
==========================================

==========================================
*/
const char * CMusFMod::ErrorMessage(long err)
{
	switch (err)
	{
		case FMOD_ERR_NONE:				return "No errors";
		case FMOD_ERR_BUSY:				return "Cannot call this command after FSOUND_Init.  Call FSOUND_Close first.";
		case FMOD_ERR_UNINITIALIZED:	return "This command failed because FSOUND_Init was not called";
		case FMOD_ERR_PLAY:				return "Playing the sound failed.";
		case FMOD_ERR_INIT:				return "Error initializing output device.";
		case FMOD_ERR_ALLOCATED:		return "The output device is already in use and cannot be reused.";
		case FMOD_ERR_OUTPUT_FORMAT:	return "Soundcard does not support the features needed for this soundsystem (16bit stereo output)";
		case FMOD_ERR_COOPERATIVELEVEL:	return "Error setting cooperative level for hardware.";
		case FMOD_ERR_CREATEBUFFER:		return "Error creating hardware sound buffer.";
		case FMOD_ERR_FILE_NOTFOUND:	return "File not found";
		case FMOD_ERR_FILE_FORMAT:		return "Unknown file format";
		case FMOD_ERR_FILE_BAD:			return "Error loading file";
		case FMOD_ERR_MEMORY:			return "Not enough memory ";
		case FMOD_ERR_VERSION:			return "The version number of this file format is not supported";
		case FMOD_ERR_INVALID_MIXER:	return "Incorrect mixer selected";
		case FMOD_ERR_INVALID_PARAM:	return "An invalid parameter was passed to this function";
		case FMOD_ERR_NO_EAX:			return "Tried to use an EAX command on a non EAX enabled channel or output.";
		case FMOD_ERR_CHANNEL_ALLOC:	return "Failed to allocate a new channel";
		default :						return "Unknown error";
	};
}

