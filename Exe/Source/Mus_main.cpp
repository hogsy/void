#ifdef INCLUDE_MUSIC

#include "Sys_hdr.h"
#include "Mus_main.h"
#include "Fmod/fmod.h"		//FMOD


//Local Vars
static CVar	  *	m_pvolume=0;			//playback volume
static char		muspath[MAX_PATH];  //temp path to mus dir

//CVar validation func
static bool MusVolume(const CVar * var, int argc, char** argv); 

//Console func declaration
static void MusPlay(int argc, char** argv);
static void MusPause(int argc, char** argv);
static void MusStop(int argc, char** argv);
static void MusResume(int argc, char** argv);

const char * ErrorString(long err);

//CVar m_pvolume ("mvol","0",CVar::CVAR_INT,0,&MusVolume);
		

#define MUS_PATH "music/"

enum EMusState
{
	M_INACTIVE =0,
	M_PLAYING  =1,
	M_PAUSED   =2,
	M_STOPPED  =4
};

typedef struct MusicInfo
{
	char		name[128];
	EMusState	state;
}musInfo_t;

static musInfo_t musinfo;

//FMOD specific stuff
static FSOUND_STREAM * fmod_stream =0;


/*
=======================================
Constructor
=======================================
*/
CMusic::CMusic()
{
	musinfo.state  = M_INACTIVE;

/*	Sys_GetConsole()->RegisterCFunc("mplay",&MusPlay);
	Sys_GetConsole()->RegisterCFunc("mstop",&MusStop);
	Sys_GetConsole()->RegisterCFunc("mpause",&MusPause);
	Sys_GetConsole()->RegisterCFunc("mresume",&MusResume);
*/	
	m_pvolume = Sys_GetConsole()->RegisterCVar("mvol","0",CVar::CVAR_INT,0,&MusVolume);
}

/*
=======================================
Destructor
=======================================
*/
CMusic::~CMusic()
{
}


/*
=======================================
Init the music system
=======================================
*/

bool CMusic::Init() 
{
	sprintf(muspath,"%s/%s", CFileSystem::GetCurrentPath(), MUS_PATH);
	
	FSOUND_SetOutput(FSOUND_OUTPUT_WINMM);
	FSOUND_SetDriver(0);
	
	if(!FSOUND_Init(44100, 16, 0))
	{
		ComPrintf("CMusic::Init, Couldnt Init FSOUND: %s\n", 
						ErrorString(FSOUND_GetError()));
		return false;
	}
	ComPrintf("CMusic::Init OK\n");
	return true;
}


/*
=======================================
Shutdown the music system
=======================================
*/
bool CMusic::Shutdown()
{
//	if(fmod_stream)
//		FSOUND_Stream_Close(fmod_stream);
	MusStop(0,0);
	FSOUND_Close();

	ComPrintf("CMusic::Shutdown - OK\n");
	return true;
}


//==============================================================================
//==============================================================================

/*
=======================================
Return Error string
=======================================
*/
const char * ErrorString(long err)
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
		case FMOD_ERR_NO_A3D:			return "Tried to use A3D and not an A3D hardware card, or dll didnt exist, try another output type.";
		case FMOD_ERR_NO_EAX:			return "Tried to use an EAX command on a non EAX enabled channel or output.";
		case FMOD_ERR_CHANNEL_ALLOC:	return "Failed to allocate a new channel";
		default :						return "Unknown error";
	};
};


/*
=======================================
Play
=======================================
*/
void MusPlay(int argc, char** argv)
{
	if(argc==1)
	{
		ComPrintf("Usage - mplay <filename>\n");
		return;
	}

	if(musinfo.state != M_INACTIVE)
	{	MusStop(0,0);
	}

	strcpy(musinfo.name,muspath);
	strcat(musinfo.name,argv[1]);

	char	ext[4];		//extension
	Util_GetExtension(argv[1],ext);

	if(!strlen(ext))	//no extension entered, lets see if we can findit
	{	
		Util_FindExtension(musinfo.name,ext);
		strcat(musinfo.name,".");	
		strcat(musinfo.name,ext);
	}

	ComPrintf("CMusic:: Attempting to play %s\n",musinfo.name);

	fmod_stream = FSOUND_Stream_OpenMpeg(musinfo.name, 
										 FSOUND_LOOP_NORMAL|FSOUND_NORMAL);	
	if(!fmod_stream)
	{
		ComPrintf("MusPlay, Couldnt Open music at %s, %s\n",
						 musinfo.name, ErrorString(FSOUND_GetError()));
		memset(musinfo.name,0,sizeof(128));
		return;
	}
	
	if(FSOUND_Stream_Play(FSOUND_FREE, fmod_stream) == -1)
	{
		ComPrintf("MusPlay, Couldnt Play music at %s, %s\n",
						musinfo.name, ErrorString(FSOUND_GetError()));
		memset(musinfo.name,0,sizeof(128));
		return;
	}
	musinfo.state = M_PLAYING;

}

/*
=======================================
stop - Console Command
=======================================
*/
void MusStop(int argc, char** argv)
{
	if(musinfo.state != M_INACTIVE && fmod_stream)
	{
		FSOUND_Stream_Close(fmod_stream);
		musinfo.state = M_INACTIVE;
		ComPrintf("MusPause: Stopped %s\n",musinfo.name);
		return;
	}
	ComPrintf("MusPause: Can't stop, nothing loaded\n");
}

/*
=======================================
Pause - Console Command
=======================================
*/
void MusPause(int argc, char** argv)
{
	if(musinfo.state == M_PLAYING)
	{	
		FSOUND_Stream_SetPaused(fmod_stream,TRUE);
		musinfo.state = M_PAUSED;
		ComPrintf("MusPause: Paused %s\n",musinfo.name);
		return;
	}
	ComPrintf("MusPause: Can't paused, nothing playing\n");
}

/*
=======================================
Resume - Console command
=======================================
*/
void MusResume(int argc, char** argv)
{
	if(musinfo.state == M_PAUSED)
	{
		FSOUND_Stream_SetPaused(fmod_stream,FALSE);
		musinfo.state = M_PLAYING;
		ComPrintf("MusPause: Resumed %s\n",musinfo.name);
	}
	ComPrintf("MusPause: Can't resume, nothing paused\n");
}


bool MusVolume(const CVar * var, int argc, char** argv)
{
	return true;
}


#endif