#if 0

#include "Mus_cd.h"

//======================================================================================
//======================================================================================


CMusCDAudio::CMusCDAudio()
{
}

CMusCDAudio::~CMusCDAudio()
{
}


bool  CMusCDAudio::Init()
{
}

bool  CMusCDAudio::Shutdown()
{
}

bool  CMusCDAudio::Play(char * trackname)
{
}

bool  CMusCDAudio::SetPause(bool pause)
{
}

bool  CMusCDAudio::Stop()
{
}

void  CMusCDAudio::PrintStats()
{
}

void  CMusCDAudio::SetVolume(float vol)
{
}

float CMusCDAudio::GetVolume() const
{
}

void  CMusCDAudio::PrintErrorMessage(DWORD err, const char * msg)
{
}

private:

	HWND			m_hwnd;
	MCIDEVICEID		m_mciDevice;	//mci device

	int				m_numTracks;
	int				m_curTrack;
};









#if 0


///////////////////////////////////////////////////////////
//
// Component 
//

#include "Mus_hdr.h"
#include "Mci_util.h"
#include "Mus_cd.h"


/*
=======================================
Prints out MCI errors
=======================================
*/
void  CCDMusic::PrintError(DWORD err)
{
	char szErrorBuf[MAXERRORLENGTH];
    
	if(mciGetErrorString(err, (LPSTR) szErrorBuf, MAXERRORLENGTH)) 
	{
		Print("CD Audio Error::%s\n",szErrorBuf);
	}
    else
		Print("CD Audio Error::Unknown Error\n");
}


/*
=======================================
Generic MCI Sendcommand for our cdplayer
=======================================
*/
DWORD CCDMusic::SendCommand(UINT uMsg, DWORD fdwCommand, DWORD dwParam) 
{
	DWORD dwReturn;
	
	if (dwReturn = mciSendCommand(m_mciDevice, uMsg, fdwCommand, dwParam))
	{
		m_lasterror = dwReturn;
		PrintError(dwReturn);
	}
	return dwReturn;
}

/*
=======================================
Generic MCI Status command
=======================================
*/

DWORD CCDMusic::GetStatus(DWORD dwItem) 
{	
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwCallback = (DWORD) m_hwnd;
	mciStatusParms.dwItem = dwItem;
	mciStatusParms.dwReturn = 0;
	
	SendCommand(MCI_STATUS, MCI_STATUS_ITEM, (DWORD) &mciStatusParms);
		
	return mciStatusParms.dwReturn;
}


/*
=======================================
Generic MCI Set  Command
=======================================
*/

DWORD CCDMusic::Set(DWORD dwWhat)
{
	MCI_SET_PARMS mciSetParms;

	mciSetParms.dwCallback = (DWORD) m_hwnd;
	
	return SendCommand(MCI_SET, dwWhat, (DWORD) &mciSetParms);
}




/*
=======================================
Initialize our cdplayer
=======================================
*/
bool __stdcall CCDMusic::CDInit(DPRINT print, HWND	hwnd)
{
	dprintf = print;

	DWORD dwReturn;
	MCI_OPEN_PARMS mciOpenParms;

	// Opens a device by specifying a device-type constant.
	mciOpenParms.lpstrDeviceType = (LPCSTR)MCI_DEVTYPE_CD_AUDIO;
	
	DWORD dwFlags = MCI_OPEN_TYPE|MCI_OPEN_TYPE_ID|MCI_WAIT;

/*
	if (bShareable) 
		dwFlags |= MCI_OPEN_SHAREABLE;
*/
	
	dwReturn = SendCommand(MCI_OPEN, dwFlags, 
			(DWORD)(LPVOID) &mciOpenParms);

	if (dwReturn == 0)
	{
		// The device opened successfully; get the device ID.
		m_mciDevice = mciOpenParms.wDeviceID;		
	}

	Print("CCDMusic::CDInit - OK\n");
	return true;
}


/*
=======================================
Release/Shutdown the CDPlayer
=======================================
*/

bool __stdcall CCDMusic::CDShutdown()
{

	MCI_GENERIC_PARMS mciGenericParms;
//	mciGenericParms.dwCallback = (DWORD) m_hwnd;
	SendCommand(MCI_CLOSE, 0, (DWORD) &mciGenericParms);
	return true;
}



/*
=======================================
Play a track
=======================================
*/

bool __stdcall CCDMusic::CDPlay(int track)
{
/*
	if((m_cdstate == CD_PLAYING) && !CDStop())
	{
		Print("CCDMusic::CDPlay: Couldnt stop playing
		return false;
	}
*/

	DWORD dwReturn;
	// Save current time format
	DWORD dwOldTimeFormat = GetTimeFormat();

	BOOL bAsync= false;
    
	// Set the time format to track/minute/second/frame (TMSF).
    if (dwReturn = SetTimeFormat(MCI_FORMAT_TMSF))
		return false;
	
	// Get track lenght
	CMsf msf = GetTrackLength(track);
	
	CTmsf msfFrom = CTmsf(track, 0, 0, 0);
	CTmsf msfTo = CTmsf(track, 
		msf.GetMinute(), msf.GetSecond(), msf.GetFrame());
	    
	// Play the track
	if (dwReturn = Play(msfFrom, msfTo, bAsync))
		return false;
    	
	// Restore the saved time format
	if(dwReturn  = SetTimeFormat(dwOldTimeFormat))
		return false;

	return true;
}


/*
=======================================
Stop playing
=======================================
*/

bool __stdcall CCDMusic::CDStop()
{
	if(m_cdstate == CD_PLAYING)
	{
		MCI_GENERIC_PARMS mciGenericParms;
		
		mciGenericParms.dwCallback = (DWORD) m_hwnd;

		if(SendCommand(MCI_STOP, 0, (DWORD) &mciGenericParms))
			return false;
		return true;
	}
	return false;
}

bool __stdcall CCDMusic::CDPause()
{
	if(m_cdstate == CD_PLAYING)
	{
		MCI_GENERIC_PARMS mciGenericParms;

		mciGenericParms.dwCallback = (DWORD) m_hwnd;
	
		if(SendCommand(MCI_PAUSE, 0, (DWORD) &mciGenericParms))
			return false;
		return true;
	}
	return false;
}

bool __stdcall CCDMusic::CDResume()
{
	return false;
}


void __stdcall CCDMusic::CDTrackListing()
{
}

bool __stdcall CCDMusic::CDEject()
{
	DWORD mode;
	mode = GetStatus(MCI_STATUS_MODE);

	if(mode == MCI_MODE_OPEN)
		return false;

	if(Set(MCI_SET_DOOR_OPEN))
		return false;
	return true;
}


bool __stdcall CCDMusic::CDClose()
{
	DWORD mode;
	mode = GetStatus(MCI_STATUS_MODE);

	if(mode != MCI_MODE_OPEN)
		return false;

	if(Set(MCI_SET_DOOR_CLOSED))
		return false;
	return true;
}
	


/*
=======================================
Plays CD from a position to another. dwFrom and dwTo are interpreted
accordingly with the current time format.
=======================================
*/


DWORD CCDMusic::Play(DWORD dwFrom /*=0L*/, DWORD dwTo /*=0L*/, 
					 BOOL bAsync /*=TRUE*/)
{
/*
	if((m_cdstate == CD_PLAYING) && !CDStop())
	{
		Print("CCDMusic::CDPlay: Couldnt stop playing
		return false;
	}
*/

	MCI_PLAY_PARMS mciPlayParms;
    mciPlayParms.dwFrom = dwFrom;
	mciPlayParms.dwTo = dwTo;
	DWORD dwFlags = MCI_FROM | MCI_TO;

	if (bAsync) 
	{		
		mciPlayParms.dwCallback = (DWORD) m_hwnd;
		dwFlags |= MCI_NOTIFY;
	}

	return SendCommand(MCI_PLAY, dwFlags, (DWORD)(LPVOID) &mciPlayParms);
}



/*
=======================================
Set the device time format. Allowable time formats are:
MCI_FORMAT_TMSF, MCI_FORMAT_MSF, MCI_FORMAT_MILLISECONDS
=======================================
*/

DWORD CCDMusic::SetTimeFormat(DWORD dwTimeFormat) 
{   
	MCI_SET_PARMS mciSetParms;
	
	mciSetParms.dwTimeFormat = dwTimeFormat;

	return SendCommand(MCI_SET, 
        	MCI_SET_TIME_FORMAT, (DWORD)(LPVOID) &mciSetParms);    
}

/*
=======================================
Get the current time format
=======================================
*/
DWORD CCDMusic::GetTimeFormat() 
{
	return GetStatus(MCI_STATUS_TIME_FORMAT);
}



/*
=======================================
Seek to locations
=======================================
*/


// Seek to a specified position
DWORD CCDMusic::Seek(DWORD dwTo, BOOL bAsync /*=FALSE*/) 
{
	return Seek(dwTo, MCI_TO, bAsync);
}

// Seek to end
DWORD CCDMusic::SeekToStart(BOOL bAsync /*=FALSE*/) 
{
	return Seek(0, MCI_SEEK_TO_START, bAsync);
}

// Seek to start
DWORD CCDMusic::SeekToEnd(BOOL bAsync /*=FALSE*/)
 {
	return Seek(0, MCI_SEEK_TO_END, bAsync);
}

// Generic Seek
DWORD CCDMusic::Seek(DWORD dwTo, DWORD dwFlags, BOOL bAsync)
{	
	MCI_SEEK_PARMS mciSeekParms;

	if (bAsync) 
	{
		mciSeekParms.dwCallback = (DWORD) m_hwnd;
		dwFlags |= MCI_NOTIFY;
	}

	mciSeekParms.dwTo = dwTo;
    	
	return SendCommand(MCI_SEEK, dwFlags, (DWORD) &mciSeekParms);	
}





/*
=======================================
Track info
=======================================
*/

// Retrieves the starting position of a track
DWORD CCDMusic::GetTrackPos(DWORD dwTrack) 
{	
	return GetTrackInfo(dwTrack, MCI_STATUS_POSITION);	
}

// Retrieves the type of a track
DWORD CCDMusic::GetTrackType(DWORD dwTrack) 
{	
	return GetTrackInfo(dwTrack, MCI_CDA_STATUS_TYPE_TRACK);
}

// Retrieves the length of a track
DWORD CCDMusic::GetTrackLength(DWORD dwTrack) 
{	
	return GetTrackInfo(dwTrack, MCI_STATUS_LENGTH);
}

// Retrieves a track information
DWORD CCDMusic::GetTrackInfo(DWORD dwTrack, DWORD dwItem) 
{	
	
	MCI_STATUS_PARMS mciStatusParms;
	mciStatusParms.dwCallback = (DWORD) m_hwnd;
	mciStatusParms.dwItem = dwItem;
	mciStatusParms.dwTrack = dwTrack;
	mciStatusParms.dwReturn = 0;
	
	SendCommand(MCI_STATUS, 
		MCI_STATUS_ITEM|MCI_TRACK, (DWORD) &mciStatusParms);
		
	return mciStatusParms.dwReturn;
}








// Retrieves the total CD length
DWORD CCDMusic::GetMediaLength(DWORD dwTrack)
{	
	return GetStatus(MCI_STATUS_LENGTH);
}

// Retrieves the current track
DWORD CCDMusic::GetCurrentTrack() 
{
	return GetStatus(MCI_STATUS_CURRENT_TRACK);
}

// Retrieves the CD current position
DWORD CCDMusic::GetCurrentPos() 
{
	return GetStatus(MCI_STATUS_POSITION);
}

// Retrieves the CD starting position
DWORD CCDMusic::GetStartPos()
{
	return GetStatus(MCI_STATUS_START);
}

// Gets the number of playable tracks
DWORD CCDMusic::GetNumberOfTracks() 
{
	return GetStatus(MCI_STATUS_NUMBER_OF_TRACKS);
}

// Checks if the device is Ready to be operated
BOOL CCDMusic::IsReady() 
{
	return GetStatus(MCI_STATUS_READY);
}


/*
// Generic MCI_GETDEVCAPS_ITEM command
DWORD CCDMusic::GetDevCapsItem(DWORD dwItem) 
{	
	MCI_GETDEVCAPS_PARMS mciCapsParms;

	mciCapsParms.dwCallback = (DWORD) m_hMainWnd;
	mciCapsParms.dwItem = dwItem;
	mciCapsParms.dwReturn = 0;
	
	SendCommand(MCI_GETDEVCAPS, MCI_GETDEVCAPS_ITEM,
		(DWORD) &mciCapsParms);
	
	return mciCapsParms.dwReturn;
}
*/




#endif


#endif
