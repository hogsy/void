#include "Mus_cd.h"
#include "Mus_main.h"

namespace
{
	enum
	{
		CD_PLAY   = 1,
		CD_PAUSE  = 2,
		CD_RESUME = 3,
		CD_STOP	  = 4,
		CD_STATS  = 5,
		CD_EJECT  = 6,
		CD_CLOSE  = 7
	};
}
using namespace VoidMusic;

//======================================================================================
//======================================================================================

CMusCDAudio::CMusCDAudio()
{
	m_mciDevice=0;	//mci device
	m_numTracks=0;

	m_curTrack=0;
	m_curPos = 0;

	m_cdInserted = false;
	
	m_eState = M_INACTIVE;

	System::GetConsole()->RegisterCommand("cdplay", CD_PLAY, this);
	System::GetConsole()->RegisterCommand("cdpause", CD_PAUSE, this);
	System::GetConsole()->RegisterCommand("cdstop", CD_STOP, this);
	System::GetConsole()->RegisterCommand("cdresume", CD_RESUME, this);
	System::GetConsole()->RegisterCommand("cdinfo", CD_STATS, this);
	System::GetConsole()->RegisterCommand("cdeject", CD_EJECT, this);
	System::GetConsole()->RegisterCommand("cdclose", CD_CLOSE, this);
}

CMusCDAudio::~CMusCDAudio()
{
	if(m_eState != M_INACTIVE)
		Shutdown();
}

/*
==========================================
Initilize the cdplayer
==========================================
*/
bool  CMusCDAudio::Init()
{
	m_hwnd = System::GetHwnd();

	DWORD dwRet = 0;
	DWORD dwFlags = 0;
	
	//Enumerate cd devices, and pick one
/*	DWORD numDevices;
	MCI_SYSINFO_PARMS mciInfo;
	
	mciInfo.wDeviceType = MCI_DEVTYPE_CD_AUDIO;
	mciInfo.lpstrReturn = (LPSTR)&numDevices;
	mciInfo.dwRetSize   = sizeof(DWORD);

	dwFlags = MCI_SYSINFO_QUANTITY;		//First get the quantity of cd audio devices

	dwRet =  mciSendCommand(0, MCI_SYSINFO,  dwFlags, (DWORD) &mciInfo);
	if(dwRet)
	{
		PrintErrorMessage(dwRet,"CMusCDAudio::Init: Failed to enumerate:");
		return false;
	}
	ComPrintf("Found %d cd devices\n", numDevices);

	//Get their names
	char devicename[32];
	for(int i=0; i < numDevices; i++)
	{
		memset(devicename,0,32);
		dwFlags = MCI_SYSINFO_NAME;
		
		mciInfo.wDeviceType = MCI_DEVTYPE_CD_AUDIO;
		mciInfo.lpstrReturn = devicename;
		mciInfo.dwRetSize = 32;
		mciInfo.dwNumber = i+1;
		
		dwRet =  mciSendCommand(0, MCI_SYSINFO,  dwFlags, (DWORD) &mciInfo);
		if(dwRet)
		{
			PrintErrorMessage(dwRet,"CMusCDAudio::Init: Failed to get name:");
			return false;
		}
		ComPrintf("Device name %d: %s\n", i+1, devicename);
	}
*/
	//Open it
	MCI_OPEN_PARMS mciOpenParms;
	dwFlags  = MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID;

	// Opens a device by specifying a device-type constant.
	memset(&mciOpenParms,0,sizeof(MCI_OPEN_PARMS));
	mciOpenParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
	mciOpenParms.lpstrElementName = 0;
//	mciOpenParms.lpstrElementName = devicename;

	dwRet = mciSendCommand(0,MCI_OPEN,dwFlags,(DWORD)&mciOpenParms);
	if(dwRet)
	{
		PrintErrorMessage(dwRet,"CMusCDAudio::Init:Open");
		return false;
	}

	// The device opened successfully; get the device ID.
	m_mciDevice = mciOpenParms.wDeviceID;		
	m_eState = M_STOPPED;

	if(!SetTimeFormat(MCI_FORMAT_TMSF))
	{
		ComPrintf("CMusCDAudio::Init:TimeFormat set failed");
		Shutdown();
		return false;
	}

	//Get number of tracks if there is a CD there
	if(IsCDInserted())
		m_numTracks =GetNumTracks();
	
	ComPrintf("CMusCDAudio::Init:OK\n");
	return true;
}

/*
==========================================
Close/Shutdown
==========================================
*/
bool  CMusCDAudio::Shutdown()
{
	if(!m_mciDevice)
		return true;

	if(m_eState != M_STOPPED)
		Stop();

	m_eState = M_INACTIVE;

	MCI_GENERIC_PARMS mciGenericParms;

	DWORD ret = mciSendCommand(m_mciDevice,MCI_CLOSE,0, (DWORD)&mciGenericParms);
	if(ret)
	{
		PrintErrorMessage(ret,"CMusCDAudio::Shutdown");
		return false;
	}
	m_mciDevice = 0;
	return true;
}

/*
==========================================
Play a track
==========================================
*/
void CMusCDAudio::Play(const char * trackname)
{
	if(!IsCDInserted())
	{
		ComPrintf("CMusCDAudio::Play:No CD inserted\n");
		return;
	}

	if(!trackname)
	{
		ComPrintf("CMusCDAudio::Play:No Trackname specified\n");
		return;
	}

	if(_strnicmp(trackname,"track",5))
	{
		ComPrintf("CMusCDAudio::Play:Bad Track specfied\n");
		return;
	}

	int trackNum=0;
	const char * p = trackname + 5;
	if(!sscanf(p,"%d",&trackNum))
	{	
		ComPrintf("CMusCDAudio::Play:Bad Trackname\n");
		return;
	}

	m_numTracks =  GetNumTracks();
	if(trackNum < 1 || trackNum > m_numTracks)
	{
		ComPrintf("CMusCDAudio::Play: Bad Track Num : %d\n", trackNum);
		return;
	}

	if(m_eState != M_STOPPED)
		Stop();

	DWORD dwRet = 0;
	MCI_PLAY_PARMS mciPlayParms;
	mciPlayParms.dwFrom = 0;
	mciPlayParms.dwTo = 0;
	mciPlayParms.dwFrom = MCI_MAKE_TMSF(trackNum,0,0,0);
	mciPlayParms.dwTo = MCI_MAKE_TMSF(trackNum + 1,0,0,0);

	ComPrintf("Attempting to play %d\n",trackNum);
    
	if (dwRet = mciSendCommand(m_mciDevice, MCI_PLAY , MCI_FROM | MCI_TO , (DWORD)&mciPlayParms)) 
    {
		PrintErrorMessage(dwRet,"CMusCDAudio::Play:");
		return;
    }
	m_curTrack = trackNum;
	ComPrintf("playing %d\n",trackNum);
	m_eState = M_PLAYING;
}

/*
==========================================
Pause if playing, resume if paused
==========================================
*/
void CMusCDAudio::SetPause(bool pause)
{	
	DWORD dwRet;
	if(pause)
	{
		//Are we playing something ?
		if(m_eState != M_PLAYING)
		{
			ComPrintf("CMusCDAudio::SetPause:Not playing anything\n");
			return;
		}

		//Save current position
		MCI_STATUS_PARMS mciStatusParms;
		
		mciStatusParms.dwItem = MCI_STATUS_POSITION ;
		mciStatusParms.dwTrack = m_curTrack;
		mciStatusParms.dwReturn = 0;
		mciStatusParms.dwCallback = 0;
        
		if (dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK, (DWORD)&mciStatusParms)) 
		{
			PrintErrorMessage(dwRet,"CMusCDAudio::SetPause:Get Pos");
			return;
		}
		//Got pos, save it
		m_curPos = mciStatusParms.dwReturn;

		//now pause it
		MCI_GENERIC_PARMS mciGenericParms;
		mciGenericParms.dwCallback = 0;

		if(dwRet = mciSendCommand(m_mciDevice, MCI_PAUSE, 0, (DWORD)&mciGenericParms))
		{
			PrintErrorMessage(dwRet,"CMusCDAudio::SetPause:");
			return;
		}
		m_eState = M_PAUSED;

		ComPrintf("Paused\n");
	}
	else
	{
		if(m_eState != M_PAUSED || !m_curPos)
		{
			ComPrintf("CMusCDAudio::SetPause:Not paused anything\n");
			return;
		}

//		Stop();

		//Play from last pos
		MCI_PLAY_PARMS mciPlayParms;
		mciPlayParms.dwFrom = 0;
		mciPlayParms.dwTo = 0;
		mciPlayParms.dwFrom = m_curPos;
		mciPlayParms.dwTo = MCI_MAKE_TMSF(m_curTrack + 1,0,0,0);

		if (dwRet = mciSendCommand(m_mciDevice, MCI_PLAY , MCI_FROM | MCI_TO, (DWORD)&mciPlayParms)) 
		{
			PrintErrorMessage(dwRet,"CMusCDAudio::Play:");
			return;
		}

		m_eState = M_PLAYING;
		m_curPos = 0;

		ComPrintf("Resumed\n");

 		//Resume, this just plays it again ?
/*		MCI_GENERIC_PARMS mciGenericParms;
		mciGenericParms.dwCallback = 0;

		if(dwRet = mciSendCommand(m_mciDevice, MCI_RESUME, 0, (DWORD)&mciGenericParms))
		{
			PrintErrorMessage(dwRet,"CMusCDAudio::SetPause:");
			return;
		}
*/
 	}
}

/*
==========================================
Stop if playing
==========================================
*/
void  CMusCDAudio::Stop()
{
	if(!m_mciDevice || m_eState == M_STOPPED)
	{
		ComPrintf("CMusCDAudio::Stop: Device is already inactive\n");
		return;
	}

	//Stop it 
	DWORD dwRet = 0;
	MCI_GENERIC_PARMS mciGenericParms;
	mciGenericParms.dwCallback = 0;

	if(dwRet = mciSendCommand(m_mciDevice, MCI_STOP, 0, (DWORD)&mciGenericParms))
	{
		PrintErrorMessage(dwRet,"CMusCDAudio::SetPause:");
		return;
	}
	m_curPos = 0;
	m_eState = M_STOPPED;
	ComPrintf("Stopped\n");
}

/*
==========================================
Check for presence of cd
==========================================
*/
bool CMusCDAudio::IsCDInserted()
{
	DWORD dwRet=0;
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_MEDIA_PRESENT;

    if (dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms)) 
    {
		PrintErrorMessage(dwRet,"CMusCDAudio::IsCDInserted:");
		return false;
    }
	if(mciStatusParms.dwReturn == FALSE)
	{
		m_cdInserted = false;
		m_numTracks = 0;
		m_curTrack = 0;
		m_curPos = 0;
		return false;
	}
	m_cdInserted = true;
	return true;
}

/*
==========================================
Close or open the tray
==========================================
*/
void CMusCDAudio::OpenTray(bool open)
{
	DWORD dwRet=0;
	DWORD dwFlags = 0;

	MCI_SET_PARMS mciSetParms;
	mciSetParms.dwAudio = 0;
	mciSetParms.dwCallback = 0;
	mciSetParms.dwTimeFormat = 0;

	if(open == false)
		dwFlags = MCI_SET_DOOR_CLOSED;
	else 
		dwFlags = MCI_SET_DOOR_OPEN;

	if(dwRet = mciSendCommand(m_mciDevice, MCI_SET, dwFlags , (DWORD)&mciSetParms))
		PrintErrorMessage(dwRet,"CMusCDAudio::OpenTray:");
}

/*
==========================================
Get Number of tracks in cd
==========================================
*/
int CMusCDAudio::GetNumTracks()
{
	DWORD dwRet=0;
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwItem = MCI_STATUS_NUMBER_OF_TRACKS;

    if (dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)(LPVOID) &mciStatusParms)) 
    {
		PrintErrorMessage(dwRet,"CMusCDAudio::PrintStats:");
		return 0;
    }
	return mciStatusParms.dwReturn;
}

/*
==========================================
Set timing method
==========================================
*/
bool CMusCDAudio::SetTimeFormat(DWORD format)
{
	//Set the timing method
	MCI_SET_PARMS mciSetParms; 
	mciSetParms.dwTimeFormat = format; 
	DWORD dwRet = mciSendCommand(m_mciDevice, MCI_SET, MCI_SET_TIME_FORMAT, (DWORD) &mciSetParms);
	if(dwRet)
	{
		PrintErrorMessage(dwRet,"CMusCDAudio::SetTimeFormat:");
		return false;
	}
	return true;
}

/*
==========================================
Print CD info
==========================================
*/
void  CMusCDAudio::PrintStats()
{
	if(!m_mciDevice)
	{
		ComPrintf("CMusCDAudio: Device is inactive\n");
		return;
	}

	//Check if there is a CD inserted
	if(!IsCDInserted())
	{
		ComPrintf("CMusCDAudio: No CD in drive\n");
		return;
	}

	m_numTracks = GetNumTracks();
	ComPrintf("%2d track(s) on CD\n", m_numTracks);

	DWORD dwRet=0;
	int minutes,seconds;
	MCI_STATUS_PARMS mciStatusParms;

	if(!SetTimeFormat(MCI_FORMAT_MSF))
		return;
	
	for(int i=1; i<=m_numTracks; i++) 
    {
		//Check if its an audio track
		mciStatusParms.dwItem = MCI_CDA_STATUS_TYPE_TRACK;
        mciStatusParms.dwTrack = i;
		mciStatusParms.dwReturn = 0;

		if(dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK, (DWORD)&mciStatusParms)) 
		{
            PrintErrorMessage(dwRet,"CMusCDAudio::PrintStats:TrackType");
            return;
        }
		
		if(mciStatusParms.dwReturn != MCI_CDA_TRACK_AUDIO)
		{
			ComPrintf("Track %2d: Data\n",i);
			continue;
		}

		//Now get the track len
        mciStatusParms.dwItem = MCI_STATUS_LENGTH;
        mciStatusParms.dwTrack = i;
		mciStatusParms.dwReturn = 0;
        
		if (dwRet = mciSendCommand(m_mciDevice, MCI_STATUS, MCI_STATUS_ITEM|MCI_TRACK, (DWORD)&mciStatusParms)) 
		{
            PrintErrorMessage(dwRet,"CMusCDAudio::PrintStats:TrackLen");
            continue;
        }
		minutes = MCI_MSF_MINUTE(mciStatusParms.dwReturn);
		seconds = MCI_MSF_SECOND(mciStatusParms.dwReturn);

		ComPrintf("Track %2d:%2d:%02d mins/secs\n", i,minutes,seconds);
    }

	if(!SetTimeFormat(MCI_FORMAT_TMSF))
		Shutdown();
}

/*
==========================================
Print Debug messages
==========================================
*/
void  CMusCDAudio::PrintErrorMessage(DWORD err, const char * msg)
{
	char szErrorBuf[MAXERRORLENGTH];
    
	if(::mciGetErrorString(err, szErrorBuf, MAXERRORLENGTH)) 
		ComPrintf("%s:%s\n",msg,szErrorBuf);
    else
		ComPrintf("%s:Unknown MCI Error\n",msg);
}


/*
==========================================

==========================================
*/
void CMusCDAudio::HandleMCIMsg(uint &wParam, long &lParam)
{
}

/*
==========================================
Command Handler
==========================================
*/
void CMusCDAudio::HandleCommand(HCMD cmdId, int numArgs, char ** szArgs)
{
	switch(cmdId)
	{
	case CD_PLAY:
		Play(szArgs[1]);
		break;
	case CD_PAUSE:
		SetPause(true);
		break;
	case CD_RESUME:
		SetPause(false);
		break;
	case CD_STOP:
		Stop();
		break;
	case CD_STATS:
		PrintStats();
		break;
	case CD_EJECT:
		OpenTray(true);
		break;
	case CD_CLOSE:
		OpenTray(false);
		break;
	}
}

