#if 0

#include "Mus_hdr.h"
#include "Mus_dm.h"

IDirectMusicLoader*      CDirectMusic::m_pLoader;
IDirectMusicPerformance* CDirectMusic::m_pPerformance;
IDirectMusicSegment*     CDirectMusic::m_pSegment;
IDirectMusicPort*		 CDirectMusic::m_pPort;
IDirectMusic*			 CDirectMusic::m_pDirect;
IDirectMusicComposer* 	 CDirectMusic::m_pComposer;
IDirectMusicStyle*       CDirectMusic::m_pStyle;

/*
IDirectMusic	*			m_pDirect;
	IDirectMusicLoader*      m_pLoader;
	IDirectMusicPerformance* m_pPerformance;
	IDirectMusicSegment*     m_pSegment;
	IDirectMusicPort*		m_pPort;
	IDirectMusicComposer* 	m_pComposer;
	IDirectMusicStyle*       m_pStyle;
*/

#define MULTI_TO_WIDE( x,y )	MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, y, -1, x, _MAX_PATH );


/*
=======================================
Constructor
=======================================
*/
CDirectMusic::CDirectMusic()
{
//	m_refcount=0;
	InterlockedIncrement(&g_cComponents) ;
	dprintf = 0;
}

/*
=======================================
Destructor
=======================================
*/
CDirectMusic::~CDirectMusic()
{
	InterlockedDecrement(&g_cComponents) ; 
	dprintf = 0;
}




//
// IUnknown implementation
//
HRESULT __stdcall CDirectMusic::QueryInterface(const IID& iid, void** ppv)
{    
	if (iid == IID_IUnknown)
	{
		*ppv = static_cast<I_MusDMusic*>(this) ; 
//		trace("Component:\t\tReturn pointer to IUnknown.") ; 
	}
	else if (iid == IID_IMUSDMUSIC)
	{
		*ppv = static_cast<I_MusDMusic*>(this) ;
//		trace("Component:\t\tReturn pointer to IMusCD.") ; 
	}
	else
	{
		*ppv = NULL ;
		return E_NOINTERFACE ;
	}
	reinterpret_cast<IUnknown*>(*ppv)->AddRef() ;
	return S_OK ;
}

ULONG __stdcall CDirectMusic::AddRef()
{
	return InterlockedIncrement(&m_refcount) ;
}

ULONG __stdcall CDirectMusic::Release() 
{
	if (InterlockedDecrement(&m_refcount) == 0)
	{
		delete this ;
		return 0 ;
	}
	return m_refcount ;
}




/*
=======================================
Init System
=======================================
*/
bool __stdcall CDirectMusic::DMInit(DPRINT print,char *gamepath, IDirectSound *p_ds)
{
	if(!print)
		return false;

	dprintf = print;
	dprintf("INIT DMINIT\n");

	HRESULT hr;
	WCHAR           wszDir[_MAX_PATH];

	//COM library
/*	hr = CoInitialize(NULL);

    if ( FAILED(hr) )
    {
        g_pCons->MsgBox("CDirectMusic::Init: Could not initialize COM");
		CoUninitialize();
        return false;
	}
*/

		// Create loader object
    hr = CoCreateInstance( CLSID_DirectMusicLoader, NULL, CLSCTX_INPROC, 
                           IID_IDirectMusicLoader, (void**)&m_pLoader );
    if ( FAILED(hr) )
    {
		dprintf("CDirectMusic::Init: Couldnt create loader object");
		m_pPerformance = 0;
		m_pLoader = 0;
		m_pComposer = 0;
//		CoUninitialize();
        return false;
	}


	dprintf("INIT DMINIT - loader ok\n");
	
	// Change the loader's current search directory 
	MULTI_TO_WIDE( wszDir, gamepath);
    m_pLoader->SetSearchDirectory( GUID_DirectMusicAllTypes, wszDir, FALSE );
	m_pLoader->EnableCache(GUID_DirectMusicAllTypes, true);

	
	hr = CoCreateInstance(CLSID_DirectMusicComposer, NULL, CLSCTX_INPROC,
						  IID_IDirectMusicComposer, (void**)(&m_pComposer));
	if (FAILED(hr)) 
	{
		dprintf("CDirectMusic::Init: Couldnt create composer object");
		m_pLoader->Release();
		//m_pComposer->Release();
		
		m_pPerformance = 0;
		m_pLoader = 0;
		m_pComposer = 0;
//		CoUninitialize();
		return false;
	}

	dprintf("INIT DMINIT - composer ok\n");

	// Create performance object
    hr = CoCreateInstance( CLSID_DirectMusicPerformance, 
							NULL, 
							CLSCTX_INPROC, 
                           IID_IDirectMusicPerformance, (void**)&m_pPerformance );
    if ( FAILED(hr) )
    {
		dprintf("CDirectMusic::Init: Couldnt create performance object");
		//m_pPerformance->Release();
		m_pLoader->Release();
		m_pComposer->Release();

		m_pPerformance = 0;
		m_pLoader = 0;
		m_pComposer = 0;
//		CoUninitialize();
        return false;
	}

	dprintf("INIT DMINIT - performance ok\n");

	hr = InitSynth(p_ds);

    if ( FAILED(hr) )
    {  
		m_pPerformance->Release();
		m_pLoader->Release();
		m_pComposer->Release();
//		CoUninitialize();
		m_pPerformance = 0;
		m_pLoader = 0;
		m_pComposer = 0;
		dprintf("CDirectMusic:: FAILED INIT SYNTH\n");
		return false;
	}

	dprintf("CDirectMusic::Init OK\n");
	return true;
}

/*
=======================================
Shutdown
=======================================
*/
bool __stdcall CDirectMusic::Shutdown()
{
	// If there is any music playing, stop it.
	if(m_pPerformance)
	{
		m_pPerformance->Stop( NULL, NULL, 0, 0 );

		// CloseDown and Release the performance object
		m_pPerformance->CloseDown();
		m_pPerformance->Release();
		m_pPerformance = 0;
	}

    if(m_pLoader)
	{
		// Release the loader object
		m_pLoader->ClearCache(GUID_DirectMusicAllTypes);
		m_pLoader->Release();
		m_pLoader = 0;
	}

	//Composer
	if(m_pComposer)
	{
		m_pComposer->Release();
		m_pComposer = 0;
	}

	//COM
//	CoUninitialize(); 

	dprintf("CDirectMusic::Shutdown - OK\n");
	return true;
}

/*
=======================================
Play
=======================================
*/
bool __stdcall CDirectMusic::Play(const char *file)
{
	HRESULT         hr;
	GUID			guid;
	DMUS_OBJECTDESC ObjDesc; // Object descriptor for pLoader->GetObject()
	
    // now load the segment file.
    // sections load as type Segment, as do MIDI files, for example.
    ObjDesc.guidClass = CLSID_DirectMusicSegment;
    ObjDesc.dwSize = sizeof(DMUS_OBJECTDESC);
    
	MULTI_TO_WIDE(ObjDesc.wszFileName, file);
    
	ObjDesc.dwValidData = DMUS_OBJ_CLASS | DMUS_OBJ_FILENAME;
    hr = m_pLoader->GetObject( &ObjDesc, IID_IDirectMusicSegment, (void**)&m_pSegment );

    if(FAILED(hr))
	{
		dprintf("CDirectMusic::Play:Couldnt load %s\n",file);
		return false;
	}

	m_pSegment->SetRepeats(200);

	// Get the Style from the Segment by calling the Segment's GetData()
	// with the data type GUID_StyleTrackStyle.
	// 0xffffffff indicates to look at Tracks in all TrackGroups in the segment.
	// The first 0 indicates to retrieve the Style from the first Track 
	// in the indicated TrackGroup.
	// The second 0 indicates to retrieve the Style from the beginning
	// of the Segment, i.e. time 0 in Segment time.
	// If this Segment was loaded from a Section file, there is only one 
	// Style and it is at time 0.
	// Note that the GetData() call with GUID_IDirectMusicStyle assumes the
	// third parameter is the address of a pointer to an IDirectMusicStyle.
		
	guid = GUID_IDirectMusicStyle;
	hr = m_pSegment->GetParam( guid, 0xffffffff, 0, 0, NULL, (void*)&m_pStyle );
	if ( FAILED(hr) )
	{
		dprintf("CDirectMusic::Play: Could not get DMusic style\n");
//		return false;
	}

    // Play the segment and wait. The DMUS_SEGF_BEAT indicates to play on the next beat 
    // if there is a segment currently playing. The first 0 indicates to
    // play (on the next beat from) now.
    // The final NULL means do not return an IDirectMusicSegmentState* in
    // the last parameter.
    m_pPerformance->PlaySegment( m_pSegment, DMUS_SEGF_BEAT, 0, NULL );

    // Get the names of the Motifs from the Style. Styles may have
    // any number of Motifs, but for simplicity's sake only get
    // a maximum of 9 here.
/*    
	for( dwIndex = 0; dwIndex < 9; dwIndex++ )
    {
        if( S_OK != g_pStyle->EnumMotif( dwIndex, g_awszMotifName[dwIndex] ))
        {
            g_awszMotifName[dwIndex][0] = 0;
            break;
        }
    }
*/

	dprintf("CDirectMusic::Playing\n");
	return true;
}






/*
=======================================
Stop
=======================================
*/
bool __stdcall CDirectMusic::Stop()
{
	// Release the Segment
    m_pSegment->Release();

   // If there is any music playing, stop it.
    m_pPerformance->Stop( NULL, NULL, 0, 0 );
	return true;
}


/*
=======================================
Resume
=======================================
*/
bool __stdcall CDirectMusic::Resume()
{
	return true;
}


/*
=======================================
Pause
=======================================
*/
bool __stdcall CDirectMusic::Pause()
{
	return true;
}







/*
=======================================
Adds a Capable Port
=======================================
*/
HRESULT CDirectMusic::InitPort(DWORD minChannelGroups)
{
	// Create the software synth port.
    // An alternate, easier method is to call
    // pPerf->AddPort(NULL), which automatically
    // creates the synth port, adds it to the
    // performance, and assigns PChannels.
	HRESULT			  hr;
	DMUS_PORTPARAMS   dmos;
    GUID              guidSink;

    ZeroMemory( &dmos, sizeof(DMUS_PORTPARAMS) );
    dmos.dwSize = sizeof(DMUS_PORTPARAMS);  
    dmos.dwChannelGroups = minChannelGroups;	// create 1 channel groups on the port
    dmos.dwValidParams = DMUS_PORTPARAMS_CHANNELGROUPS; 

    ZeroMemory( &guidSink, sizeof(GUID) );

    hr = m_pDirect->CreatePort( CLSID_DirectMusicSynth, &dmos, &m_pPort, NULL );
    if ( FAILED(hr) )
	{
	    dprintf("CDirectMusic::InitPort:Could not create port");
        return hr;
	}

    hr = m_pDirect->Activate( TRUE );
    if ( FAILED(hr) )
	{
	    dprintf("CDirectMusic::InitPort: Could not activate DMusic");
        return hr;
	}

    // Succeeded in creating the port. Add the port to the
    // Performance with five groups of 16 midi channels.
    hr = m_pPerformance->AddPort( m_pPort );
    if ( FAILED(hr) )
	{
	    dprintf("CDirectMusic::InitPort:Could not add port"); 
		return hr;
	}

    // Assign a block of 16 PChannels to this port.
    // Block 0, port pPort, and group 1 means to assign
    // PChannels 0-15 to group 1 on port pPort.
    // PChannels 0-15 correspond to the standard 16
    // MIDI channels.
    m_pPerformance->AssignPChannelBlock( 0, m_pPort, 1 );

    // Release the port since the performance now has its own reference.
    m_pPort->Release();

    // release the DirectMusic object. The performance has its
    // own reference and we just needed it to call CreatePort.
    m_pDirect->Release();

    return S_OK;
}



/*
=======================================
Initialize the software synthesizer into the performance.
=======================================
*/
HRESULT CDirectMusic::InitSynth(IDirectSound *p_ds)
{
    HRESULT hr;

    // Calling AddPort with NULL automatically initializes the Performance
    // by calling IDirectMusicPerformance.Init(), adds a default
    // port with one channel group, and assigns PChannels 0-15 
    // to the synth port's channel group MChannels 0-15.
    // Please refer to the PlayMotf example for a more flexible, yet slightly
    // more complicated, way of doing this.
    
	hr = m_pPerformance->Init(&m_pDirect,p_ds,NULL);
    if ( FAILED(hr) )
    {
        dprintf("CDirectMusic::InitSynth: Could not initialize performance");
        return hr;
	}
    
	// Set autodownloading to be on.  This will cause DLS instruments to be downloaded
	// whenever a segment is played, and unloaded whenever it stops.  Please see the
	// DirectMusic documentation for more information.
	
	BOOL fAutoDownload = TRUE;
	m_pPerformance->SetGlobalParam(GUID_PerfAutoDownload,&fAutoDownload,sizeof(BOOL));

	//	m_pPerformance->AddPort(0);
	hr= InitPort(1);

    if( FAILED(hr) )
	{
        dprintf("CDirectMusic::InitSynth: Could not add port to performance");
		return hr;
	}

    return S_OK;
}




/*
=======================================
Play Motif
=======================================
*/

HRESULT CDirectMusic::PlayMotif( WCHAR* pwszMotifName )
{
    IDirectMusicSegment* pSeg;
    HRESULT              hr;

    // Get the Motif Segment from the Style, setting it to play once
    // through (no repeats.) Check for S_OK specifically, because
    // GetMotif() returns S_FALSE if it doesn't find the Motif.
    hr = m_pStyle->GetMotif( pwszMotifName, &pSeg );

    if( S_OK == hr )
    {

        // Play the segment. The PSF_BEAT indicates to play on the next beat 
        // if there is a segment currently playing. PSF_SECONDARY means to
        // play the segment as a secondary segment, which plays on top of
        // the currently playing primary segment. The first 0 indicates to
        // play (on the next beat from) now.
        // The final NULL means do not return an IDirectMusicSegmentState* in
        // the last parameter.
        m_pPerformance->PlaySegment( pSeg, 
                                     DMUS_SEGF_BEAT | DMUS_SEGF_SECONDARY, 
                                     0, 
                                     NULL );
        pSeg->Release();
    }

    return hr;
}

#endif



