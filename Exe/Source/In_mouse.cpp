#include "In_mouse.h"

//========================================================================================
//Private Local Variables
//========================================================================================

#define M_DIBUFFER_SIZE		16
#define M_MOUSEBUTTONS		4
#define M_W32MOUSEBUTTONS	3

//Current mouse co-ords
static float	m_curX;
static float	m_curY;
static float	m_curZ;

//Last mouse co-ords
static float	m_lastx;
static float	m_lasty;
static float	m_lastz;

//Center of the screen, Win32 mouse routines need these
static int		m_xcenter;
static int		m_ycenter;

//Console Vars
static CVar	*	m_cxsens=0;
static CVar	*	m_cysens=0;
static CVar	*	m_csens=0;
static CVar	*	m_cinvert=0;
static CVar	*	m_cmode=0;
static CVar *	m_cfilter=0;

//Sensitivities
static float	m_xsens;
static float	m_ysens;
static float	m_sens;

//Other flags
static bool		m_exclusive;
static bool		m_invert;
static bool		m_filter;

//Class Info
static EDeviceState			m_mousestate;
static CMouse::EMouseMode	m_mousemode;

//Direct Input Device
static LPDIRECTINPUTDEVICE7 m_pdimouse=0;	

//Input buffers
static DIMOUSESTATE2	  * m_pdistate=0;	
static DIDEVICEOBJECTDATA	m_dibufdata[M_DIBUFFER_SIZE];
static POINT				m_w32pos;
static short				m_w32buttons[M_W32MOUSEBUTTONS];

//Button States
static KeyEvent_t			m_buttons[M_MOUSEBUTTONS];	//Used to store oldstats of the keys

//Handler Funcs
extern float				g_fRepeatRate;		//Current repeat rate
extern IN_KEYHANDLER		g_pfnKeyHandler;	//Key handler func
extern IN_CURSORHANDLER		g_pfnCursorHandler;	//Cursor Handler


//========================================================================================
//Local function declarations
//========================================================================================

//update Mousestate object with latest info 
static void Update_DIBuffered();
static void Update_DIImmediate();	
static void Update_Win32();	

//Console Variable Validation/Handler funcs
static bool CXSens(const CVar * var, int argc, char** argv);
static bool CYSens(const CVar * var, int argc, char** argv);
static bool CSens(const CVar *var, int argc, char** argv);
static bool CInvert(const CVar *var, int argc, char** argv);
static bool CMouseMode(const CVar *var, int argc, char** argv);
static bool CMouseFilter(const CVar *var, int argc, char** argv);


//========================================================================================
//Implementation
//========================================================================================

/*
=====================================
Constructor
=====================================
*/
CMouse :: CMouse()
{
	m_pdimouse = 0;
	m_mousestate = DEVNONE;
	m_mousemode = M_NONE;

	UpdateMouse = 0;
	m_exclusive = false;
	m_invert = false;
	
	m_pdistate = 0;

	m_curX=0.0f;
	m_curY=0.0f;
	m_curZ=0.0f;

	m_xsens= 0.0f;
	m_ysens= 0.0f;
	m_sens= 0.0f;
	m_lastx = 0.0f;
	m_lasty = 0.0f;
	m_lastz = 0.0f;
	
	m_xcenter = 0;
	m_ycenter = 0;

	g_pCons->RegisterCVar(&m_cxsens,"m_xsens","0.2",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE, &CXSens);
	g_pCons->RegisterCVar(&m_cysens,"m_ysens","0.2",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE, &CYSens);
	g_pCons->RegisterCVar(&m_csens,"m_sens","5.0",CVar::CVAR_FLOAT,CVar::CVAR_ARCHIVE, &CSens);
	g_pCons->RegisterCVar(&m_cinvert,"m_invert","0",CVar::CVAR_BOOL,CVar::CVAR_ARCHIVE, &CInvert);
	g_pCons->RegisterCVar(&m_cmode,"m_mode","1",CVar::CVAR_INT,CVar::CVAR_ARCHIVE, &CMouseMode);
	g_pCons->RegisterCVar(&m_cfilter,"m_filter","0",CVar::CVAR_BOOL, CVar::CVAR_ARCHIVE, &CMouseFilter);
}

/*
=====================================
Destructor
=====================================
*/
CMouse::~CMouse()
{
	m_mousestate = DEVNONE;
	UpdateMouse = 0;

	if(m_pdistate)
		delete m_pdistate;
	m_pdimouse = 0;
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
		m_exclusive = true;

	if(mode == M_NONE)
		mode = m_mousemode;

	//Activate specified mode
	switch(mode)
	{
	case M_WIN32:
		{
			//Don't if already in that mode
			if((m_mousestate != DEVNONE) &&
			   (m_mousemode == M_WIN32))
			{
				ComPrintf("CMouse::InitMode: Already in M_WIN32 mode\n");
				return DI_OK;
			}
			
			//Initialize to the mode
			hr = Win32_Init();
			if(FAILED(hr))
				return hr;

			//Set Update pointer
			UpdateMouse = Update_Win32;
			ComPrintf("CMouse::InitMode:Initialized Win32 mode\n");
			m_mousemode = M_WIN32;
			break;
		}
	case M_DIIMMEDIATE:
		{
			if((m_mousestate != DEVNONE) &&
			   (m_mousemode == M_DIIMMEDIATE))
			{
				ComPrintf("CMouse::InitMode: Already in M_DIIMMEDIATE mode\n");
				return DI_OK;
			}

			hr = DI_Init(M_DIIMMEDIATE);
			if(FAILED(hr))
				return hr;
			UpdateMouse = Update_DIImmediate;
			ComPrintf("CMouse::InitMode:Initialized DI Immediate mode\n");
			m_mousemode = M_DIIMMEDIATE;
			break;
		}
	case M_NONE:
	case M_DIBUFFERED:
		{
			if((m_mousestate != DEVNONE) &&
			   (m_mousemode == M_DIBUFFERED))
			{
				ComPrintf("CMouse::InitMode: Already in M_DIBUFFERED mode\n");
				return DI_OK;
			}
			
			hr = DI_Init(M_DIBUFFERED);
			if(FAILED(hr))
				return hr;
			UpdateMouse = Update_DIBuffered;
			ComPrintf("CMouse::InitMode:Initialized DI Buffered mode\n");
			m_mousemode = M_DIBUFFERED;
			break;
		}
	}

	m_xsens= m_cxsens->value;
	m_ysens= m_cysens->value;
	m_sens= m_csens->value;

	if(m_cfilter->value)
		m_filter = true;
	else
		m_filter = false;
	if(m_cinvert->value)
		m_invert = true;
	else
		m_invert = false;


	//Update state vars
	m_mousestate = DEVINITIALIZED;

	Acquire();

	return DI_OK;
}


/*
=====================================
Shutdown/Release the mouse
=====================================
*/
void CMouse :: Shutdown()
{
	switch(m_mousemode)
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
	m_mousestate = DEVNONE;
	m_mousemode = M_NONE;
}


/*
=====================================
Initialize DI specific stuff
=====================================
*/
HRESULT CMouse::DI_Init(EMouseMode mode)
{
	HRESULT hr;

	//Shutdown the mouse if its active in another mode
	if(m_mousestate != DEVNONE)
		Shutdown();
	
	//Create the Device
	hr = (g_pInput->GetDirectInput())->CreateDeviceEx(GUID_SysMouse, 
									   IID_IDirectInputDevice7,
									   (void**)&m_pdimouse, 
									   NULL); 
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::DI_Init: Couldnt create Device\n");
		return hr;
	}
	
	//Set Data format for mouse
	hr = m_pdimouse->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::Create:Set Data Format failed\n");
		return hr;
	}

	//Set Co-operative mode
	if(m_exclusive)
		hr = m_pdimouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND|DISCL_EXCLUSIVE);
	else
		hr = m_pdimouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
	if (FAILED(hr)) 
	{
		ComPrintf("CMouse::DI_Init:Set Coop Level failed\n");
		return hr;
	}
	
	//Which DI mode to use ?
	if(mode == M_DIIMMEDIATE)
	{
		if(m_pdistate)
			delete m_pdistate;
		m_pdistate= new DIMOUSESTATE2;
	}
	else if(mode == M_DIBUFFERED)
	{
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = M_DIBUFFER_SIZE;

		hr = m_pdimouse->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

		if(FAILED(hr))
		{
			ComPrintf("CMouse::DI_Init:Setting buffer size failed\n");
			return hr;
		}

		if(m_pdistate)
			delete m_pdistate;
		m_pdistate = 0;
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
	if(m_pdimouse)
	{
		if(m_mousestate == DEVACQUIRED)
			m_pdimouse->Unacquire(); 
		m_pdimouse->Release();
		m_pdimouse = NULL;
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
	if(m_mousestate != DEVNONE)
		Shutdown();

	//Lock Cursor to center
	::SetCursorPos(m_xcenter,m_ycenter);

	//Cap the mouse
	::SetCapture(g_hWnd);

	//FIX ME
	//GetSystemMetrics(SM_SWAPBUTTON) 
	//returns TRUE if the mouse buttons have been swapped

	//Set to exclusive
	if(m_exclusive)
		SetExclusive();
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
	if(m_exclusive)
		LoseExclusive();

	//Unacquire it
	if(m_mousestate == DEVACQUIRED)
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
{ return m_mousestate;
}


//========================================================================================
//Mouse Update functions


/*
=====================================
Flush Mouse Data
=====================================
*/
void Flush()
{
	if(m_mousemode == CMouse::M_DIBUFFERED)
	{
		DWORD dwItems = INFINITE; 
		m_pdimouse->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), 
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
void Update_DIBuffered()
{
/*	if(!m_pdimouse)
	{
		g_pInput->InputError(DIERR_NOTINITIALIZED,"CMouse::Update_DIBuffered:");
		return;
	}
*/
	if(m_mousestate != DEVACQUIRED)
		return;

	DWORD dwElements = M_DIBUFFER_SIZE;

	//Get buffered mouse data
	HRESULT hr = m_pdimouse->GetDeviceData(sizeof(DIMOUSESTATE),
											m_dibufdata,
											&dwElements,
											0);
	if(hr != DI_OK)
	{
		//If it failed because of losing input or focus, try once more
		if((hr == DIERR_INPUTLOST)|| (hr ==DIERR_NOTACQUIRED))
		{
			m_mousestate = DEVINITIALIZED;
			hr = g_pMouse->Acquire();
			if(!FAILED(hr))
			{
				hr = m_pdimouse->GetDeviceData(sizeof(DIMOUSESTATE),
												m_dibufdata,
												&dwElements,
												0);
				//Failed yet again, throw error
				if(FAILED(hr))
				{
					ComPrintf("CMouse::GetState:Error getting state2\n");
					g_pInput->InputError(hr,"CMouse::Update_DIBuffered:");
					return;
				}
			}
		}
		//Buffered Overflowed, just flush and return
		else if(hr == DI_BUFFEROVERFLOW)
		{
			Flush();
			return;
		}
	}

	//Reset events
	m_curX = m_curY = m_curZ = 0;

	//Got data, loop through buffers to get events
	for(unsigned int i=0;i<dwElements;i++)
	{
		switch((int)m_dibufdata[i].dwOfs)
		{
		case DIMOFS_BUTTON0:
			{
				m_buttons[0].id = INKEY_MOUSE1;
				if(m_dibufdata[i].dwData & 0x80)
					m_buttons[0].state = BUTTONDOWN;
				else
					m_buttons[0].state = BUTTONUP;
				break;
			}
		case DIMOFS_BUTTON1:
			{
				m_buttons[1].id = INKEY_MOUSE2;
				if(m_dibufdata[i].dwData & 0x80)
					m_buttons[1].state = BUTTONDOWN;
				else
					m_buttons[1].state = BUTTONDOWN;
				break;
			}
		case DIMOFS_BUTTON2:
			{
				m_buttons[0].id = INKEY_MOUSE3;
				if(m_dibufdata[i].dwData & 0x80)
					m_buttons[0].state = BUTTONDOWN;
				else
					m_buttons[0].state = BUTTONUP;
				break;
			}
		case DIMOFS_BUTTON3:
			{
				m_buttons[1].id = INKEY_MOUSE4;
				if(m_dibufdata[i].dwData & 0x80)
					m_buttons[1].state = BUTTONDOWN;
				else
					m_buttons[1].state = BUTTONDOWN;
				break;
			}
		case DIMOFS_X:
			m_curX = ((int)m_dibufdata[i].dwData) * m_xsens * m_sens;
			break;
		case DIMOFS_Y:
			m_curY = ((int)m_dibufdata[i].dwData) * m_ysens * m_sens;
			break;
		case DIMOFS_Z:
			m_curZ = ((int)m_dibufdata[i].dwData) * m_sens;
			break;
		}
	}


	//Dispatch all the mouse button events,
	//Button UPs are reset to 0
	for(i=0;i<M_MOUSEBUTTONS;i++)
	{
		if(m_buttons[i].id)
		{
			if(m_buttons[i].state != BUTTONHELD)
				m_buttons[i].time = g_fcurTime;

			g_pfnKeyHandler(&m_buttons[i]);

			if(m_buttons[i].state == BUTTONUP)
				m_buttons[i].id = 0;
			else
				m_buttons[i].state = BUTTONHELD;
		}
	}

	
	//Apply filter if needed
	if(m_filter)
	{
		m_lastx = m_curX = (m_curX + m_lastx)/2;
		m_lasty = m_curY = (m_curY + m_lasty)/2;
		m_lastz = m_curZ = (m_curZ + m_lastz)/2;
	}
	
	//Invery y-axis if needed
	if(m_invert)
		m_curY = -(m_curY);

	g_pfnCursorHandler(m_curX,m_curY,m_curZ);
}


/*
=====================================
DirectInput Immediate data
=====================================
*/
void Update_DIImmediate()
{
/*	if(!m_pdimouse)
	{
		g_pInput->InputError(DIERR_NOTINITIALIZED,"CMouse::Update_DIBuffered:");
		return;
	}
*/
	if(m_mousestate != DEVACQUIRED)
		return;

	//Get Mouse State
	HRESULT hr = m_pdimouse->GetDeviceState(sizeof(DIMOUSESTATE2), 
										  m_pdistate);
	if(hr != DI_OK)
	{
		//Unknown error
		if((hr != DIERR_INPUTLOST)&& (hr !=DIERR_NOTACQUIRED))
		{
			g_pInput->InputError(hr,"CMouse::Update_DII:");
			return;
		}

		//Try to regain focus
		m_mousestate = DEVINITIALIZED;
		hr = g_pMouse->Acquire();
		if(!FAILED(hr))
		{
			hr = m_pdimouse->GetDeviceState(sizeof(DIMOUSESTATE), m_pdistate);
			
			//Failed again, error
			if(hr != DI_OK)
			{	
				ComPrintf("CMouse::GetState:Error getting state2\n");
				return;
			}
		}
	}

	//Update Buttons
	for(int i=0;i<M_MOUSEBUTTONS;i++)
	{
		//Button is down
		if(m_pdistate->rgbButtons[i] & 0x80)
		{
			m_buttons[i].id = INKEY_MOUSE1 + i;

			//Was it down before ?
			if(m_buttons[i].state != BUTTONUP)
			{
				//Button has been held
				m_buttons[i].state = BUTTONHELD;
				//no change in time
			}
			else
			{
				//Button just went down
				m_buttons[i].state = BUTTONDOWN;
				m_buttons[i].time = g_fcurTime;
			}
			g_pfnKeyHandler(&m_buttons[i]);
		}
		else //Button is up
		{
			 //Was it down before ?
			if(m_buttons[i].state != BUTTONUP)
			{
				m_buttons[i].state = BUTTONUP;
				m_buttons[i].time = g_fcurTime;

				g_pfnKeyHandler(&m_buttons[i]);
			}
		}
	}


	//Average out values if filtering is on
	if(m_filter)
	{
		m_curX = m_lastx = ((m_pdistate->lX * m_xsens * m_sens)+ m_lastx)/2;
		m_curY = m_lasty = ((m_pdistate->lY * m_ysens * m_sens) + m_lasty)/2;
		m_curZ = m_lastz = ((m_pdistate->lZ * m_sens) + m_lastz)/2;

	}
	else
	{
		m_curX = m_pdistate->lX * m_xsens * m_sens;
		m_curY = m_pdistate->lY * m_ysens * m_sens;
		m_curZ = m_pdistate->lZ * m_sens;
	}
	
	//Inverse Y-Axis if mouse is inverted
	if(m_invert)
		m_curY = -(m_curY);

	g_pfnCursorHandler(m_curX,m_curY,m_curZ);
}



/*
=====================================
Win32 Mouse update
=====================================
*/
void Update_Win32()
{
	//Get current cursor pos
	::GetCursorPos(&m_w32pos);

	//Calc offsets
	m_curX = (m_w32pos.x - m_xcenter) * m_xsens * m_sens;;
	m_curY = (m_ycenter - m_w32pos.y) * m_ysens * m_sens;

	if(m_filter)
	{
		m_lastx = m_curX = (m_curX + m_lastx)/2;
		m_lasty = m_curY = (m_curY + m_lasty)/2;
	}
	if(m_invert)
		m_curY = -(m_curY);

	/*	
	If the most significant bit is set, the key is down, and if the least significant bit is set, 
	the key was pressed after the previous call to GetAsyncKeyState
	*/

	m_w32buttons[0] = ::GetKeyState(VK_LBUTTON);
	m_w32buttons[1] = ::GetKeyState(VK_RBUTTON);
	m_w32buttons[2] = ::GetKeyState(VK_MBUTTON);

	for(int i=0;i<M_W32MOUSEBUTTONS;i++)
	{
		//Button is down
		if(m_w32buttons[i] & 0x80)
		{
			m_buttons[i].id = INKEY_MOUSE1 + i;

			//Was it down before ?
			if(m_buttons[i].state != BUTTONUP)
			{
				//Button has been held
				m_buttons[i].state = BUTTONHELD;
				//no change in time
			}
			else
			{
				//Button just went down
				m_buttons[i].state = BUTTONDOWN;
				m_buttons[i].time = g_fcurTime;
			}
			g_pfnKeyHandler(&m_buttons[i]);
		}
		else //Button is up
		{
			 //Was it down before ?
			if(m_buttons[i].state != BUTTONUP)
			{
				m_buttons[i].state = BUTTONUP;
				m_buttons[i].time = g_fcurTime;

				g_pfnKeyHandler(&m_buttons[i]);
			}
		}
	}
	
	//Lock cursor to center of screen
	::SetCursorPos(m_xcenter,m_ycenter);

	g_pfnCursorHandler(m_curX,m_curY,m_curZ);
}


//========================================================================================
//Misc Mouse Funcsions
//========================================================================================

/*
=====================================
Swtich to Exclusive mode
=====================================
*/
HRESULT CMouse::SetExclusive()
{
	if(!m_exclusive)
	{
		//Try changing to DI Exclusive mode is using DirectInput
		if(m_pdimouse && 
		  ((m_mousemode == M_DIIMMEDIATE) ||
		   (m_mousemode == M_DIBUFFERED)))
		{
			HRESULT hr = m_pdimouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND|DISCL_EXCLUSIVE);
			if(FAILED(hr))
				return hr;
		}
		m_exclusive = true;
	}
	return DI_OK;
}


/*
=====================================
Switch to non-exclusive mode
=====================================
*/
HRESULT CMouse::LoseExclusive()
{
	if(m_exclusive)
	{
		//Try changing to DI NONExclusive mode is using DirectInput
		if(m_pdimouse && 
		  ((m_mousemode == M_DIIMMEDIATE) ||
		   (m_mousemode == M_DIBUFFERED)))
		{
			HRESULT hr = m_pdimouse->SetCooperativeLevel(g_hWnd, DISCL_FOREGROUND|DISCL_NONEXCLUSIVE);
			if(FAILED(hr))
				return hr;
		}
		m_exclusive = false;
	}
	return DI_OK;
}

/*
===========================================
Main Window resized,
updated center coords
===========================================
*/
void CMouse::Resize()
{
	m_xcenter = (g_hRect.left + g_hRect.right)/2;
	m_ycenter = (g_hRect.top + g_hRect.bottom)/2;
}


/*
=====================================
Acquire the mouse
=====================================
*/
HRESULT CMouse :: Acquire()
{
	//Already acquired
	if(m_mousestate == DEVACQUIRED)
		return S_OK;
		
	if((m_mousemode == M_WIN32) &&  
	   (m_mousestate==DEVINITIALIZED))
	{	
		::ShowCursor(false);				//Make cursor disappear for Win32 mode	
		m_mousestate = DEVACQUIRED;
		return S_OK;
	}
	else if(m_pdimouse)
	{
		HRESULT hr;
		hr = m_pdimouse->Acquire();
		if(hr == DI_OK)
		{	
			ComPrintf("CMouse::Acquire OK\n");
			m_mousestate = DEVACQUIRED;
			return DI_OK;
		}

		//try again for a while
/*		for(int i=0;i<50;i++)
		{
			hr = m_pdimouse->Acquire();
			if(hr == DI_OK)
			{
				ComPrintf("CMouse::Acquire OK\n");
				m_mousestate = DEVACQUIRED;
				return DI_OK;
			}
		}
*/
		g_pInput->InputError(hr,"CMouse::Acquire:Unable to acquire\n");
		m_mousestate = DEVINITIALIZED;
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
	if(m_mousestate != DEVACQUIRED)
		return true;

	if(m_mousemode == M_WIN32)
	{
		::ShowCursor(true);					//Show cursor now	
		::ClipCursor(0);					//Get rid of any clipping
		m_mousestate = DEVINITIALIZED;
		return true;
	}
	else if(m_pdimouse)
	{
		m_pdimouse->Unacquire();
		m_mousestate = DEVINITIALIZED;
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
Win32 Hooks
=====================================
*/
bool CMouseMode(const CVar * var, int argc,char** argv)
{
	if(argc >= 2 && argv[1])
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			//Allow configs to change the mousemode if its valid
			//even before the mouse actually inits
			if( m_mousestate == DEVNONE  &&
				(temp == CMouse::M_DIBUFFERED ||
			    temp == CMouse::M_DIIMMEDIATE ||
			    temp == CMouse::M_WIN32))
			{
				m_mousemode = (CMouse::EMouseMode)temp;
			   return true;
			}

			HRESULT hr = E_FAIL;

			switch((CMouse::EMouseMode)temp)
			{
			case CMouse::M_DIIMMEDIATE:
				hr = g_pMouse->Init(m_exclusive,CMouse::M_DIIMMEDIATE);
				break;
			case CMouse::M_DIBUFFERED:
				hr = g_pMouse->Init(m_exclusive,CMouse::M_DIBUFFERED);
				break;
			case CMouse::M_WIN32:
				hr =g_pMouse->Init(m_exclusive,CMouse::M_WIN32);
				break;
			}
			
			if(FAILED(hr))
			{
				ComPrintf("CMouse:CMouseMode: Couldnt change to mode %d\n",temp);
				return false;
			}

			return true;
		}
		ComPrintf("CMouse::CMouseMoude:couldnt read required mode (valid 1-3)\n");
	}

	//Show current info
	ComPrintf("MouseMode is %d\n",m_mousemode);

	switch(m_mousemode)
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
bool CXSens(const CVar * var, int argc,char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_xsens = temp;
				return true;
			}
			ComPrintf("CMouse::CXSens:Invalid Value entered");
		}
	}
	ComPrintf("X-axis Sensitivity: %.2f\n", m_xsens);
	return false;
}

/*
======================================
Console Func - Sets Y-axis Sensitivity
======================================
*/
bool CYSens(const CVar * var, int argc,char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_ysens = temp;
				return true;
			}
			ComPrintf("CMouse::CYSens:Invalid Value entered");
		}
	}
	ComPrintf("Y-axis Sensitivity: %.2f\n", m_ysens);
	return false;
}


/*
======================================
Console Func - Sets master Sensitivity
======================================
*/
bool CSens(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0.0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp > 0.0 && temp < 30.0)
			{
				m_sens = temp;
				return true;
			}
			ComPrintf("CMouse::CSens:Invalid Value entered");
		}
	}
	ComPrintf("Mouse Sensitivity: %.2f\n", m_sens);
	return false;

}


/*
======================================
Console Func - Sets Mouse Inverted states
======================================
*/
bool CInvert(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if(temp)
				m_invert = true;
			else 
				m_invert = false;
			return true;
		}
	}
	if(m_invert)
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
bool CMouseFilter(const CVar * var, int argc, char** argv)
{
	if(argc >= 2 && argv[1])
	{
		int temp = 0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			if(temp)
				m_filter = true;
			else 
				m_filter = false;
			return true;
		}
	}
	if(m_filter)
		ComPrintf("CInput::Mouse filter on\n");
	else
		ComPrintf("CInput::Mouse filter off\n");
	return false;
}