#include "In_kb.h"

/*
========================================================================================
Additional Virtual key constants
sort of an hack. not sure if they will work on all keyboards
========================================================================================
*/
namespace
{
	enum
	{
		VK_SEMICOLON= 186,
		VK_EQUALS	= 187,
		VK_COMMA	= 188,
		VK_MINUS	= 189,
		VK_PERIOD	= 190,
		VK_SLASH	= 191,	
		VK_GRAVE	= 192,
		
		VK_LBRACKET	= 219,
		VK_BACKSLASH= 220,
		VK_RBRACKET	= 221,
		VK_QUOTE	= 222
	};

	//Crappy. Need this for the Win32 Hook function. Cant find a way around it
	VoidInput::CKeyboard	* m_pKeyboard=0;	
}

using namespace VoidInput;

//========================================================================================
//========================================================================================

/*
=====================================
Constructor
=====================================
*/
CKeyboard::CKeyboard(CInputState * pStateManager) : 
				m_pVarKbMode("kb_mode","1",CVAR_INT,CVAR_ARCHIVE)

{
	m_pKeyboard = this;

	m_pStateManager = pStateManager;

	m_pDIKb = 0;
	m_eKbState = DEVNONE;

	m_bExclusive = false;
	m_eKbMode = KB_NONE;

	m_pDIKb=0;

	memset(m_aCharVal, 0,  IN_NUMKEYS*sizeof(int));
	memset(m_aKeyState, 0, IN_NUMKEYS*sizeof(BYTE));
	memset(m_aDIBufKeydata,0,KB_DIBUFFERSIZE*sizeof(DIDEVICEOBJECTDATA));

	System::GetConsole()->RegisterCVar(&m_pVarKbMode,this);
}

/*
=====================================
Destructor
=====================================
*/
CKeyboard::~CKeyboard()
{
	Shutdown();
	m_pDIKb = 0;
	m_pStateManager = 0;
	m_pKeyboard = 0;
}

/*
=====================================
Shutdown the keyboard interface
=====================================
*/
void CKeyboard :: Shutdown()
{
	FlushKeyBuffer();

	switch(m_eKbMode)
	{
	case KB_WIN32HOOK:
    case KB_WIN32POLL:
		{
			Win32_Shutdown();
			break;
		}
	case KB_DIBUFFERED:
	case KB_DIIMMEDIATE:
		{
			DI_Shutdown();
			break;
		}
	}
	m_eKbState = DEVNONE;
	m_eKbMode = KB_NONE;
	ComPrintf("CKeyboard::Shutdown:OK\n");
}

/*
===========================================
Initializes the keyboard to the given mode
===========================================
*/
HRESULT	CKeyboard::Init(int exclusive,EKbMode mode)
{
	if(exclusive)
		m_bExclusive = true;

	//mode specified as NONE when the device is first initialized
	if(mode == KB_NONE)
	{
		//then default to what we read from the config files
		if(m_eKbMode != KB_NONE)
			mode = m_eKbMode;
		//If read nothing from config, then default to BUFFERED
		else
			mode = KB_DIBUFFERED;
	}

	//If the device is active and is in the same mode 
	//then we don't need to do anything
	if((m_eKbMode == mode) && (m_eKbState != DEVNONE))
	{
		ComPrintf("CKeyboard::InitMode: Already in given mode\n");
		return DI_OK;
	}

	HRESULT hr;
	switch(mode)
	{
	case KB_WIN32HOOK:
    case KB_WIN32POLL:
		{
			//Initialize to the mode
			hr = Win32_Init(mode);
			if(FAILED(hr))
				return hr;
			break;
		}
	case KB_DIBUFFERED:
	case KB_DIIMMEDIATE:
		{
			//Initialize to the mode
			hr = DI_Init(mode);
			if(FAILED(hr))
				return hr;
			break;
		}
	}
	m_eKbState = DEVINITIALIZED;
	
	//Try to acquire the kb now
	Acquire();
	return DI_OK;
}

/*
=====================================
DirectInput init
=====================================
*/
HRESULT CKeyboard::DI_Init(EKbMode mode)
{
	if(m_eKbState != DEVNONE)
		Shutdown();

	//Create the device
	HRESULT hr = (VoidInput::GetDirectInput())->CreateDeviceEx( GUID_SysKeyboard, 
										IID_IDirectInputDevice7, 
										(void**)&m_pDIKb, 
										NULL); 
	//Failed
	if(FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init:Keyboard Initialization failed\n");
		return hr;
	}
	
	//Set data format
	hr = m_pDIKb->SetDataFormat(&c_dfDIKeyboard); 
	if(FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init :Data Format failed\n");
		return hr;
	}

	//Set coop level depending on exclusive flag
	if(!m_bExclusive)
		hr = m_pDIKb->SetCooperativeLevel(System::GetHwnd(),
										  DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	else
		hr = m_pDIKb->SetCooperativeLevel(System::GetHwnd(),
										  DISCL_FOREGROUND | DISCL_EXCLUSIVE); 
	
	if (FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init :Coop Level failed\n");
		return hr;
	}

	//Set Event Notification
	const char szKbEventName[] = "DI Keyboard";

	m_hDIKeyboardEvent =  ::CreateEvent(0,		// pointer to security attributes
							FALSE,				// flag for manual-reset event
						    FALSE,				// flag for initial state
							szKbEventName);     // pointer to event-object name

	if(m_hDIKeyboardEvent == NULL)
	{
		ComPrintf("CKeyboard::DI_Init : Unable to set event\n");
		return ::GetLastError();
	}


	hr = m_pDIKb->SetEventNotification(m_hDIKeyboardEvent);
	if(FAILED(hr))
	{
		ComPrintf("CKeyboard::DI_Init : Unable to set event\n");
		return hr;
	}

	//Set Mode specific properties
	if(mode == KB_DIBUFFERED)
	{
		//Setup buffers
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = KB_DIBUFFERSIZE;
	
		hr = m_pDIKb->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph );
		if(FAILED(hr))
		{
			ComPrintf("CKeyboard::Init :Set Buffer failed\n");
			return hr;
		}

		//Set Update pointer
		ComPrintf("CKeyboard::InitMode:Initialized DI Buffered mode\n");
		m_eKbMode = KB_DIBUFFERED;
	}
	else if(mode == KB_DIIMMEDIATE)
	{
		ComPrintf("CKeyboard::InitMode:Intialized to DI Immediate mode\n");
		m_eKbMode = KB_DIIMMEDIATE;
	}
	SetCharTable(KB_DIIMMEDIATE);
	return DI_OK;
}

/*
=====================================
DirectInput shutdown
=====================================
*/
bool CKeyboard::DI_Shutdown()
{
	if(m_pDIKb)
	{
		UnAcquire();
		SetExclusive(false);
		m_pDIKb->SetEventNotification(0);
		m_pDIKb->Release();
		m_pDIKb = NULL;
	}
	return true;
}

/*
===========================================
Win32 Initialize
===========================================
*/
HRESULT	CKeyboard::Win32_Init(EKbMode mode)
{
	if(m_eKbState != DEVNONE)
		Shutdown();

	if(mode == KB_WIN32POLL)
		ComPrintf("CKeyboard::InitMode:Initialized Win32 Polling mode\n");
	else
		ComPrintf("CKeyboard::InitMode:Initialized Win32 Hook\n");

	m_eKbMode = mode;
	SetCharTable(mode);
	return DI_OK;
}

/*
===========================================
Win32 Shutdown
===========================================
*/
bool CKeyboard::Win32_Shutdown()
{
 	UnAcquire();
	SetExclusive(false);
	return true;
}


/*
===========================================
Acquire the keyboard
===========================================
*/
HRESULT CKeyboard::Acquire()
{
	if(m_eKbState == DEVACQUIRED)
		return S_OK;

	if(m_eKbMode == KB_NONE)
	{
		ComPrintf("CKeyboard::Acquire failed. Keyboard is not intialized\n");
		return DIERR_NOTINITIALIZED;
	}

	if(m_eKbMode == KB_WIN32HOOK)
	{
		hWinKbHook = ::SetWindowsHookEx(WH_KEYBOARD,		// type of hook to install
									&Win32_KeyboardProc,	// address of hook procedure
									System::GetHInstance(),	// handle to application instance
								  ::GetCurrentThreadId());	// identity of thread to install hook for
	
		if(hWinKbHook == 0)
		{
			ComPrintf("CKeyboard::Win32_Init : Failed to set Kb Hook\n");
			return E_FAIL;
		}
	}
	else if((m_eKbMode == KB_DIBUFFERED || m_eKbMode == KB_DIIMMEDIATE))
	{
		if(!m_pDIKb)
		{
			ComPrintf("CKeyboard::Acquire failed. DI is not intialized\n");
			return DIERR_NOTINITIALIZED;
		}

		HRESULT hr = m_pDIKb->Acquire();
		if(FAILED(hr))
		{
			ComPrintf("CKeyboard::Unable to acquire the keyboard\n");
			m_eKbState = DEVINITIALIZED;
			return hr;
		}
	}
	ComPrintf("CKeyboard::Acquire :OK\n");
	m_eKbState = DEVACQUIRED;
	return DI_OK;
}

/*
===========================================
Unacquire the keyboard
===========================================
*/
bool CKeyboard::UnAcquire()
{
	if(m_eKbState == DEVACQUIRED)
	{
		if(m_eKbMode == KB_WIN32HOOK)
		{
			if(hWinKbHook)
				UnhookWindowsHookEx(hWinKbHook);
		}
		else if(m_eKbMode == KB_DIBUFFERED || m_eKbMode == KB_DIIMMEDIATE)
		{
			FlushKeyBuffer();
			m_pDIKb->Unacquire();
		}
		m_eKbState = DEVINITIALIZED;
		ComPrintf("CKeyboard::UnAcquire :OK\n");
	}
	return true;
}

/*
===========================================
Toggle Exclusive Mode
===========================================
*/
HRESULT	CKeyboard::SetExclusive(bool exclusive)
{
	if(m_pDIKb && 
	   ((m_eKbMode == KB_DIIMMEDIATE) ||
 	    (m_eKbMode == KB_DIBUFFERED)))
	{
		if(exclusive && !m_bExclusive)
		{
			//Try changing to DI Exclusive mode is using DirectInput
			UnAcquire();
			HRESULT hr = m_pDIKb->SetCooperativeLevel(System::GetHwnd(), 
													DISCL_FOREGROUND|DISCL_EXCLUSIVE);
			if(FAILED(hr))
				return hr;
			ComPrintf("CKeyboard::SetExclusive, Now in Exclusive Mode\n");
			Acquire();
			m_bExclusive = true;
			return hr;
		}
		else if(!exclusive && m_bExclusive)
		{
			UnAcquire();
			HRESULT hr = m_pDIKb->SetCooperativeLevel(System::GetHwnd(), 
														DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
			if(FAILED(hr))
				return hr;
			ComPrintf("CKeyboard::SetExclusive, Now in Non-Exclusive Mode\n");
			Acquire();
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
Returns current State
===========================================
*/
int	 CKeyboard::GetDeviceState() const
{ return m_eKbState; 
}

/*
===========================================
Flushes current data
===========================================
*/
void CKeyboard::FlushKeyBuffer()
{
	if(m_pDIKb && m_eKbMode == CKeyboard::KB_DIBUFFERED)
	{
		DWORD num= INFINITE;
		m_pDIKb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),NULL,&num,0);
	}
	m_pStateManager->FlushKeys();
}


bool CKeyboard::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == &m_pVarKbMode)
		return CKBMode((CVar*)cvar, parms);
	return false;
}


//========================================================================================
/*
===========================================
Update the Keyboard,
called by Input Main
===========================================
*/
void CKeyboard::Update()
{
	if(m_eKbState != DEVACQUIRED)
		return;

	switch(m_eKbMode)
	{
	case KB_DIBUFFERED:
		{
			if(WaitForSingleObject(m_hDIKeyboardEvent, 0) == WAIT_OBJECT_0)
			{
				Update_DIBuffered();
				return;
			}
			break;
		}
	case KB_DIIMMEDIATE:
		{
			if(WaitForSingleObject(m_hDIKeyboardEvent, 0) == WAIT_OBJECT_0)
			{
				Update_DIImmediate();
				return;
			}
			break;
		}
    case KB_WIN32POLL:
		{
			Update_Win32();
			break;
		}
	}
	m_pStateManager->DispatchKeys();
}

/*
===========================================
Query DirectInput Bufferedmode
===========================================
*/
void CKeyboard::Update_DIBuffered() 
{
	//number of elements in query buffer
    DWORD numElements = KB_DIBUFFERSIZE;	

	//query for new info
	HRESULT hr = m_pDIKb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
										m_aDIBufKeydata,
										&numElements,
										0);
	//Unable to get data
	if (hr != DI_OK)
	{
		//unknown error, kill the keyboard here ?
		if (hr!=DIERR_NOTACQUIRED)
			ComPrintf("CKeyboard::Update_DIBuffered:Failed,Unable to Query keyboard\n");
		else
		{
			//Try to acquire it again
			m_eKbState = DEVINITIALIZED;
			ComPrintf("CKeyboard::Update_DIBuffered:Failed,Keyboard is not acquired\n");
			Acquire();
		}
		
		//Flush and Return
		FlushKeyBuffer();
		return;
	}
	
	//Sucessfully Polled Keyboard
	//Update the items that changed
	for(unsigned int i=0;i<numElements;i++)
	{
		if(m_aDIBufKeydata[i].dwData & 0x80)
			m_pStateManager->UpdateKey(m_aCharVal[m_aDIBufKeydata[i].dwOfs], BUTTONDOWN);
		else
			m_pStateManager->UpdateKey(m_aCharVal[m_aDIBufKeydata[i].dwOfs], BUTTONUP);
	}
}

/*
=====================================
DirectInput Immediate mode
=====================================
*/
void CKeyboard::Update_DIImmediate()
{
	//Get the input's device state, and put the state in dims
    HRESULT hr = m_pDIKb->GetDeviceState(sizeof(m_aKeyState),
										 &m_aKeyState);
    if(hr != DI_OK)  
	{
		if (hr!=DIERR_NOTACQUIRED)
			ComPrintf("CKeyboard::Update_DIIImmediate: Unable to get device state\n");
		else
		{
			//we've lost contact with the keyboard try to reacquire
			ComPrintf("CKeyboard::UpdateDI:Failed, Keyboard is not acquired\n");
			m_eKbState = DEVINITIALIZED;
			Acquire();
		}
		//Flush and return
		FlushKeyBuffer();
		return;
	}

	//Sucessfully polled keyboard
	//Loop through the entire buffer to check for changed items
	for(int i=0;i<IN_NUMKEYS;i++)
	{
		if(m_aKeyState[i] & 0x80) 
			m_pStateManager->UpdateKey(m_aCharVal[i],BUTTONDOWN);
		else
			m_pStateManager->UpdateKey(m_aCharVal[i],BUTTONUP);
	}
}

/*
=====================================
Win32 Keyboard Poll function
=====================================
*/
void CKeyboard::Update_Win32()
{
	//Get the input's device state, and put the state in dims
    if(!::GetKeyboardState(m_aKeyState))
	{
		ComPrintf("CKeyboard::Update_Win32: Unable to get device state\n");
		FlushKeyBuffer();
		return;
	}

	if(m_aKeyState[VK_SHIFT] & 0x80)
	{
		if(::GetAsyncKeyState(VK_LSHIFT) & 0x80000000)
			m_aKeyState[INKEY_LEFTSHIFT] = 0x80;
		else 
			m_aKeyState[INKEY_RIGHTSHIFT] = 0x80;
	}

	if(m_aKeyState[VK_MENU] & 0x80)
	{
		if(::GetAsyncKeyState(VK_LMENU) & 0x80000000)
			m_aKeyState[INKEY_LEFTALT] = 0x80;
		else 
			m_aKeyState[INKEY_RIGHTALT] = 0x80;
	}

	if(m_aKeyState[VK_CONTROL] & 0x80)
	{
		if(::GetAsyncKeyState(VK_LCONTROL) & 0x80000000)
			m_aKeyState[INKEY_LEFTCTRL] = 0x80;
		else 
			m_aKeyState[INKEY_RIGHTCTRL] = 0x80;
	}

	for(int i=0;i<IN_NUMKEYS;i++)
	{
		if(m_aKeyState[i] & 0x80) 
			m_pStateManager->UpdateKey(m_aCharVal[i],BUTTONDOWN);
		else
			m_pStateManager->UpdateKey(m_aCharVal[i],BUTTONUP);
	}
} 

//======================================================================================
//======================================================================================

/*
=====================================
Change keyboard mode
=====================================
*/
bool CKeyboard::CKBMode(const CVar * cvar, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		int mode = parms.IntTok(1);

		//Check for Vaild value
		if(mode < KB_DIBUFFERED ||  mode > KB_WIN32POLL)
		{
			ComPrintf("CKeyboard::CKBMode:Invalid mode\n");
			return false;
		}

		//Allow configs to change the mode if its valid
		//even before the mouse actually inits
		if(m_eKbState == DEVNONE) 
		{
			m_eKbMode = (EKbMode)mode;
			return true;
		}

		if(FAILED(Init(m_bExclusive,(EKbMode)mode)))
		{
			ComPrintf("CKeyboard:CKBMode: Couldnt change to mode %d\n",mode);
			return false;
		}
		return true;
	}

	ComPrintf("Keyboard Mode is %d\n",m_eKbMode);
	switch(m_eKbMode)
	{
	case KB_NONE:
		ComPrintf("CKeyboard mode::No mode set\n");
	case KB_DIBUFFERED:
		ComPrintf("CKeyboard mode::DirectInput Buffered mode\n");
		break;
	case KB_DIIMMEDIATE:
		ComPrintf("CKeyboard mode::DirectInput Immediate mode\n");
		break;
	case KB_WIN32POLL:
		ComPrintf("CKeyboard mode::Win32 Keyboard polling\n");
		break;
	case KB_WIN32HOOK:
		ComPrintf("CKeyboard mode::Win32 Keyboard Hooks\n");
		break;
	}
	return false;
}

/*
=====================================
Creates Key Id translation table
=====================================
*/
void CKeyboard::SetCharTable(CKeyboard::EKbMode mode)
{
	switch(mode)
	{
	case CKeyboard::KB_WIN32HOOK:
    case CKeyboard::KB_WIN32POLL:
		{
			m_aCharVal[VK_BACK] = INKEY_BACKSPACE;
			m_aCharVal[VK_TAB] =  INKEY_TAB;

			m_aCharVal[VK_RETURN] =	INKEY_ENTER;

			m_aCharVal[VK_SHIFT] =	INKEY_LEFTSHIFT;
			m_aCharVal[VK_CONTROL]= INKEY_LEFTCTRL;
			m_aCharVal[VK_MENU] =	INKEY_LEFTALT;
			m_aCharVal[VK_PAUSE] =  INKEY_PAUSE;
			m_aCharVal[VK_CAPITAL]= INKEY_CAPSLOCK;

			m_aCharVal[VK_SPACE] =	INKEY_SPACE;
			m_aCharVal[VK_PRIOR] =	INKEY_PGUP;
			m_aCharVal[VK_NEXT] =	INKEY_PGDN;
			m_aCharVal[VK_END] =	INKEY_END;
			m_aCharVal[VK_HOME] =	INKEY_HOME;
			m_aCharVal[VK_LEFT] =	INKEY_LEFTARROW;
			m_aCharVal[VK_UP] =		INKEY_UPARROW;
			m_aCharVal[VK_RIGHT] =	INKEY_RIGHTARROW;
			m_aCharVal[VK_DOWN] =	INKEY_DOWNARROW;
			
			m_aCharVal[VK_EXECUTE] = INKEY_NUMENTER;
			m_aCharVal[VK_SNAPSHOT] =INKEY_PRINTSCRN;
			m_aCharVal[VK_INSERT] =	 INKEY_INS;
			m_aCharVal[VK_DELETE] =	 INKEY_DEL;

			m_aCharVal[VK_ESCAPE] =	INKEY_ESCAPE;
			
			m_aCharVal['0'] = '0';
			m_aCharVal['1'] = '1';
			m_aCharVal['2'] = '2';
			m_aCharVal['3'] = '3';
			m_aCharVal['4'] = '4';
			m_aCharVal['5'] = '5';
			m_aCharVal['6'] = '6';
			m_aCharVal['7'] = '7';
			m_aCharVal['8'] = '8';
			m_aCharVal['9'] = '9';

			m_aCharVal['A'] = 'a';
			m_aCharVal['B'] = 'b';
			m_aCharVal['C'] = 'c';
			m_aCharVal['D'] = 'd';
			m_aCharVal['E'] = 'e';
			m_aCharVal['F'] = 'f';
			m_aCharVal['G'] = 'g';
			m_aCharVal['H'] = 'h';
			m_aCharVal['I'] = 'i';
			m_aCharVal['J'] = 'j';
			m_aCharVal['K'] = 'k';
			m_aCharVal['L'] = 'l';
			m_aCharVal['M'] = 'm';
			m_aCharVal['N'] = 'n';
			m_aCharVal['O'] = 'o';
			m_aCharVal['P'] = 'p';
			m_aCharVal['Q'] = 'q';
			m_aCharVal['R'] = 'r';
			m_aCharVal['S'] = 's';
			m_aCharVal['T'] = 't';
			m_aCharVal['U'] = 'u';
			m_aCharVal['V'] = 'v';
			m_aCharVal['W'] = 'w';
			m_aCharVal['X'] = 'x';
			m_aCharVal['Y'] = 'y';
			m_aCharVal['Z'] = 'z';

			m_aCharVal[VK_NUMPAD0] =INKEY_NUM0;
			m_aCharVal[VK_NUMPAD1] =INKEY_NUM1;
			m_aCharVal[VK_NUMPAD2] =INKEY_NUM2;
			m_aCharVal[VK_NUMPAD3] =INKEY_NUM3;
			m_aCharVal[VK_NUMPAD4] =INKEY_NUM4;
			m_aCharVal[VK_NUMPAD5] =INKEY_NUM5;
			m_aCharVal[VK_NUMPAD6] =INKEY_NUM6;
			m_aCharVal[VK_NUMPAD7] =INKEY_NUM7;
			m_aCharVal[VK_NUMPAD8] =INKEY_NUM8;
			m_aCharVal[VK_NUMPAD9] =INKEY_NUM9;
			
			m_aCharVal[VK_MULTIPLY] =	INKEY_NUMSTAR;
			m_aCharVal[VK_ADD] =		INKEY_NUMPLUS;
		//	m_aCharVal[VK_SEPARATOR] =
			m_aCharVal[VK_SUBTRACT] =	INKEY_NUMMINUS;
			m_aCharVal[VK_DECIMAL] =	INKEY_NUMPERIOD;
			m_aCharVal[VK_DIVIDE] =		INKEY_NUMSLASH;

			m_aCharVal[VK_F1] = INKEY_F1;
			m_aCharVal[VK_F2] = INKEY_F2;
			m_aCharVal[VK_F3] = INKEY_F3;
			m_aCharVal[VK_F4] = INKEY_F4;
			m_aCharVal[VK_F5] = INKEY_F5;
			m_aCharVal[VK_F6] = INKEY_F6;
			m_aCharVal[VK_F7] = INKEY_F7;
			m_aCharVal[VK_F8] = INKEY_F8;
			m_aCharVal[VK_F9] = INKEY_F9;
			m_aCharVal[VK_F10]= INKEY_F10;
			m_aCharVal[VK_F11]= INKEY_F11;
			m_aCharVal[VK_F12]= INKEY_F12;

			m_aCharVal[VK_NUMLOCK] = INKEY_NUMLOCK;
			m_aCharVal[VK_SCROLL] =  INKEY_SCROLLLOCK;

			m_aCharVal[VK_SEMICOLON] =  ';';
			m_aCharVal[VK_EQUALS] =  '=';
			m_aCharVal[VK_COMMA] =  ',';
			m_aCharVal[VK_MINUS] =  '-';
			m_aCharVal[VK_PERIOD] =  '.';
			m_aCharVal[VK_SLASH] =  '/';
			m_aCharVal[VK_GRAVE] =  '`';
			m_aCharVal[VK_LBRACKET] =  '[';
			m_aCharVal[VK_BACKSLASH] =  '\\';
			m_aCharVal[VK_RBRACKET] =  ']';
			m_aCharVal[VK_QUOTE] =  '\'';

			m_aCharVal[VK_CLEAR] =

			m_aCharVal[VK_LWIN] = 0;
			m_aCharVal[VK_RWIN] = 0;
			m_aCharVal[VK_APPS] = 0;

			m_aCharVal[VK_HELP] =   0;
			m_aCharVal[VK_SELECT] = 0;

			m_aCharVal[VK_LSHIFT] = INKEY_LEFTSHIFT;
			m_aCharVal[VK_RSHIFT] = INKEY_RIGHTSHIFT;
			m_aCharVal[VK_RCONTROL] = INKEY_RIGHTCTRL;
			m_aCharVal[VK_LCONTROL] = INKEY_LEFTCTRL;
			m_aCharVal[VK_LMENU]	= INKEY_LEFTALT;
			m_aCharVal[VK_RMENU]	= INKEY_RIGHTALT;
			break;
		}
	case CKeyboard::KB_DIBUFFERED:
	case CKeyboard::KB_DIIMMEDIATE:
		{
			m_aCharVal[DIK_ESCAPE] = INKEY_ESCAPE;
			m_aCharVal[DIK_1] = '1';
			m_aCharVal[DIK_2] = '2';
			m_aCharVal[DIK_3] = '3';
			m_aCharVal[DIK_4] = '4';
			m_aCharVal[DIK_5] = '5';
			m_aCharVal[DIK_6] = '6';
			m_aCharVal[DIK_7] = '7';
			m_aCharVal[DIK_8] = '8';
			m_aCharVal[DIK_9] = '9';
			m_aCharVal[DIK_0] = '0';
			m_aCharVal[DIK_MINUS] = '-';
			m_aCharVal[DIK_EQUALS] = '=';
			m_aCharVal[DIK_BACK] = INKEY_BACKSPACE;
			m_aCharVal[DIK_TAB]  = INKEY_TAB;
			m_aCharVal[DIK_Q] = 'q';
			m_aCharVal[DIK_W] = 'w';
			m_aCharVal[DIK_E] = 'e';
			m_aCharVal[DIK_R] = 'r';
			m_aCharVal[DIK_T] = 't';
			m_aCharVal[DIK_Y] = 'y';
			m_aCharVal[DIK_U] = 'u';
			m_aCharVal[DIK_I] = 'i';
			m_aCharVal[DIK_O] = 'o';
			m_aCharVal[DIK_P] = 'p';
			m_aCharVal[DIK_LBRACKET] = '[';
			m_aCharVal[DIK_RBRACKET] = ']';
			m_aCharVal[DIK_RETURN] = INKEY_ENTER;
			m_aCharVal[DIK_LCONTROL] = INKEY_LEFTCTRL;
			m_aCharVal[DIK_A] = 'a';
			m_aCharVal[DIK_S] = 's';
			m_aCharVal[DIK_D] = 'd';
			m_aCharVal[DIK_F] = 'f';
			m_aCharVal[DIK_G] = 'g';
			m_aCharVal[DIK_H] = 'h';
			m_aCharVal[DIK_J] = 'j';
			m_aCharVal[DIK_K] = 'k';
			m_aCharVal[DIK_L] = 'l';
			m_aCharVal[DIK_SEMICOLON] = ';';
			m_aCharVal[DIK_APOSTROPHE] = '\'';
			m_aCharVal[DIK_GRAVE] = '`';
			m_aCharVal[DIK_LSHIFT] = INKEY_LEFTSHIFT;
			m_aCharVal[DIK_BACKSLASH] = '\\';
			m_aCharVal[DIK_Z] = 'z';
			m_aCharVal[DIK_X] = 'x';
			m_aCharVal[DIK_C] = 'c';
			m_aCharVal[DIK_V] = 'v';
			m_aCharVal[DIK_B] = 'b';
			m_aCharVal[DIK_N] = 'n';
			m_aCharVal[DIK_M] = 'm';
			m_aCharVal[DIK_COMMA] = ',';
			m_aCharVal[DIK_PERIOD] = '.';
			m_aCharVal[DIK_SLASH] = '/';
			m_aCharVal[DIK_RSHIFT] = INKEY_RIGHTSHIFT;
			m_aCharVal[DIK_MULTIPLY] = INKEY_NUMSTAR;
			m_aCharVal[DIK_LMENU] = INKEY_LEFTALT;
			m_aCharVal[DIK_SPACE] = INKEY_SPACE;
			m_aCharVal[DIK_CAPITAL] = INKEY_CAPSLOCK;
			
			m_aCharVal[DIK_F1] = INKEY_F1;
			m_aCharVal[DIK_F2] = INKEY_F2;
			m_aCharVal[DIK_F3] = INKEY_F3;
			m_aCharVal[DIK_F4] = INKEY_F4;
			m_aCharVal[DIK_F5] = INKEY_F5;
			m_aCharVal[DIK_F6] = INKEY_F6;
			m_aCharVal[DIK_F7] = INKEY_F7;
			m_aCharVal[DIK_F8] = INKEY_F8;
			m_aCharVal[DIK_F9] = INKEY_F9;
			m_aCharVal[DIK_F10] = INKEY_F10;
			
			m_aCharVal[DIK_NUMLOCK] = INKEY_NUMLOCK;
			m_aCharVal[DIK_SCROLL] = INKEY_SCROLLLOCK;
			m_aCharVal[DIK_NUMPAD7] = INKEY_NUM7;
			m_aCharVal[DIK_NUMPAD8] = INKEY_NUM8;
			m_aCharVal[DIK_NUMPAD9] = INKEY_NUM9;
			m_aCharVal[DIK_SUBTRACT] = INKEY_NUMMINUS;
			m_aCharVal[DIK_NUMPAD4] = INKEY_NUM4;
			m_aCharVal[DIK_NUMPAD5] = INKEY_NUM5;
			m_aCharVal[DIK_NUMPAD6] = INKEY_NUM6;
			m_aCharVal[DIK_ADD] = INKEY_NUMPLUS;
			m_aCharVal[DIK_NUMPAD1] = INKEY_NUM1;
			m_aCharVal[DIK_NUMPAD2] = INKEY_NUM2;
			m_aCharVal[DIK_NUMPAD3] = INKEY_NUM3;
			m_aCharVal[DIK_NUMPAD0] = INKEY_NUM0;
			m_aCharVal[DIK_DECIMAL] = INKEY_NUMPERIOD;
			m_aCharVal[DIK_F11] = INKEY_F11;
			m_aCharVal[DIK_F12] = INKEY_F12; 
			//0x58
			//0x9C
			m_aCharVal[DIK_NUMPADENTER] = INKEY_NUMENTER;
			m_aCharVal[DIK_RCONTROL] = INKEY_RIGHTCTRL;
		//	m_aCharVal[DIK_NUMPADCOMMA] = INKEY_NUMCOMMA
			m_aCharVal[DIK_DIVIDE] = INKEY_NUMSLASH;
			m_aCharVal[DIK_SYSRQ] = INKEY_PRINTSCRN;
			m_aCharVal[DIK_RMENU] = INKEY_RIGHTALT;
			m_aCharVal[DIK_HOME] = INKEY_HOME;
			m_aCharVal[DIK_UP] = INKEY_UPARROW;
			m_aCharVal[DIK_PRIOR] = INKEY_PGUP;
			m_aCharVal[DIK_LEFT] = INKEY_LEFTARROW;
			m_aCharVal[DIK_RIGHT] = INKEY_RIGHTARROW;
			m_aCharVal[DIK_END] = INKEY_END;
			m_aCharVal[DIK_DOWN] = INKEY_DOWNARROW;
			m_aCharVal[DIK_NEXT] = INKEY_PGDN;
			m_aCharVal[DIK_INSERT] = INKEY_INS;
			m_aCharVal[DIK_DELETE] = INKEY_DEL;
			m_aCharVal[DIK_LWIN] = 0;
			m_aCharVal[DIK_RWIN] = 0;
			m_aCharVal[DIK_APPS] = 0;
			break;
		}
	}
	m_aCharVal[INKEY_MOUSE1]  = 	INKEY_MOUSE1;
	m_aCharVal[INKEY_MOUSE2]  = 	INKEY_MOUSE2;
	m_aCharVal[INKEY_MOUSE3]  = 	INKEY_MOUSE3;
	m_aCharVal[INKEY_MOUSE4]  = 	INKEY_MOUSE4;
}

//======================================================================================
//======================================================================================

namespace VoidInput
{
/*
=====================================
Win32 Callback keyboard Hook function
=====================================
*/
LRESULT CALLBACK Win32_KeyboardProc(int code,       // hook code
									WPARAM wParam,  // virtual-key code
									LPARAM lParam)   // keystroke-message information
{
	//if(code >= 0)
	if(code == HC_ACTION)
	{
/*		//wParam
		WORD wInfo = (WORD) (lParam >> 16);
		BOOL fRepeat = wInfo & KF_REPEAT;

		// check for ctrl-alt-del
		if (wParam == VK_DELETE && (wInfo & KF_ALTDOWN) &&
			(GetAsyncKeyState(VK_CONTROL) & 0x80000000))
			return CallNextHookEx( hWinKbHook, code, wParam, lParam );

		// check for ctrl-esc
		if (wParam == VK_ESCAPE && (GetAsyncKeyState(VK_CONTROL) & 0x80000000))
			return CallNextHookEx( hWinKbHook, code, wParam, lParam );

		// check for alt-tab
		if (wParam == VK_TAB && (wInfo & KF_ALTDOWN))
			return CallNextHookEx( hWinKbHook, code, wParam, lParam );
*/
		if((wParam >= VK_F1 && wParam <= VK_F12) &&
			((lParam >> 16) & KF_ALTDOWN))
			return CallNextHookEx(m_pKeyboard->hWinKbHook, code, wParam, lParam );

		//Set flags
		int keyindex = wParam;
		if(keyindex == VK_SHIFT)
		{
			if(!(lParam & 0x80000000))
			{
				if(::GetAsyncKeyState(VK_LSHIFT) & 0x80000000)
					keyindex = VK_LSHIFT; 
				else 
					keyindex = VK_RSHIFT; 
			}
		}
		else if(keyindex == VK_MENU)
		{
			if(!(lParam & 0x80000000))
			{
				if(::GetAsyncKeyState(VK_LMENU) & 0x80000000)
					keyindex = VK_LMENU; 
				else 
					keyindex = VK_RMENU; 
			}
		}
		else if(keyindex == VK_CONTROL)
		{
			if(!(lParam & 0x80000000))
			{
				if(::GetAsyncKeyState(VK_LCONTROL) & 0x80000000)
					keyindex = VK_LCONTROL; 
				else 
					keyindex = VK_RCONTROL; 
			}
		}

		if(!(lParam & 0x80000000))
			m_pKeyboard->m_pStateManager->UpdateKey(m_pKeyboard->m_aCharVal[keyindex],BUTTONDOWN);
		else
			m_pKeyboard->m_pStateManager->UpdateKey(m_pKeyboard->m_aCharVal[keyindex],BUTTONUP);
		return 1;
	}
	return CallNextHookEx(m_pKeyboard->hWinKbHook, code, wParam, lParam );
}
}