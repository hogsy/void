#ifdef INCLUDE_SOUND

#include "Sys_hdr.h"
#include "Snd_main.h"
#include "Snd_buf.h"


//Minimum 8 channels
#define CHAN_WORLD		0		//ambient, world sounds etc
#define CHAN_ITEM		1		//item noises, pickups etc
#define CHAN_WEAPON		2		//weapon noises
#define CHAN_PLAYER		3		//player sounds
#define CHAN_AUTO		4		//first freely available


/*
============================================
Util Class to manage our DirectSound buffers
============================================
*/

typedef struct Buffers_t
{
	Buffers_t() 
	{	
		next = 0; dsBuffer = 0;
	}

	~Buffers_t() 
	{ 
		next = 0;  if(dsBuffer) delete dsBuffer;
	}; //if(next) delete next;
	
	CDirectSoundBuffer *dsBuffer;
	Buffers_t	*next;
	int			index;
}Buffers_t;

class CBuffermanager
{
public:
	Buffers_t	*m_Buffers;
	Buffers_t	*m_Head;
	int			curchans;

	CBuffermanager(int chans=8);
	~CBuffermanager();
		
	Buffers_t * Next(Buffers_t *buf);
	Buffers_t * AddChan(Buffers_t *buf);

	Buffers_t * Index(int i);
};


/*
======================================
Constructor
======================================
*/

CBuffermanager::CBuffermanager(int chans)
{	
	m_Buffers = new Buffers_t;
	m_Buffers->index =0;
	m_Buffers->next = 0;
	m_Buffers->dsBuffer = new CDirectSoundBuffer();
	
	curchans = 0;
	
	m_Head = m_Buffers;

	Buffers_t *temp = m_Buffers;
	for(int i=1;i<chans;i++)
	{	
		temp = AddChan(temp);
		temp->index= i;
//		ComPrintf("Creating buffer - %d\n",temp->index);
	}
}

/*
======================================
Destructor
======================================
*/
CBuffermanager::~CBuffermanager()
{
//FIXME
	while(m_Buffers->next)
	{
		Buffers_t *temp = m_Buffers->next;
		delete m_Buffers;
		m_Buffers = temp;
	}
	delete m_Buffers;
	m_Head=0;
}


/*
======================================
returns Next channel
0 if not
======================================
*/

Buffers_t * CBuffermanager::Next(Buffers_t *buf)
{	
	if(buf && buf->next)
		return buf->next;
	return 0;
}


/*
======================================
Adds a new channel to buffer list - 
returns pointer to new channel
0 if not
======================================
*/

Buffers_t * CBuffermanager::AddChan(Buffers_t *buf)
{
	if(!buf)
		return 0;

	if(!(buf->next))
	{	
		buf->next = new Buffers_t;
		buf->next->next = 0;
		buf->next->dsBuffer = new CDirectSoundBuffer();
		curchans++;
		return buf->next;
	}
	else
	{
		while(buf->next)
			buf= Next(buf);
		buf->next = new Buffers_t;
		buf->next->next = 0;
		buf->next->dsBuffer = new CDirectSoundBuffer();
		curchans++;
		return buf->next;
	}
	return 0;
}

/*
======================================
Gets item at that position in our list
======================================
*/
Buffers_t * CBuffermanager::Index(int i)
{
	if(i > curchans)
		return 0;

	Buffers_t *temp = m_Head;
	for(int j=0;j<=i;j++)
	{	
		if(j==i)
		{	return temp;
		}
		temp = Next(temp);
	}
	
	if(j==i)	return temp;
	else		return 0;
}



/*
============================================================================
============================================================================
*/



CVar 			*	CSound::m_pvolume;		// Master DirectSound Volume 
CVar			*	CSound::m_pChannels;	// Max channels


//CVar 			CSound::m_pvolume("s_vol","0",CVar::CVAR_INT,0,&SVolume);		// Master DirectSound Volume 
//CVar			CSound::m_pChannels("s_chans","8",CVar::CVAR_INT,CVar::CVAR_ARCHIVE|CVar::CVAR_LATCH);	// Max channels

char				CSound::soundpath[MAX_PATH];

IDirectSound	*	CSound::m_pdSound = 0;	// The global sound object.

CWavemanager	*	CSound::m_pWavelist = 0;
CBuffermanager	*	g_Bufferpool = 0;

/*
=====================================
Constructor
=====================================
*/

CSound::CSound()
{
	//Buffer Pool
	g_Bufferpool = 0;

	//sound list
	m_pWavelist = 0;

/*	Sys_GetConsole()->RegisterCFunc("splay",&SPlay);
	Sys_GetConsole()->RegisterCFunc("sstop",&SStop);
	Sys_GetConsole()->RegisterCFunc("spause",&SPause);
	Sys_GetConsole()->RegisterCFunc("sresume",&SResume);
*/
	m_pvolume  = Sys_GetConsole()->RegisterCVar("s_vol","0",CVar::CVAR_INT,0,&SVolume);
	m_pChannels= Sys_GetConsole()->RegisterCVar("s_chans","8",CVar::CVAR_INT,CVar::CVAR_ARCHIVE|CVar::CVAR_LATCH);
}


/*
=====================================
Destructor
=====================================
*/

CSound::~CSound()
{
	if(g_Bufferpool)
		delete g_Bufferpool;
	if(m_pdSound)
		delete m_pdSound;
	if(m_pWavelist)
		delete m_pWavelist;
}


/*
=======================================
System Init
=======================================
*/
bool CSound::Init()//int maxchannels) 
{
	// Nothing to do if already created.
	if (m_pdSound) 
	{ return true; 
	}

	//COM library
	HRESULT hr = CoInitialize(NULL);
    if ( FAILED(hr) )
    {
        g_pCons->MsgBox("CSound::Init: Could not initialize COM\n");
		CoUninitialize();
        return false;
	}

	// Create the DirectSound object.
	hr = CoCreateInstance(CLSID_DirectSound, 0, CLSCTX_ALL,
								  IID_IDirectSound, (void**)&m_pdSound);

	if (FAILED(hr))
	{ 
		ComPrintf("CSound::Init Failed to get DirectSound Interface\n");
		CoUninitialize();
		return false; 
	}

	// Initialize the DirectSound object.
	// Defaulting to Primary Sound Driver right now
	// FIX-ME
	hr = m_pdSound->Initialize(0);
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed Init Directsound\n");
		Shutdown(); 
		return false; 
	}

	// Set the cooperative level.
	hr = m_pdSound->SetCooperativeLevel(Sys_GetHwnd(),DSSCL_NORMAL);//DSSCL_PRIORITY
	if (FAILED(hr)) 
	{ 
		ComPrintf("CSound::Init Failed SetCoopLevel\n");
		Shutdown(); 
		return false; 
	}

	sprintf(soundpath,"%s\\%s\\",CFileSystem::GetCurrentPath(),SND_PATH);

//	strcpy(soundpath,g_gamedir);
//	strcat(soundpath,SND_PATH);

		//Init Buffer Pool
	g_Bufferpool = new CBuffermanager((int)m_pChannels->value);
//	g_Bufferpool = new CBuffermanager((int)m_pChannels.value);

	//Init resource manager
	m_pWavelist = new CWavemanager;

		
//	ComPrintf("CSound::Init OK\n");
/*	g_pCons->RegisterCFunc("splay",&SPlay);
	g_pCons->RegisterCFunc("sstop",&SStop);
	g_pCons->RegisterCFunc("spause",&SPause);
	g_pCons->RegisterCFunc("sresume",&SResume);
	g_pCons->RegisterCVar(&m_pvolume,"svol","0",CVAR_INT,0,&SVolume);
*/	
//	m_pWavelist->Register("loop.wav",RES_CACHE_GAME);
	m_pWavelist->Register("talk.wav",RES_CACHE_GAME);

/*	
	m_pWavelist->Register("thunder.wav",RES_CACHE_GAME);
	m_pWavelist->Register("wind.wav",RES_CACHE_GAME);
	m_pWavelist->Register("hum.wav",RES_CACHE_GAME);
*/
	ComPrintf("CSound::Init OK\n");
	return true;
}

/*
=======================================
Shutdown
=======================================
*/
bool CSound::Shutdown(void)
{
	if (!m_pdSound) 
	{ return true; 
	}

	// Release the DirectSound object.
	m_pdSound->Release();
//	if(
//	bool result = ( == 0);
	ComPrintf("CSound::Shutdown - Released DirectSound\n");

	// Set pointer to null.
//	m_pdSound = 0;

	// Deinit the COM library.
	CoUninitialize();

	if(m_pdSound==0)
	{
		ComPrintf("CSound::Shutdown OK\n");
		return true;
	}
	
	m_pdSound = 0;
	return false;
}


/*
=======================================
makes and returns a DirectSoundBuffer
arguments, 
=======================================
*/

HRESULT CSound::MakeBuffer(IDirectSoundBuffer** buffer,
									   unsigned int bufferSize,
									  unsigned int sampleRate, 
									  unsigned int bitDepth, 
									  unsigned int channels)
{
	WAVEFORMATEX waveFmt;
	DSBUFFERDESC dsBufDesc;
	HRESULT hr;
	
	memset(&waveFmt, 0, sizeof(WAVEFORMATEX));
	waveFmt.wFormatTag = WAVE_FORMAT_PCM;
	waveFmt.nSamplesPerSec = sampleRate;
	waveFmt.wBitsPerSample = bitDepth;
	waveFmt.nChannels = channels;
	waveFmt.nBlockAlign = (channels*bitDepth)>>3;
	waveFmt.nAvgBytesPerSec = sampleRate*waveFmt.nBlockAlign;

	memset(&dsBufDesc, 0, sizeof(DSBUFFERDESC));
	dsBufDesc.dwSize = sizeof(DSBUFFERDESC);
//	dsBufDesc.dwFlags = DSBCAPS_CTRLDEFAULT | DSBCAPS_STATIC;
	dsBufDesc.dwFlags =  DSBCAPS_STATIC;
	dsBufDesc.dwBufferBytes = bufferSize;
	dsBufDesc.lpwfxFormat = &waveFmt;

	hr = m_pdSound->CreateSoundBuffer(&dsBufDesc, buffer, NULL);
	return hr;
	
}


/*
=======================================
Copies a Buffer to another buffer
- takes pointer to cur buffer, and returns a new one
=======================================
*/

IDirectSoundBuffer* CSound::CopyBuffers(IDirectSoundBuffer* buffer)
{
	// Make sure mSound was created.
	if (!m_pdSound)
	{
		ComPrintf("CSound::CopyBuffers - DirectSound not active\n");
		return 0;
	}

	// Make sure we have a buffer to duplicate.
	if (!buffer) 
	{
		ComPrintf("CSound::CopyBuffers - no buffer to copy\n");
		return 0;
	}

	IDirectSoundBuffer* duplicate = 0;
	HRESULT hr = m_pdSound->DuplicateSoundBuffer(buffer, &duplicate);

	if (FAILED(hr)) 
	{
		ComPrintf("CSound::CopyBuffers - could not duplicate buffers\n");
		return 0;
	}

	return duplicate;
}


/*
=======================================
Game Frame - 
locate entity sounds in range for OUR Client
and update volume and panning
=======================================
*/

void CSound::RunSounds()
{
}



/*
=======================================
Play Wave
=======================================
*/



/*
=======================================
Play - Console Command
buffer Auto

=======================================
*/
void CSound::SPlay(int argc,  char** argv)
{
	if((argc<=1) || !argv[1])
	{
		ComPrintf("Usage - splay <filename>\n");
		return;
	}

	int		waveindex =0;
	char *	wavename;
	char 	ext[4];				//extension

	if(!(sscanf(argv[1],"%d",&waveindex)))
	{
		ComPrintf("CSound::Play - Cant load by index %s\n",argv[1]);
		
		wavename = new char[(strlen(argv[1])+4)];			//was a filename entered ?
		strcpy(wavename,argv[1]);
		Util::GetExtension(wavename,ext);
		if(!strlen(ext))									//no extension entered, add a .wav etc
			strcat(wavename,".wav");

		ComPrintf("CSound::Play - Attempting to find %s\n",wavename);

		//lets try finding the index again
		if(!(waveindex = m_pWavelist->GetIndex(wavename)))			
		{													
			ComPrintf("CSound::Play - Attempting to register %s\n",wavename);
			if(!(waveindex = m_pWavelist->Register(wavename,RES_CACHE_LEVEL)))	//nope, can we load it
			{
				ComPrintf("CSound::Play Cant load %s\n",wavename);
				return;
			}
		}
	}
	
	//Ok we have our index now
	//select buffer and play it
	int bufindex = CHAN_AUTO;
	Buffers_t *temp = g_Bufferpool->Index(bufindex);
	
	//This one is being used - get another
	if(temp->dsBuffer->IsPlaying()) 
	for(bufindex=CHAN_AUTO; bufindex<= g_Bufferpool->curchans; bufindex++)
	{
		if(!temp->dsBuffer->IsPlaying())
			break;
		temp = g_Bufferpool->Index(bufindex);
	}

	if(temp &&  !temp->dsBuffer->IsPlaying())
	{ 
		temp->dsBuffer->Play(waveindex,1);
		//ComPrintf("CSound::Play On buffer %d, total chans %d\n",temp->index,g_Bufferpool->curchans);
		return;
	}
	ComPrintf("CSound::Play - out of AUTO buffers\n");
	
}


void CSound::Play(char *name,bool loop)
{
	if(!name)
	{
		ComPrintf("Usage - splay <filename>\n");
		return;
	}

	int		waveindex =0;
	char *	wavename;
	char 	ext[4];				//extension

		
	wavename = new char[(strlen(name)+4)];			//was a filename entered ?
	strcpy(wavename,name);
	Util::GetExtension(wavename,ext);
	if(ext && !strlen(ext))									//no extension entered, add a .wav etc
		strcat(wavename,".wav");

//	ComPrintf("CSound::Play - Attempting to find %s\n",wavename);

	//lets try finding the index again
	if(!(waveindex = m_pWavelist->GetIndex(wavename)))			
	{													
//		ComPrintf("CSound::Play - Attempting to register %s\n",wavename);
		if(!(waveindex = m_pWavelist->Register(wavename,RES_CACHE_LEVEL)))	//nope, can we load it
		{
			ComPrintf("CSound::Play Cant load %s\n",wavename);
			return;
		}
	}
	
	
	//Ok we have our index now
	//select buffer and play it
	int bufindex = CHAN_AUTO;
	Buffers_t *temp = g_Bufferpool->Index(bufindex);
	
	//This one is being used - get another
	if(temp->dsBuffer->IsPlaying()) 
	for(bufindex=CHAN_AUTO; bufindex<= g_Bufferpool->curchans; bufindex++)
	{
		if(!temp->dsBuffer->IsPlaying())
			break;
		temp = g_Bufferpool->Index(bufindex);
	}

	if(temp &&  !temp->dsBuffer->IsPlaying())
	{ 
		temp->dsBuffer->Play(waveindex,loop);
		//ComPrintf("CSound::Play On buffer %d, total chans %d\n",temp->index,g_Bufferpool->curchans);
		return;
	}
	ComPrintf("CSound::Play - out of AUTO buffers\n");
}

/*
=======================================
stop - Console Command
=======================================
*/
void CSound::SStop(int argc,  char** argv)
{
}


/*
=======================================
Pause - Console Command
=======================================
*/
void CSound::SPause(int argc,  char** argv)
{
}

/*
=======================================
Resume - Console command
=======================================
*/
void CSound::SResume(int argc,  char** argv)
{
}


/*
=======================================
Volume - Console Command
=======================================
*/
bool  CSound::SVolume(const CVar * var, int argc,  char** argv)
{
	return true;
}


/*
=======================================
Return the Direct Sound object for DirectMusic
=======================================
*/

IDirectSound * CSound::GetDirectSound()
{ return	m_pdSound;
}



/*
=======================================
Error codes
=======================================
*/

void DSError(HRESULT hr)
{
	char error[256];
	strcpy(error,"CSound::Error: ");
	
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
		strcat(error,"Unknow Error");
		break;
	}
	strcat(error,"\n");
	ComPrintf(error);
}
























#if 0


/*==========================================================================
 *
 *  Copyright (C) 1995-1997 Microsoft Corporation. All Rights Reserved.
 *
 *  File:   dsstream.c
 *  Content:   Illustrates streaming data from a disk WAVE file to a
 *             DirectSound secondary buffer for playback.
 *
 ***************************************************************************/
#define WIN32_LEAN_AND_MEAN
#define INITGUID
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsound.h>
#include <commctrl.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include <ctype.h>
#include <tchar.h>


#include "dsstream.h"
#include "debug.h"

LPDIRECTSOUND		lpDS = NULL;
LPDIRECTSOUNDBUFFER	lpDSBStreamBuffer = NULL;
LPDIRECTSOUNDNOTIFY lpDirectSoundNotify = NULL;
CRITICAL_SECTION    csDirectSound;
BOOL                fInitCSDirectSound = FALSE;

WAVEINFOCA		wiWave;



/*==========================================================================
 *
 *  Copyright (C) 1995-1996 Microsoft Corporation. All Rights Reserved.
 *
 *  File:   dstrenum.c
 *  Content:   Illustrates enumerating DirectSound drivers
 *
 ***************************************************************************/
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <dsound.h>
#include <memory.h>

#include "dsstream.h"

extern HINSTANCE hInst;
extern HWND      hWndMain;



/****************************************************************************/
/* DoDSoundEnumerate()                                                      */
/*                                                                          */
/*   This function takes care of handling the DirectSound enumeration, which*/
/* simply means creating a dialog box, at this point...                     */
/****************************************************************************/
BOOL DoDSoundEnumerate( LPGUID lpGUID )
    {
    if( DialogBoxParam( hInst, MAKEINTRESOURCE(IDD_DSENUMBOX), hWndMain,
			DSEnumDlgProc, (LPARAM)lpGUID ))
	return( TRUE );

    return( FALSE );
    }

/****************************************************************************/
/* DSEnumDlgProc()                                                          */
/*                                                                          */
/*   Dialog procedure for the DSound enumeration choice dialog. Allows the  */
/* user to choose from installed drivers.  Returns TRUE on error.           */
/****************************************************************************/
BOOL CALLBACK DSEnumDlgProc( HWND hDlg, UINT msg,
				WPARAM wParam, LPARAM lParam )
    {
    static HWND   hCombo;
    static LPGUID lpGUID;
    LPGUID        lpTemp;
    int           i;

    switch( msg )
	{
	case WM_INITDIALOG:
	    hCombo = GetDlgItem( hDlg, IDC_DSENUM_COMBO );
	    lpGUID = (LPGUID)lParam;

	    if( DirectSoundEnumerate( (LPDSENUMCALLBACK)DSEnumProc, &hCombo ) != DS_OK )
		{
		EndDialog( hDlg, TRUE );
		return( TRUE );
		}
	    if( ComboBox_GetCount( hCombo ))
		ComboBox_SetCurSel( hCombo, 0 );
	    else
		{
		EndDialog( hDlg, TRUE );
		return( TRUE );
		}
	    return( TRUE );


	case WM_COMMAND:
	    switch( LOWORD( wParam ))
		{
		case IDOK:
		    for( i = 0; i < ComboBox_GetCount( hCombo ); i++ )
			{
			lpTemp = (LPGUID)ComboBox_GetItemData( hCombo, i );
			if( i == ComboBox_GetCurSel( hCombo ))
			    {
			    if( lpTemp != NULL )
				memcpy( lpGUID, lpTemp, sizeof(GUID));
			    else
				lpGUID = NULL;
			    }
			if( lpTemp )
			    LocalFree( lpTemp );
			}
		    // If we got the NULL GUID, then we want to open the default
		    // sound driver, so return with an error and the init code
		    // will know not to pass in the guID and will send NULL
		    // instead.
		    if( lpGUID == NULL )
			EndDialog( hDlg, TRUE );
		    else
			EndDialog( hDlg, FALSE );
		    return( TRUE );

		case IDCANCEL:
		    // Force a NULL GUID
		    EndDialog( hDlg, TRUE );
		    return( TRUE );
		}
	    break;


	default:
	    return( FALSE );
	}

    return( FALSE );
    }


/****************************************************************************/
/* DSEnumProc()                                                             */
/*                                                                          */
/*   This is the Enumeration procedure called by DirectSoundEnumerate with  */
/* the parameters of each DirectSound Object available in the system.       */
/****************************************************************************/
BOOL CALLBACK DSEnumProc( LPGUID lpGUID, LPSTR lpszDesc,
				LPSTR lpszDrvName, LPVOID lpContext )
    {
    HWND   hCombo = *(HWND *)lpContext;
    LPGUID lpTemp = NULL;

    if( lpGUID != NULL )
	{
	if(( lpTemp = LocalAlloc( LPTR, sizeof(GUID))) == NULL )
	    return( TRUE );

	memcpy( lpTemp, lpGUID, sizeof(GUID));
	}

    ComboBox_AddString( hCombo, lpszDesc );
    ComboBox_SetItemData( hCombo,
			ComboBox_FindString( hCombo, 0, lpszDesc ),
			lpTemp );
    return( TRUE );
    }


#endif



#endif










