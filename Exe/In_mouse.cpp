#include "In_mouse.h"
#include "In_state.h"

namespace
{	const char	SZ_DIMOUSEEVENT[]= "DI MouseEvent";
}

using namespace VoidInput;

/*
=====================================
Constructor
=====================================
*/
CMouse::CMouse(CInputState * pStateManager) :
					m_pVarXSens("m_xsens","0.2",CVAR_FLOAT,CVAR_ARCHIVE),
					m_pVarYSens("m_ysens","0.2",CVAR_FLOAT,CVAR_ARCHIVE),
					m_pVarSens ("m_sens","5.0",CVAR_FLOAT,CVAR_ARCHIVE),
					m_pVarInvert("m_invert","0",CVAR_BOOL,CVAR_ARCHIVE),
					m_pVarMode("m_mode","1",CVAR_INT,CVAR_ARCHIVE),
					m_pVarFilter("m_filter","0",CVAR_BOOL, CVAR_ARCHIVE)
{
	m_pStateManager = pStateManager;

	m_pDIMouse = 0;
	m_eMouseState = DEVNONE;
	m_eMouseMode = M_NONE;

	m_bExclusive = false;
	
	m_pDIState = 0;

	m_fXPos= m_fYPos= m_fZPos=0.0f;
	m_fLastXPos = m_fLastYPos = m_fLastZPos = 0.0f;
	
	m_dCenterX = m_dCenterY = 0;

	System::GetConsole()->RegisterCVar(&m_pVarXSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarYSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarMode,this);
	System::GetConsole()->RegisterCVar(&m_pVarFilter);
	System::GetConsole()->RegisterCVar(&m_pVarInvert);
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

	if(m_pDIState)
		delete m_pDIState;
	m_pDIMouse = 0;

	m_pStateManager =0;
}

/*
=====================================
Intialize the mouse
The mouse mode SHOULD have been set
by a config file by now, or else it 
will default to DI_IMMEDIATE
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
		else	//If read nothing from config, then default to IMMEDIATE
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
	//Update state
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
	HRESULT hr = (VoidInput::GetDirectInput())->CreateDeviceEx(GUID_SysMouse, 
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
	m_hDIMouseEvent =  ::CreateEvent(0,			// pointer to security attributes
							FALSE,				// flag for manual-reset event
						    FALSE,				// flag for initial state
							SZ_DIMOUSEEVENT);   // pointer to event-object name

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
EDeviceState CMouse::GetDeviceState() const
{ return m_eMouseState;
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
void CMouse::DI_FlushMouseData()
{
	if(m_eMouseMode == M_DIBUFFERED)
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
					m_pStateManager->UpdateKey(INKEY_MOUSE1, BUTTONDOWN);
				else
					m_pStateManager->UpdateKey(INKEY_MOUSE1, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON1:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					m_pStateManager->UpdateKey(INKEY_MOUSE2, BUTTONDOWN);
				else
					m_pStateManager->UpdateKey(INKEY_MOUSE2, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON2:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					m_pStateManager->UpdateKey(INKEY_MOUSE3, BUTTONDOWN);
				else
					m_pStateManager->UpdateKey(INKEY_MOUSE3, BUTTONUP);
				break;
			}
		case DIMOFS_BUTTON3:
			{
				if(m_aDIMouseBuf[i].dwData & 0x80)
					m_pStateManager->UpdateKey(INKEY_MOUSE4, BUTTONDOWN);
				else
					m_pStateManager->UpdateKey(INKEY_MOUSE4, BUTTONUP);
				break;
			}
		case DIMOFS_X:
			m_fLastXPos = m_fXPos;
			m_fXPos = ((int)m_aDIMouseBuf[i].dwData);
			break;
		case DIMOFS_Y:
			m_fLastYPos = m_fYPos;
			m_fYPos = ((int)m_aDIMouseBuf[i].dwData);
			break;
		case DIMOFS_Z:
			m_fLastZPos = m_fZPos;
			m_fZPos = ((int)m_aDIMouseBuf[i].dwData);
			break;
		}
	}


	//Inverse Y-Axis if mouse is inverted
	if(m_pVarInvert.bval)
		m_fYPos = -(m_fYPos);

	float x, y, z;

	//Average out values if filtering is on
	if(m_pVarFilter.bval)
	{
		x = (m_fLastXPos + m_fXPos) * m_pVarXSens.fval * m_pVarSens.fval / 2;
		y = (m_fLastYPos + m_fYPos) * m_pVarYSens.fval * m_pVarSens.fval / 2;
		z = (m_fLastZPos + m_fZPos) * m_pVarSens.fval / 2;
	}
	else
	{
		x = m_fXPos * m_pVarXSens.fval * m_pVarSens.fval;
		y = m_fYPos * m_pVarYSens.fval * m_pVarSens.fval;
		z = m_fZPos * m_pVarSens.fval;
	}

	m_pStateManager->UpdateCursor(x, y, z);
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
		m_pStateManager->UpdateKey(INKEY_MOUSE1+i, 
			m_pDIState->rgbButtons[i] & 0x80 ? BUTTONDOWN : BUTTONUP);
	}

	m_fLastXPos = m_fXPos;
	m_fLastYPos = m_fYPos;
	m_fLastZPos = m_fZPos;

	m_fXPos = m_pDIState->lX;
	m_fYPos = m_pDIState->lY;
	m_fZPos = m_pDIState->lZ;

	//Inverse Y-Axis if mouse is inverted
	if(m_pVarInvert.bval)
		m_fYPos = -(m_fYPos);

	float x, y, z;

	//Average out values if filtering is on
	if(m_pVarFilter.bval)
	{
		x = (m_fLastXPos + m_fXPos) * m_pVarXSens.fval * m_pVarSens.fval / 2;
		y = (m_fLastYPos + m_fYPos) * m_pVarYSens.fval * m_pVarSens.fval / 2;
		z = (m_fLastZPos + m_fZPos) * m_pVarSens.fval / 2;
	}
	else
	{
		x = m_fXPos * m_pVarXSens.fval * m_pVarSens.fval;
		y = m_fYPos * m_pVarYSens.fval * m_pVarSens.fval;
		z = m_fZPos * m_pVarSens.fval;
	}

	m_pStateManager->UpdateCursor(x, y, z);
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
	m_fLastXPos = m_fXPos;
	m_fLastYPos = m_fYPos;
	m_fXPos = (m_w32Pos.x - m_dCenterX);
	m_fYPos = (m_dCenterY - m_w32Pos.y);

	//Inverse Y-Axis if mouse is inverted
	if(!m_pVarInvert.bval)
		m_fYPos = -(m_fYPos);

	//Average out values if filtering is on
	float x, y, z;
	if(m_pVarFilter.bval)
	{
		x = (m_fLastXPos + m_fXPos) * m_pVarXSens.fval * m_pVarSens.fval / 2;
		y = (m_fLastYPos + m_fYPos) * m_pVarYSens.fval * m_pVarSens.fval / 2;
		z = (m_fLastZPos + m_fZPos) * m_pVarSens.fval / 2;
	}
	else
	{
		x = m_fXPos * m_pVarXSens.fval * m_pVarSens.fval;
		y = m_fYPos * m_pVarYSens.fval * m_pVarSens.fval;
		z = m_fZPos * m_pVarSens.fval;
	}


	if(m_pVarFilter.bval)
	{
		m_fLastXPos = m_fXPos = (m_fXPos + m_fLastXPos)/2;
		m_fLastYPos = m_fYPos = (m_fYPos + m_fLastYPos)/2;
	}


	/*	
	If the most significant bit is set, the key is down, and if the least significant bit is set, 
	the key was pressed after the previous call to GetAsyncKeyState
	*/

	m_w32Buttons[0] = ::GetAsyncKeyState(VK_LBUTTON);
	m_w32Buttons[1] = ::GetAsyncKeyState(VK_RBUTTON);
	m_w32Buttons[2] = ::GetAsyncKeyState(VK_MBUTTON);

	for(int i=0;i<M_W32MOUSEBUTTONS;i++)
	{
		m_pStateManager->UpdateKey(INKEY_MOUSE1+i, 
			m_w32Buttons[i] & 0x80000000 ? BUTTONDOWN : BUTTONUP);
	}
	
	//Lock cursor to center of screen
	::SetCursorPos(m_dCenterX,m_dCenterY);

	m_pStateManager->UpdateCursor(x, y, z);
}

/*
========================================================================================
Misc Mouse Funcsions
========================================================================================
*/
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
		ComPrintf("CMouse::Acquire Failed\n");
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

/*
========================================================================================
Console Variable validation functions
The CVars are updated with the proposed value if the function allows it
========================================================================================
*/
//CVar Handler
bool CMouse::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == &m_pVarXSens)
		return CXSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarYSens)
		return CYSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarSens)
		return CSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarMode)
		return CMouseMode((CVar*)cvar,parms);
	return false;
}

/*
=====================================
Change mouse mode
DI_Buffered
DI_Immediate
Win32 update
=====================================
*/
bool CMouse::CMouseMode(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		int mode= parms.IntTok(1);
		
		if(mode < CMouse::M_DIIMMEDIATE || mode > CMouse::M_WIN32)
		{
			ComPrintf("Invalid Mouse Mode specified\n");
			return false;
		}
			
		//Allow configs to change the mousemode if its valid
		//even before the mouse actually inits
		if(m_eMouseState == DEVNONE)
		{
			m_eMouseMode = (CMouse::EMouseMode)mode;
			return true;
		}

		if(FAILED(Init(m_bExclusive,(CMouse::EMouseMode)mode)))
		{
			ComPrintf("CMouse:CMouseMode: Couldn't change to mode %d\n",mode);
			return false;
		}
		return true;
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
bool CMouse::CXSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
				return true;
		ComPrintf("CMouse::CXSens:Invalid Value entered");
	}
	ComPrintf("X-axis Sensitivity: %.2f\n", m_pVarXSens.fval);
	return false;
}

/*
======================================
Console Func - Sets Y-axis Sensitivity
======================================
*/
bool CMouse::CYSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
				return true;
		ComPrintf("CMouse::CYSens:Invalid Value entered");
	}
	ComPrintf("Y-axis Sensitivity: %.2f\n", m_pVarYSens.fval);
	return false;
}

/*
======================================
Console Func - Sets master Sensitivity
======================================
*/
bool CMouse::CSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
				return true;
		ComPrintf("CMouse::CSens:Invalid Value entered");
	}
	ComPrintf("Mouse Sensitivity: %.2f\n", m_pVarSens.fval);
	return false;
}
