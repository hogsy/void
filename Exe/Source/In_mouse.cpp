#include "In_mouse.h"

//========================================================================================
//Private Definations

#define M_DIBUFFER_SIZE		16
#define M_MOUSEBUTTONS		4
#define M_W32MOUSEBUTTONS	3

//========================================================================================
//Private Local Vars

//extern CMouse * g_pMouse;	//pointer to Mouse class

//Current mouse co-ords
static float	m_fXPos, 
				m_fYPos, 
				m_fZPos;		
//Last mouse co-ords		
static float	m_fLastXPos, 
				m_fLastYPos, 
				m_fLastZPos;	
//Center of the screen, Win32 mouse routines need these
static int		m_dCenterX, 
				m_dCenterY;
//Sensitivities
static float	m_fXSens,
				m_fYSens, 
				m_fSens;
//Console Vars

//Other flags
static bool		m_bExclusive;
static bool		m_bInvert;
static bool		m_bFilter;

//Mouse State and Mode
static EDeviceState			m_eMouseState;
static CMouse::EMouseMode	m_eMouseMode;

//DirectInput Device
static LPDIRECTINPUTDEVICE7 m_pDIMouse=0;	

//Input buffers
static DIMOUSESTATE2	  * m_pDIState=0;	
static DIDEVICEOBJECTDATA	m_aDIMouseBuf[M_DIBUFFER_SIZE];
static POINT				m_w32Pos;
static short				m_w32Buttons[M_W32MOUSEBUTTONS];

//Event Handle and Name
static HANDLE	m_hDIMouseEvent=0;
static char		m_szMouseEventName[] = "DI MouseEvent";

//Cursor Listener
static I_InCursorListener  * m_pCursorHandler=0;


//========================================================================================
//Local function declarations
//========================================================================================


//========================================================================================
//Implementation
//========================================================================================

//CVar Handler

bool CMouse::HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)
{
	if(cvar == &m_pVarXSens)
		return CXSens((CVar*)cvar,numArgs,szArgs);
	else if(cvar == &m_pVarYSens)
		return CYSens((CVar*)cvar,numArgs,szArgs);
	else if(cvar == &m_pVarSens)
		return CSens((CVar*)cvar,numArgs,szArgs);
	else if(cvar == &m_pVarInvert)
		return CInvert((CVar*)cvar,numArgs,szArgs);
	else if(cvar == &m_pVarMode)
		return CMouseMode((CVar*)cvar,numArgs,szArgs);
	else if(cvar == &m_pVarFilter)
		return CMouseFilter((CVar*)cvar,numArgs,szArgs);
	return false;
}

/*
=====================================
Constructor
=====================================
*/
CMouse::CMouse() :	m_pVarXSens("m_fXSens","0.2",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE),
					m_pVarYSens("m_fYSens","0.2",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE),
					m_pVarSens ("m_fSens","5.0",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE),
					m_pVarInvert("m_invert","0",CVar::CVAR_BOOL,CVar::CVAR_ARCHIVE),
					m_pVarMode("m_mode","1",CVar::CVAR_INT,CVar::CVAR_ARCHIVE),
					m_pVarFilter("m_filter","0",CVar::CVAR_BOOL, CVar::CVAR_ARCHIVE)
{
	m_pDIMouse = 0;
	m_eMouseState = DEVNONE;
	m_eMouseMode = M_NONE;

//	PollMouse = 0;

	m_bExclusive = false;
	m_bInvert = false;
	
	m_pDIState = 0;

	m_fXPos= m_fYPos= m_fZPos=0.0f;
	m_fXSens= m_fYSens= m_fSens= 0.0f;
	m_fLastXPos = m_fLastYPos = m_fLastZPos = 0.0f;
	
	m_dCenterX = m_dCenterY = 0;

	SetCursorListener(this);

	System::GetConsole()->RegisterCVar(&m_pVarXSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarYSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarInvert,this);
	System::GetConsole()->RegisterCVar(&m_pVarMode,this);
	System::GetConsole()->RegisterCVar(&m_pVarFilter,this);
}

/*
=====================================
Destructor
=====================================
*/
CMouse::~CMouse()
{
	Shutdown();

	m_eMouseState = DEVNONE;
//	PollMouse = 0;

	if(m_pDIState)
		delete m_pDIState;

	m_pDIMouse = 0;
	m_pCursorHandler = 0;
}

/*
=====================================
Intialize the mouse
The mouse mode SHOULD have been set
by a config file by now, or else it 
will default to DI_BUFFERED
=====================================
*/
HRESULT CMouse::Init(int exclusive, EMouseMode mode)
{
	HRESULT hr;

	if(exclusive)
		m_bExclusive = true;

	//mode specified as NONE when the device is first initialized
	if(mode == M_NONE)
	{
		//then default to what we read from the config files
		if(m_eMouseMode != M_NONE)
			mode = m_eMouseMode;
		//If read nothing from config, then default to IMMEDIATE
		else
			mode = M_DIIMMEDIATE;
	}

	//If the device is active and is in the same mode 
	//then we don't need to do anything
	if((m_eMouseMode == mode) && (m_eMouseState != DEVNONE))
	{
		ComPrintf("CMouse::InitMode: Already in given mode\n");
		return DI_OK;
	}

	//Activate specified mode
	switch(mode)
	{
	case M_WIN32:
		{
			hr = Win32_Init();
			if(FAILED(hr))
				return hr;
			break;
		}
	case M_DIIMMEDIATE:
	case M_DIBUFFERED:
		{
			hr = DI_Init(mode);
			if(FAILED(hr))
				return hr;
			break;
		}
	}

	//Update Sensitivities and other variables that are used at runtime
	m_fXSens= m_pVarXSens.value;
	m_fYSens= m_pVarYSens.value;
	m_fSens= m_pVarSens.value;
	m_pVarFilter.value ? m_bFilter = true: m_bFilter = false;
	m_pVarInvert.value ? m_bInvert = true: m_bInvert = false;

	//Update state vars
	m_eMouseState = DEVINITIALIZED;

	Acquire();
	return DI_OK;
}

/*
=====================================
Shutdown/Release the mouse
=====================================
*/
void CMouse::Shutdown()
{
	switch(m_eMouseMode)
	{
	case M_DIIMMEDIATE:
	case M_DIBUFFERED:
		{
			DI_Shutdown();
			break;
		}
	case M_WIN32:
		{
			Win32_Shutdown();
			break;
		}
	}
	m_eMouseState = DEVNONE;
	m_eMouseMode = M_NONE;
}

/*
=====================================
Initialize DI specific stuff
=====================================
*/
HRESULT CMouse::DI_Init(EMouseMode mode)
{
	//Shutdown the mouse if its active in another mode
	if(m_eMouseState != DEVNONE)
		Shutdown();
	
	//Create the Device
	HRESULT hr = (In_GetDirectInput())->CreateDeviceEx(GUID_SysMouse, 
									   IID_IDirectInputDevice7,
									   (void**)&m_pDIMouse, 
									   NULL); 
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::DI_Init: Couldnt create Device\n");
		return hr;
	}
	
	//Set Data format for mouse
	hr = m_pDIMouse->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::Create:Set Data Format failed\n");
		return hr;
	}

	//Set Co-operative mode
	if(m_bExclusive)
		hr = m_pDIMouse->SetCooperativeLevel(System::GetHwnd(), 
											DISCL_FOREGROUND|DISCL_EXCLUSIVE);
	else
		hr = m_pDIMouse->SetCooperativeLevel(System::GetHwnd(), 
											DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::DI_Init:Set Coop Level failed\n");
		return hr;
	}

	//Set Event Notification
	m_hDIMouseEvent =  ::CreateEvent(0,     // pointer to security attributes
							FALSE,  // flag for manual-reset event
						    FALSE,  // flag for initial state
							m_szMouseEventName);   // pointer to event-object name

	if(m_hDIMouseEvent == NULL)
	{
		ComPrintf("CKeyboard::DI_Init : Unable to set event\n");
		return ::GetLastError();
	}

	hr = m_pDIMouse->SetEventNotification(m_hDIMouseEvent);
	if(FAILED(hr))
	{
		ComPrintf("CMouse::DI_Init : Unable to set event\n");
		return hr;
	}
	
	//Which DI mode to use ?
	if(mode == M_DIIMMEDIATE)
	{
		if(m_pDIState)
			delete m_pDIState;
		m_pDIState= new DIMOUSESTATE2;

		//Set update info
//		PollMouse = Update_DIImmediate;
		ComPrintf("CMouse::InitMode:Initialized DI Immediate mode\n");
		m_eMouseMode = M_DIIMMEDIATE;
	}
	else if(mode == M_DIBUFFERED)
	{
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = M_DIBUFFER_SIZE;

		hr = m_pDIMouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

		if(FAILED(hr))
		{
			ComPrintf("CMouse::DI_Init:Setting buffer size failed\n");
			return hr;
		}

		if(m_pDIState)
			delete m_pDIState;
		m_pDIState = 0;

		//Set update info
//		PollMouse = Update_DIBuffered;
		ComPrintf("CMouse::InitMode:Initialized DI Buffered mode\n");
		m_eMouseMode = M_DIBUFFERED;
	}
	return S_OK;
}

/*
=====================================
Release DI stuff
=====================================
*/
bool CMouse::DI_Shutdown()
{
	if(m_pDIMouse)
	{
		if(m_eMouseState == DEVACQUIRED)
			m_pDIMouse->Unacquire();
		SetExclusive(false);
		m_pDIMouse->SetEventNotification(0);
		m_pDIMouse->Release();
		m_pDIMouse = NULL;
	}
	ComPrintf("CMouse::DI_Shutdown OK\n");
	return true;
}

/*
===========================================
Intialize the mouse to use Win32 routines
===========================================
*/
HRESULT CMouse::Win32_Init()
{
	//Shutdown older mouse mode
	if(m_eMouseState != DEVNONE)
		Shutdown();

	//Lock Cursor to center
	::SetCursorPos(m_dCenterX,m_dCenterY);

	//Cap the mouse
	::SetCapture(System::GetHwnd());

	//FIX ME
	//GetSystemMetrics(SM_SWAPBUTTON) 
	//returns TRUE if the mouse buttons have been swapped

	//Set to exclusive
	if(m_bExclusive)
		SetExclusive(true);

//	PollMouse = Update_Win32;

	ComPrintf("CMouse::InitMode:Initialized Win32 mode\n");
	m_eMouseMode = M_WIN32;
	return S_OK;
}


/*
===========================================
Shutdown Win32 mouse 
===========================================
*/
bool CMouse::Win32_Shutdown()
{
	//Lose exclusive
	if(m_bExclusive)
		SetExclusive(false);

	//Unacquire it
	if(m_eMouseState == DEVACQUIRED)
		UnAcquire();
	
	::ClipCursor(0);
	::ReleaseCapture();
	ComPrintf("CMouse::Win32_Shutdown OK\n");
	return true;
}


/*
==========================================
Utility needed by CInput()
==========================================
*/
EDeviceState CMouse::GetDeviceState() 
{ return m_eMouseState;
}


/*
==========================================
Set Listener object
==========================================
*/
void CMouse::SetCursorListener( I_InCursorListener *  plistener)
{
	if(plistener)
		m_pCursorHandler = plistener;
	else
		m_pCursorHandler = this;
}

//========================================================================================
//Mouse Update functions
//========================================================================================

/*
=====================================
Update Mouse
=====================================
*/
void CMouse::Update()
{
	if(m_eMouseState != DEVACQUIRED)
		return;
	
	//Win32 mode doesnt use any notification
	if(m_eMouseMode == M_WIN32)
		Update_Win32();
	else if (WaitForSingleObject(m_hDIMouseEvent, 0) == WAIT_OBJECT_0) 
	{ 
		// Event is set. If the event was created as 
		// autoreset, it has also been reset. 
		//PollMouse();
		if(m_eMouseMode == M_DIBUFFERED)
			Update_DIBuffered();
		else if(m_eMouseMode == M_DIIMMEDIATE)
			Update_DIImmediate();

	}
}

/*
=====================================
Flush Mouse Data
=====================================
*/
void DI_FlushMouseData()
{
	if(m_eMouseMode == CMouse::M_DIBUFFERED)
	{
		DWORD dwItems = INFINITE; 
		m_pDIMouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
								  NULL, 
								  &dwItems, 
								   0);
	}
}


/*
=====================================
DirectInput Buffered Data
=====================================
*/
void CMouse::Update_DIBuffered()
{
	DWORD dwElements = M_DIBUFFER_SIZE;

	//Get buffered mouse data
	HRESULT hr = m_pDIMouse->GetDeviceData(sizeof(DIMOUSESTATE),
											m_aDIMouseBuf,
											&dwElements,
											0);
	if(hr != DI_OK)
	{
		//If it failed because of losing input or focus, try once more
		if((hr == DIERR_INPUTLOST)|| (hr ==DIERR_NOTACQUIRED))
		{
			ComPrintf("CMouse::Update_DIBuffered, failed to update. Lost device");
			m_eMouseState = DEVINITIALIZED;
			hr = Acquire();
		}
		else
			ComPrintf("CMouse::Update_DIBuffered, failed to update. Unknown error");
		DI_FlushMouseData();
		return;
	}

	//Reset events
	m_fXPos = m_fYPos = m_fZPos = 0;

	//Got data, loop through buffers to get events
	for(unsigned int i=0;i<dwElements;i++)
	{
		switch((int)m_aDIMouseBuf[i].dwOfs)
		{
		case DIMOFS_BUTTON0:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					In_UpdateKey(INKEY_MOUSE1, BUTTONDOWN);
				else
					In_UpdateKey(INKEY_MOUSE1, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON1:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					In_UpdateKey(INKEY_MOUSE2, BUTTONDOWN);
				else
					In_UpdateKey(INKEY_MOUSE2, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON2:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					In_UpdateKey(INKEY_MOUSE3, BUTTONDOWN);
				else
					In_UpdateKey(INKEY_MOUSE3, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON3:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					In_UpdateKey(INKEY_MOUSE4, BUTTONDOWN);
				else
					In_UpdateKey(INKEY_MOUSE4, BUTTONUP);
				break;
			}
		case DIMOFS_X:
			m_fXPos = ((int)m_aDIMouseBuf[i].dwData) * m_fXSens * m_fSens;
			break;
		case DIMOFS_Y:
			m_fYPos = ((int)m_aDIMouseBuf[i].dwData) * m_fYSens * m_fSens;
			break;
		case DIMOFS_Z:
			m_fZPos = ((int)m_aDIMouseBuf[i].dwData) * m_fSens;
			break;
		}
	}

	//Apply filter if needed
	if(m_bFilter)
	{
		m_fLastXPos = m_fXPos = (m_fXPos + m_fLastXPos)/2;
		m_fLastYPos = m_fYPos = (m_fYPos + m_fLastYPos)/2;
		m_fLastZPos = m_fZPos = (m_fZPos + m_fLastZPos)/2;
	}
	
	//Invery y-axis if needed
	if(m_bInvert)
		m_fYPos = -(m_fYPos);

	m_pCursorHandler->HandleCursorEvent(m_fXPos,m_fYPos,m_fZPos);
}


/*
=====================================
DirectInput Immediate data
=====================================
*/
void CMouse::Update_DIImmediate()
{
	//Get Mouse State
	HRESULT hr = m_pDIMouse->GetDeviceState(sizeof(DIMOUSESTATE2), 
										    m_pDIState);
	if(hr != DI_OK)
	{
		
		if((hr == DIERR_INPUTLOST)|| (hr ==DIERR_NOTACQUIRED))
		{
			ComPrintf("CMouse::Update_DIImmediate, failed to update. Lost device");
			m_eMouseState = DEVINITIALIZED;
			hr = Acquire();
		}
		else
			ComPrintf("CMouse::Update_DIImmediate, failed to update. Unknown Error");
		return;
	}

	//Update Buttons
	for(int i=0;i<M_MOUSEBUTTONS;i++)
	{
		In_UpdateKey(INKEY_MOUSE1+i, 
			m_pDIState->rgbButtons[i] & 0x80 ? BUTTONDOWN : BUTTONUP);
	}

	//Average out values if filtering is on
	if(m_bFilter)
	{
		m_fXPos = m_fLastXPos = ((m_pDIState->lX * m_fXSens * m_fSens)+ m_fLastXPos)/2;
		m_fYPos = m_fLastYPos = ((m_pDIState->lY * m_fYSens * m_fSens) + m_fLastYPos)/2;
		m_fZPos = m_fLastZPos = ((m_pDIState->lZ * m_fSens) + m_fLastZPos)/2;

	}
	else
	{
		m_fXPos = m_pDIState->lX * m_fXSens * m_fSens;
		m_fYPos = m_pDIState->lY * m_fYSens * m_fSens;
		m_fZPos = m_pDIState->lZ * m_fSens;
	}
	
	//Inverse Y-Axis if mouse is inverted
	if(m_bInvert)
		m_fYPos = -(m_fYPos);

	m_pCursorHandler->HandleCursorEvent(m_fXPos,m_fYPos,m_fZPos);
}



/*
=====================================
Win32 Mouse update
=====================================
*/
void CMouse::Update_Win32()
{
	//Get current cursor pos
	::GetCursorPos(&m_w32Pos);

	//Calc offsets
	m_fXPos = (m_w32Pos.x - m_dCenterX) * m_fXSens * m_fSens;;
	m_fYPos = (m_dCenterY - m_w32Pos.y) * m_fYSens * m_fSens;

	if(m_bFilter)
	{
		m_fLastXPos = m_fXPos = (m_fXPos + m_fLastXPos)/2;
		m_fLastYPos = m_fYPos = (m_fYPos + m_fLastYPos)/2;
	}
	if(m_bInvert)
		m_fYPos = -(m_fYPos);

	/*	
	If the most significant bit is set, the key is down, and if the least significant bit is set, 
	the key was pressed after the previous call to GetAsyncKeyState
	*/

	m_w32Buttons[0] = ::GetAsyncKeyState(VK_LBUTTON);
	m_w32Buttons[1] = ::GetAsyncKeyState(VK_RBUTTON);
	m_w32Buttons[2] = ::GetAsyncKeyState(VK_MBUTTON);

	for(int i=0;i<M_W32MOUSEBUTTONS;i++)
	{
		In_UpdateKey(INKEY_MOUSE1+i, 
			m_w32Buttons[i] & 0x80000000 ? BUTTONDOWN : BUTTONUP);
	}
	
	//Lock cursor to center of screen
	::SetCursorPos(m_dCenterX,m_dCenterY);

	m_pCursorHandler->HandleCursorEvent(m_fXPos,m_fYPos,m_fZPos);
}

//========================================================================================
//Misc Mouse Funcsions
//========================================================================================

/*
=====================================
Swtich to Exclusive mode
=====================================
*/
HRESULT	CMouse::SetExclusive(bool exclusive)
{
	if(m_pDIMouse && 
	   ((m_eMouseMode == M_DIIMMEDIATE) ||
 	    (m_eMouseMode == M_DIBUFFERED)))
	{
		if(exclusive && !m_bExclusive)
		{
			//Try changing to DI Exclusive mode is using DirectInput
			UnAcquire();
			HRESULT hr = m_pDIMouse->SetCooperativeLevel(System::GetHwnd(), 
											DISCL_FOREGROUND|DISCL_EXCLUSIVE);
			if(FAILED(hr))
				return hr;
			ComPrintf("CMouse::SetExclusive, Now in Exclusive Mode");
			hr = Acquire();
			m_bExclusive = true;
			return hr;
		}
		else if(!exclusive && m_bExclusive)
		{
			UnAcquire();
			HRESULT hr = m_pDIMouse->SetCooperativeLevel(System::GetHwnd(), 
											DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
			if(FAILED(hr))
				return hr;
			ComPrintf("CMouse::SetExclusive, Now in Non-Exclusive Mode");
			hr =  Acquire();
			m_bExclusive = false;
			return hr;
		}
		return DI_OK;
	}
	//In another Input mode. just set and return
	m_bExclusive = exclusive;
	return DI_OK;
}

/*
===========================================
Main Window resized,
updated center coords
===========================================
*/
void CMouse::Resize(int x, int y, int w, int h)
{
	m_dCenterX = (x + w)/2;
	m_dCenterY = (y + h)/2;

//	m_dCenterX = (g_hRect.left + g_hRect.right)/2;
//	m_dCenterY = (g_hRect.top + g_hRect.bottom)/2;
}


/*
=====================================
Acquire the mouse
=====================================
*/
HRESULT CMouse :: Acquire()
{
	//Already acquired
	if(m_eMouseState == DEVACQUIRED)
		return DI_OK;
	
	if((m_eMouseMode == M_WIN32) && (m_eMouseState==DEVINITIALIZED))
	{	
		::ShowCursor(false);				//Make cursor disappear for Win32 mode	
		m_eMouseState = DEVACQUIRED;
		return DI_OK;
	}
	else if(m_pDIMouse)
	{
		HRESULT hr;
		hr = m_pDIMouse->Acquire();
		if(hr == DI_OK)
		{	
			ComPrintf("CMouse::Acquire OK\n");
			m_eMouseState = DEVACQUIRED;
			return DI_OK;
		}

		In_DIErrorMessageBox(hr,"CMouse::Acquire:Unable to acquire\n");
		m_eMouseState = DEVINITIALIZED;
		return hr;
	}
	return DIERR_NOTINITIALIZED;
}

/*
=====================================
Unaquire the mouse
=====================================
*/
bool CMouse :: UnAcquire()
{
	if(m_eMouseState != DEVACQUIRED)
		return true;

	if(m_eMouseMode == M_WIN32)
	{
		::ShowCursor(true);					//Show cursor now	
		::ClipCursor(0);					//Get rid of any clipping
		m_eMouseState = DEVINITIALIZED;
		return true;
	}
	else if(m_pDIMouse)
	{
		m_pDIMouse->Unacquire();
		m_eMouseState = DEVINITIALIZED;
		ComPrintf("CMouse::UnAcquire OK\n");
		return true;
	}
	return false;
}


/*========================================================================================
Console Variable validation functions
The CVars are updated with the proposed value if the function allows it
========================================================================================*/

/*
=====================================
Change mouse mode
DI_Buffered
DI_Immediate
Win32 update
=====================================
*/
bool CMouse::CMouseMode(const CVar * var, int argc,char** argv)
{
	if(argc >= 2 && argv[1])
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if(temp < CMouse::M_DIIMMEDIATE || temp > CMouse::M_WIN32)
			{
				ComPrintf("Invalid Mouse Mode specified\n");
				return false;
			}
			
			//Allow configs to change the mousemode if its valid
			//even before the mouse actually inits
			if(m_eMouseState == DEVNONE)
			{
				m_eMouseMode = (CMouse::EMouseMode)temp;
				return true;
			}

			if(FAILED(Init(m_bExclusive,(CMouse::EMouseMode)temp)))
			{
				ComPrintf("CMouse:CMouseMode: Couldn't change to mode %d\n",temp);
				return false;
			}
			return true;
		}
		ComPrintf("CMouse::CMouseMoude:couldnt read required mode (valid 1-3)\n");
	}

	//Show current info
	ComPrintf("MouseMode is %d\n",m_eMouseMode);

	switch(m_eMouseMode)
	{
	case CMouse::M_NONE:
		ComPrintf("CMouse mode::Not intialized\n");
	case CMouse::M_DIIMMEDIATE:
		ComPrintf("CMouse mode::DirectInput Immediate mode\n");
		break;
	case CMouse::M_DIBUFFERED:
		ComPrintf("CMouse mode::DirectInput Buffered mode\n");
		break;
	case CMouse::M_WIN32:
		ComPrintf("CMouse mode::Win32 Mouse polling\n");
		break;
	}
	return false;
}


/*
======================================
Console Func - Sets X-axis Sensitivity
======================================
*/
bool CMouse::CXSens(const CVar * var, int argc,char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_fXSens = temp;
				return true;
			}
			ComPrintf("CMouse::CXSens:Invalid Value entered");
		}
	}
	ComPrintf("X-axis Sensitivity: %.2f\n", m_fXSens);
	return false;
}

/*
======================================
Console Func - Sets Y-axis Sensitivity
======================================
*/
bool CMouse::CYSens(const CVar * var, int argc,char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_fYSens = temp;
				return true;
			}
			ComPrintf("CMouse::CYSens:Invalid Value entered");
		}
	}
	ComPrintf("Y-axis Sensitivity: %.2f\n", m_fYSens);
	return false;
}


/*
======================================
Console Func - Sets master Sensitivity
======================================
*/
bool CMouse::CSens(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_fSens = temp;
				return true;
			}
			ComPrintf("CMouse::CSens:Invalid Value entered");
		}
	}
	ComPrintf("Mouse Sensitivity: %.2f\n", m_fSens);
	return false;

}

/*
======================================
Console Func - Sets Mouse Inverted states
======================================
*/
bool CMouse::CInvert(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if(temp)
				m_bInvert = true;
			else 
				m_bInvert = false;
			return true;
		}
	}
	if(m_bInvert)
		ComPrintf("CInput::Mouse is inverted\n");
	else
		ComPrintf("CInput::Mouse is not inverted\n");
	return false;
}

/*
===========================================
Sets the local mouse filter value
===========================================
*/
bool CMouse::CMouseFilter(const CVar * var, int argc, char** argv)
{
	if(argc >= 2 && argv[1])
	{
		int temp = 0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if(temp)
				m_bFilter = true;
			else 
				m_bFilter = false;
			return true;
		}
	}
	if(m_bFilter)
		ComPrintf("CInput::Mouse filter on\n");
	else
		ComPrintf("CInput::Mouse filter off\n");
	return false;
}