#include "In_kb.h"

/*
========================================================================================
Additional Virtual key constants, sort of an hack. not sure if they will work
on all keyboards
========================================================================================
*/
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

#define KB_MAXEVENTS    32
#define KB_DIBUFFERSIZE	16
#define KB_TOTALCHARS	256

/*
========================================================================================
Private Local Vars
========================================================================================
*/

//Mouse Handler needs access to these for mouse Buttons
extern float				g_fRepeatRate;					//Current repeat rate
extern IN_KEYHANDLER		g_pfnKeyHandler;				//Key handler func


//The Device Object
static LPDIRECTINPUTDEVICE7	m_pdikb=0;

//Device Query Buffers
static DIDEVICEOBJECTDATA	m_dikeydata[KB_DIBUFFERSIZE];	//Receives DI buffered data 
static BYTE					m_keydata[KB_TOTALCHARS];		//Receives immediate and Win32 data

//State information
static EDeviceState			m_kbstate;
static CKeyboard::EKbMode	m_mode;
static bool					m_exclusive;

//Translation table
static unsigned int			m_chartable[KB_TOTALCHARS];		

static HKL					m_hkl;

static KeyEvent_t			m_keyEvent;						//the key event object which gets dispatched.
static KeyEvent_t			m_oldkeys[KB_TOTALCHARS];		//Used to store oldstats of the keys

static CVar		*			m_kbmode=0;

//========================================================================================

//Keyboard update funcs
static void		Update_DIBuffered();
static void		Update_DIImmediate();
static void		Update_Win32();			

//Cvar Handler
static bool		CKBMode(const CVar * var, int argc, char** argv);

//Misc
static void		FlushDIBuffer();

//Initializes the Translation table for the given mode
static void		SetCharTable(CKeyboard::EKbMode mode);
static void		ShiftCharacter(int &val);


//========================================================================================
//========================================================================================

/*
=====================================
Constructor
=====================================
*/
CKeyboard::CKeyboard()
{
	m_pdikb = 0;
	m_kbstate = DEVNONE;

	m_exclusive = false;
	m_mode = KB_NONE;

	memset(m_chartable, 0, KB_TOTALCHARS*sizeof(unsigned int));
	memset(m_oldkeys,0, KB_TOTALCHARS * sizeof(KeyEvent_t));
	memset(m_keydata, 0, KB_TOTALCHARS*sizeof(BYTE));
	memset(m_dikeydata,0,KB_DIBUFFERSIZE*sizeof(DIDEVICEOBJECTDATA));

	UpdateKeys = 0; 

	g_pCons->RegisterCVar(&m_kbmode,"kb_mode","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE, &CKBMode);

	m_hkl =  GetKeyboardLayout(0);
}

/*
=====================================
Destructor
=====================================
*/
CKeyboard::~CKeyboard()
{
	UpdateKeys = 0;
	m_pdikb = 0;
}

/*
=====================================
Shutdown the keyboard interface
=====================================
*/
void CKeyboard :: Shutdown()
{
	FlushDIBuffer();

	switch(m_mode)
	{
	case KB_WIN32:
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
	m_kbstate = DEVNONE;
	m_mode = KB_NONE;
	ComPrintf("CKeyboard::Shutdown:OK\n");
}

/*
===========================================
Initialize the keyboard to the given mode
===========================================
*/
HRESULT	CKeyboard::Init(int exclusive,EKbMode mode)
{
	if(exclusive)
		m_exclusive = true;

	if(mode == KB_NONE)
		mode = m_mode;

	HRESULT hr;
	switch(mode)
	{
	case KB_WIN32:
		{
			if((m_mode == KB_WIN32) &&
			   (m_kbstate != DEVNONE))
			{
				ComPrintf("CKeyboard::InitMode: Already in Win32 mode\n");
				return DI_OK;
			}
			
			//Initialize to the mode
			hr = Win32_Init();
			if(FAILED(hr))
				return hr;

			//Set Update pointer
			UpdateKeys = Update_Win32;
			ComPrintf("CKeyboard::InitMode:Initialized Win32 mode\n");
			m_mode = KB_WIN32;
			
			SetCharTable(KB_WIN32);
			break;
		}
	case KB_DIBUFFERED:
		{
			if((m_mode == KB_DIBUFFERED) &&
			   (m_kbstate != DEVNONE))
			{
				ComPrintf("CKeyboard::InitMode: Already in DI Buffered mode\n");
				return DI_OK;
			}
			
			//Initialize to the mode
			hr = DI_Init(KB_DIBUFFERED);
			if(FAILED(hr))
				return hr;

			//Set Update pointer
			UpdateKeys = Update_DIBuffered;
			ComPrintf("CKeyboard::InitMode:Initialized DI Buffered mode\n");
			m_mode = KB_DIBUFFERED;

			SetCharTable(KB_DIBUFFERED);
			break;
		}
	default:	
	case KB_DIIMMEDIATE:
		{
			if((m_mode == KB_DIIMMEDIATE) &&
			   (m_kbstate != DEVNONE))
			{
				ComPrintf("CKeyboard::InitMode: Already in DI Immediate mode\n");
				return DI_OK;
			}
			//Initialize to the mode
			hr = DI_Init(KB_DIIMMEDIATE);
			if(FAILED(hr))
				return hr;
			UpdateKeys = Update_DIImmediate;
			ComPrintf("CKeyboard::InitMode:Intialized to DI Immediate mode\n");
			m_mode = KB_DIIMMEDIATE;

			SetCharTable(KB_DIIMMEDIATE);
			break;
		}
	}

	m_kbstate = DEVINITIALIZED;
	//Try to acquire the kb now
	Acquire();
	return DI_OK;
}

/*
=====================================
Direct Input init
=====================================
*/
HRESULT CKeyboard::DI_Init(EKbMode mode)
{
	HRESULT  hr;

	if(m_kbstate != DEVNONE)
		Shutdown();

	//Create the device
	hr = (g_pInput->GetDirectInput())->CreateDeviceEx( GUID_SysKeyboard, 
										IID_IDirectInputDevice7, 
										(void**)&m_pdikb, 
										NULL); 
	//Failed
	if (FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init:Keyboard Initialization failed\n");
		return hr;
	}
	
	//Set data format
	hr = m_pdikb->SetDataFormat(&c_dfDIKeyboard); 
	if (FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init :Data Format failed\n");
		return hr;
	}

	
	//Set coop level depending on exclusive flag
	if(!m_exclusive)
		hr = m_pdikb->SetCooperativeLevel(g_hWnd,DISCL_FOREGROUND | DISCL_NONEXCLUSIVE); 
	else
		hr = m_pdikb->SetCooperativeLevel(g_hWnd,DISCL_FOREGROUND | DISCL_EXCLUSIVE); 
	
	if (FAILED(hr)) 
	{
		ComPrintf("CKeyboard::DI_Init :Coop Level failed\n");
		return hr;
	}

	if(mode == KB_DIBUFFERED)
	{
		//Setup buffers
		DIPROPDWORD dipdw;

		dipdw.diph.dwSize = sizeof(DIPROPDWORD);
		dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		dipdw.diph.dwObj = 0;
		dipdw.diph.dwHow = DIPH_DEVICE;
		dipdw.dwData = KB_DIBUFFERSIZE;
	
		hr = m_pdikb->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph );
		if(FAILED(hr))
		{
			ComPrintf("CKeyboard::Init :Set Buffer failed\n");
			return hr;
		}
	}
	return DI_OK;
}

/*
=====================================
Direct Input shutdown
=====================================
*/
bool CKeyboard::DI_Shutdown()
{
	if(m_pdikb)
	{
		if(m_kbstate == DEVACQUIRED)
			m_pdikb->Unacquire(); 
		m_pdikb->Release();
		m_pdikb = NULL;
	}
	return true;
}

/*
===========================================
Win32 Initialize
===========================================
*/
HRESULT	CKeyboard::Win32_Init()
{
	if(m_kbstate != DEVNONE)
		Shutdown();
	return DI_OK;
}

/*
===========================================
Win32 Shutdown
===========================================
*/
bool CKeyboard::Win32_Shutdown()
{	return true;
}


/*
===========================================
Acquire the keyboard
===========================================
*/
HRESULT CKeyboard :: Acquire()
{
	if(m_kbstate == DEVACQUIRED)
		return S_OK;

	if(m_mode == KB_WIN32)
		return S_OK;

	if((m_mode == KB_DIBUFFERED || m_mode == KB_DIIMMEDIATE) 
		&& m_pdikb)
	{
		HRESULT hr = m_pdikb->Acquire();

		if(FAILED(hr))
		{
/*			//try again for a while
			for(int i=0;i<50,hr!=DI_OK;i++)
				hr = m_pdikb->Acquire();
*/
			ComPrintf("CKeyboard::Unable to acquire the keyboard\n");
			m_kbstate = DEVINITIALIZED;
			return hr;
		}
		ComPrintf("CKeyboard::Acquire :OK\n");
		m_kbstate = DEVACQUIRED;
		return DI_OK;
	}

	ComPrintf("CKeyboard::Acquire failed. DI is not intialized\n");
	return DIERR_NOTINITIALIZED;
}

/*
===========================================
Unacquire the keyboard
===========================================
*/
bool CKeyboard::UnAcquire()
{
	if((m_mode == KB_DIBUFFERED || 
		m_mode == KB_DIIMMEDIATE) &&
	    m_kbstate == DEVACQUIRED)
	{
		m_pdikb->Unacquire();
		m_kbstate = DEVINITIALIZED;
		ComPrintf("CKeyboard::UnAcquire :OK\n");
	}
	
	//Need this ?
	FlushDIBuffer();
	return true;
}

/*
===========================================
Returns current State
===========================================
*/
int	 CKeyboard::GetDeviceState() 
{ return m_kbstate; 
}



//========================================================================================
//Polling methods

/*
===========================================
Flushes current data
===========================================
*/
void FlushDIBuffer()
{
	memset(m_oldkeys,0,sizeof(KeyEvent_t) *  KB_TOTALCHARS);

	if((m_mode == CKeyboard::KB_DIBUFFERED || m_mode == CKeyboard::KB_DIIMMEDIATE) 
		&& m_pdikb)
	{
		DWORD num= INFINITE;
		m_pdikb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),NULL,&num,0);
	}
}

/*
===========================================
Query DirectInput Bufferedmode
===========================================
*/
void Update_DIBuffered() 
{
	if(m_kbstate != DEVACQUIRED)
		return;

//	if (m_pdikb != NULL)
//    {
	HRESULT hr;
    DWORD numitems = KB_DIBUFFERSIZE;	// number of elements
    unsigned int i=0;					// misc counters

	//query for new info
	hr = m_pdikb->GetDeviceData(sizeof(DIDEVICEOBJECTDATA),
                              m_dikeydata,
							  &numitems,
							  0);
	//uh-oh
	if (hr != DI_OK)
	{
		//we've lost contact with the keyboard try to reacquire
		if (hr==DIERR_NOTACQUIRED)
		{
			ComPrintf("CKeyboard::UpdateDI:Failed, Keyboard is not acquired\n");

			//try to acquire it and then return
			m_kbstate = DEVINITIALIZED;
			hr = g_pKb->Acquire();
			
			//Unable to reaquire it
			if(!FAILED(hr))
				FlushDIBuffer();

		}
		
		//Send key up events for all current keys, to reset them
		for(i=0;i<KB_TOTALCHARS;i++)
		{
			m_keyEvent.flags = 0;

			if(m_oldkeys[i].state != BUTTONUP)
			{
				m_oldkeys[i].id = 0;
				
				m_keyEvent.time = g_fcurTime;
				m_keyEvent.state = BUTTONUP;
				m_keyEvent.id = m_chartable[i];

				g_pfnKeyHandler(&m_keyEvent);
			}
		}
		return;
	}

	m_keyEvent.flags = 0;

	//Uglyness
	if((m_oldkeys[DIK_RSHIFT].state == BUTTONHELD)  ||(m_oldkeys[DIK_LSHIFT].state == BUTTONHELD))
		m_keyEvent.flags |= SHIFTDOWN;

	if((m_oldkeys[DIK_RMENU].state == BUTTONHELD)   ||(m_oldkeys[DIK_LMENU].state == BUTTONHELD))
		m_keyEvent.flags |= ALTDOWN;

	if((m_oldkeys[DIK_RCONTROL].state== BUTTONHELD)|| (m_oldkeys[DIK_LCONTROL].state == BUTTONHELD))
		m_keyEvent.flags |= CTRLDOWN;
	

	//These are the 16 or less items that changed
	//Add the new items to kbstate
	for(i=0;i<numitems;i++)
	{
		m_keyEvent.id   = m_oldkeys[m_dikeydata[i].dwOfs].id = m_chartable[(m_dikeydata[i].dwOfs)];
		m_keyEvent.time = g_fcurTime;

		//Button is down
		if(m_dikeydata[i].dwData & 0x80)
		{
			//If it wasnt held before, then it just went down
			if(m_oldkeys[m_dikeydata[i].dwOfs].state != BUTTONHELD)
			{
				m_oldkeys[m_dikeydata[i].dwOfs].state = BUTTONDOWN;
				m_oldkeys[m_dikeydata[i].dwOfs].time = g_fcurTime;
			}
			m_keyEvent.state = BUTTONDOWN;
		}
		else
		{
			//just went up
			m_oldkeys[m_dikeydata[i].dwOfs].state = BUTTONUP;
			m_keyEvent.state = BUTTONUP;
		}
		
		//Apply flag
		if(m_keyEvent.flags & SHIFTDOWN)
			ShiftCharacter(m_keyEvent.id);

		//Dispatch event
		g_pfnKeyHandler(&m_keyEvent);
	}

	for(i=0;i<KB_TOTALCHARS;i++)
	{
		//Add all the items that are still down, as BUTTONHELD items
		if(m_oldkeys[i].state == BUTTONHELD)
		{
			m_keyEvent.id = m_chartable[i]; //m_oldkeys[i].id
			m_keyEvent.time = m_oldkeys[i].time + g_fRepeatRate;
			m_keyEvent.state = BUTTONHELD;

			if(m_keyEvent.flags & SHIFTDOWN)
				ShiftCharacter(m_keyEvent.id);

			g_pfnKeyHandler(&m_keyEvent);
		}
		//Reset all the BUTTONDOWN events to BUTTONHELD
		//So that if there was a buttonup event, they will be set to that
		//otherwise we can assume that they are held down the next round
		else if(m_oldkeys[i].state == BUTTONDOWN)
		{	m_oldkeys[i].state = BUTTONHELD;
		}
	}
//		return;
//    }
//	ComPrintf("CKeyboard :: GetState:kbd not initialized\n");
//	throw DIERR_NOTINITIALIZED;
}

/*
=====================================
DirectInput Immediate mode
=====================================
*/
void Update_DIImmediate()
{
	if(m_kbstate != DEVACQUIRED)
		return;

    HRESULT hr;
	int i=0;
	
	//Get the input's device state, and put the state in dims
    hr = m_pdikb->GetDeviceState(sizeof(m_keydata),
							   &m_keydata);
    if(FAILED(hr))  
	{
		//we've lost contact with the keyboard try to reacquire
		if (hr==DIERR_NOTACQUIRED)
		{
			ComPrintf("CKeyboard::UpdateDI:Failed, Keyboard is not acquired\n");

			//try to acquire it and then return
			m_kbstate = DEVINITIALIZED;
			hr = g_pKb->Acquire();
			
			//Unable to reaquire it
			if(!FAILED(hr))
				FlushDIBuffer();

		}
		else
			ComPrintf("CKeyboard::Update_DIIImmediate: Unable to get device state\n");

		//Send key up events for all current keys, to reset them
		for(i=0;i<KB_TOTALCHARS;i++)
		{
			m_keyEvent.flags = 0;

			if(m_oldkeys[i].id)
			{
				m_oldkeys[i].id = 0;
				
				m_keyEvent.time = g_fcurTime;
				m_keyEvent.state = BUTTONUP;
				m_keyEvent.id = m_chartable[i];
	
				g_pfnKeyHandler(&m_keyEvent);

			}
		}

		//Unable to reaquire it
		if(!FAILED(hr))
				FlushDIBuffer();
		return;
	}

	m_keyEvent.flags =0;

	if((m_keydata[DIK_RSHIFT] & 0x80) ||
	   (m_keydata[DIK_LSHIFT] & 0x80))
		m_keyEvent.flags |= SHIFTDOWN;

	if((m_keydata[DIK_RMENU] & 0x80) ||
	   (m_keydata[DIK_LMENU] & 0x80))
		m_keyEvent.flags |= ALTDOWN;

	if((m_keydata[DIK_RCONTROL] & 0x80) ||
		(m_keydata[DIK_LCONTROL] & 0x80))
		m_keyEvent.flags |= CTRLDOWN;

	for(i=0;i<KB_TOTALCHARS;i++)
	{
		//Key is down
		if(m_keydata[i] & 0x80)
		{
			//Check if it was down the last frame
			if(m_oldkeys[i].id)
			{
				//dont make any changes to the oldkey, but new key is now HELD
				m_keyEvent.time = m_oldkeys[i].time + g_fRepeatRate;
				m_keyEvent.state = BUTTONHELD;
			}
			//Key just went down
			else
			{
				m_keyEvent.time = m_oldkeys[i].time = g_fcurTime;
				m_keyEvent.state= m_oldkeys[i].state = BUTTONDOWN;
				m_oldkeys[i].id = m_chartable[i];

			}
			m_keyEvent.id = m_chartable[i];

			//Dispatch event
			if(m_keyEvent.flags & SHIFTDOWN)
				ShiftCharacter(m_keyEvent.id);
			g_pfnKeyHandler(&m_keyEvent);

		}
		//Was Key was down before, up now
		else if(m_oldkeys[i].id)
		{
			//Reset old keystate
			m_oldkeys[i].id = 0;
			m_oldkeys[i].time = 0.0f;
			m_oldkeys[i].state = BUTTONUP;

			//Send new Keystate
			m_keyEvent.id = m_chartable[i];
			m_keyEvent.time = g_fcurTime;
			m_keyEvent.state = BUTTONUP;

			//Dispatch event
			if(m_keyEvent.flags & SHIFTDOWN)
				ShiftCharacter(m_keyEvent.id);

			g_pfnKeyHandler(&m_keyEvent);
		}
	}
}


/*
=====================================
Update keyboard using standard win32 calls
=====================================
*/
void Update_Win32()
{
	if(m_kbstate != DEVACQUIRED)
		return;

	int i=0;
	
	//Get the input's device state, and put the state in dims
    if(!GetKeyboardState(m_keydata))
	{
		ComPrintf("CKeyboard::Update_Win32: Unable to get device state\n");

		//Send key up events for all current keys, to reset them
		for(i=0;i<KB_TOTALCHARS;i++)
		{
			m_keyEvent.flags = 0;

			if(m_oldkeys[i].id)
			{
				m_oldkeys[i].id = 0;

				m_keyEvent.time = g_fcurTime;
				m_keyEvent.state = BUTTONUP;
				m_keyEvent.id = m_chartable[i];
				
				g_pfnKeyHandler(&m_keyEvent);
			}
		}
		return;
	}

	m_keyEvent.flags =0;

	if(m_keydata[VK_SHIFT] & 0x80)
	{
		if(::GetKeyState(VK_LSHIFT) & 0x80)
			m_keydata[VK_LSHIFT] = 0x80;
		else 
			m_keydata[VK_RSHIFT] = 0x80;
 		m_keyEvent.flags |= SHIFTDOWN;
	}

	if(m_keydata[VK_MENU] & 0x80)
	{
		if(::GetKeyState(VK_LMENU) & 0x80)
			m_keydata[VK_LMENU] = 0x80;
		else 
			m_keydata[VK_RMENU] = 0x80;
		m_keyEvent.flags |= ALTDOWN;
	}

	if(m_keydata[VK_CONTROL] & 0x80)
	{
		if(::GetKeyState(VK_LCONTROL) & 0x80)
			m_keydata[VK_LCONTROL] = 0x80;
		else 
			m_keydata[VK_RCONTROL] = 0x80;
		m_keyEvent.flags |= CTRLDOWN;
	}

	for(i=0;i<KB_TOTALCHARS;i++)
	{
		//Key is down
		if(m_keydata[i] & 0x80)
		{
			//Check if it was down the last frame
			if(m_oldkeys[i].id)
			{
				//dont make any changes to the oldkey, but new key is now HELD
				m_keyEvent.time = m_oldkeys[i].time + g_fRepeatRate;
				m_keyEvent.state = BUTTONHELD;
			}
			//Key just went down
			else
			{
				m_keyEvent.time = m_oldkeys[i].time = g_fcurTime;
				m_keyEvent.state= m_oldkeys[i].state = BUTTONDOWN;
				m_oldkeys[i].id = m_chartable[i];

			}
			m_keyEvent.id = m_chartable[i];

			//Dispatch event
			if(m_keyEvent.flags & SHIFTDOWN)
				ShiftCharacter(m_keyEvent.id);
			g_pfnKeyHandler(&m_keyEvent);

		}
		//Was Key was down before, up now
		else if(m_oldkeys[i].id)
		{
			//Reset old keystate
			m_oldkeys[i].id = 0;
			m_oldkeys[i].time = 0.0f;
			m_oldkeys[i].state = BUTTONUP;

			//Send new Keystate
			m_keyEvent.id = m_chartable[i];
			m_keyEvent.time = g_fcurTime;
			m_keyEvent.state = BUTTONUP;

			//Dispatch event
			if(m_keyEvent.flags & SHIFTDOWN)
				ShiftCharacter(m_keyEvent.id);

			g_pfnKeyHandler(&m_keyEvent);
		}
	}
} 
 

/*
=====================================
changed val to its "shifted" equivalent
=====================================
*/
void ShiftCharacter(int &val)
{	
	if(val >= 'a' && val <= 'z')
		val = val - 'a' + 'A';
	else 
	{
		switch(val)
		{
			case '0':
				val = ')';
				break;
			case '1':
				val = '!';
				break;
			case '2':
				val = '@';
				break;
			case '3':
				val = '#';
				break;
			case '4':
				val = '$';
				break;
			case '5':
				val = '%';
				break;
			case '6':
				val = '^';
				break;
			case '7':
				val = '&';
				break;
			case '8':
				val = '*';
				break;
			case '9':
				val = '(';
				break;
			case '-':
				val = '_';
				break;
			case '=':
				val = '+';
				break;
			case '[':
				val = '{';
				break;
			case ']':
				val = '}';
				break;
			case '\\':
				val = '|';
				break;
			case ';':
				val = ':';
				break;
			case '\'':
				val = '"';
				break;
			case ',':
				val = '<';
				break;
			case '.':
				val = '>';
				break;
			case '/':
				val = '?';
				break;
			case '`':
				val = '~';
				break;
		}
	}
}

/*
=====================================
Change keyboard mode
=====================================
*/
bool  CKBMode(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		int temp=0;
		if(argv[1] && sscanf(argv[1],"%d",&temp))
		{
			//Allow configs to change the mousemode if its valid
			//even before the mouse actually inits
			if( m_kbstate == DEVNONE  &&
				(temp == CKeyboard::KB_DIBUFFERED  ||
			    temp == CKeyboard::KB_DIIMMEDIATE ||
			    temp == CKeyboard::KB_WIN32))
			{
				m_mode = (CKeyboard::EKbMode)temp;
				return true;
			}
			
			HRESULT	hr= E_FAIL;

			switch(temp)
			{
			case CKeyboard::KB_DIBUFFERED:
				hr = g_pKb->Init(m_exclusive,CKeyboard::KB_DIBUFFERED);
				break;
			case CKeyboard::KB_DIIMMEDIATE:
				hr = g_pKb->Init(m_exclusive,CKeyboard::KB_DIIMMEDIATE);
				break;
			case CKeyboard::KB_WIN32:
				hr = g_pKb->Init(m_exclusive,CKeyboard::KB_WIN32);
				break;
			}
			
			if(FAILED(hr))
			{
				ComPrintf("CKeyboard:CKBMode: Couldnt change to mode %d\n",temp);
				return false;
			}
			return true;
		}
		ComPrintf("CKeyboard::CKBMode:couldnt read required mode\n");
	}

	ComPrintf("Keyboard Mode is %d\n",m_mode);
	
	switch(m_mode)
	{
	case CKeyboard::KB_NONE:
		ComPrintf("CKeyboard mode::No mode set\n");
	case CKeyboard::KB_DIBUFFERED:
		ComPrintf("CKeyboard mode::DirectInput-Buffered mode\n");
		break;
	case CKeyboard::KB_DIIMMEDIATE:
		ComPrintf("CKeyboard mode::DirectInput-Immediate mode\n");
		break;
	case CKeyboard::KB_WIN32:
		ComPrintf("CKeyboard mode::Win32 - Keyboard polling\n");
		break;
	}
	return false;
}



void SetCharTable(CKeyboard::EKbMode mode)
{
	switch(mode)
	{
	case CKeyboard::KB_WIN32:
		{
			m_chartable[VK_BACK] = INKEY_BACKSPACE;
			m_chartable[VK_TAB] =  INKEY_TAB;

			m_chartable[VK_RETURN] =	INKEY_ENTER;

			m_chartable[VK_SHIFT] =	INKEY_LEFTSHIFT;
			m_chartable[VK_CONTROL]= INKEY_LEFTCTRL;
			m_chartable[VK_MENU] =	INKEY_LEFTALT;
			m_chartable[VK_PAUSE] =  INKEY_PAUSE;
			m_chartable[VK_CAPITAL]= INKEY_CAPSLOCK;

			m_chartable[VK_SPACE] =	INKEY_SPACE;
			m_chartable[VK_PRIOR] =	INKEY_PGUP;
			m_chartable[VK_NEXT] =	INKEY_PGDN;
			m_chartable[VK_END] =	INKEY_END;
			m_chartable[VK_HOME] =	INKEY_HOME;
			m_chartable[VK_LEFT] =	INKEY_LEFTARROW;
			m_chartable[VK_UP] =		INKEY_UPARROW;
			m_chartable[VK_RIGHT] =	INKEY_RIGHTARROW;
			m_chartable[VK_DOWN] =	INKEY_DOWNARROW;
			
			m_chartable[VK_EXECUTE] = INKEY_NUMENTER;
			m_chartable[VK_SNAPSHOT] =INKEY_PRINTSCRN;
			m_chartable[VK_INSERT] =	 INKEY_INS;
			m_chartable[VK_DELETE] =	 INKEY_DEL;

			m_chartable[VK_ESCAPE] =	INKEY_ESCAPE;
			
			m_chartable['0'] = '0';
			m_chartable['1'] = '1';
			m_chartable['2'] = '2';
			m_chartable['3'] = '3';
			m_chartable['4'] = '4';
			m_chartable['5'] = '5';
			m_chartable['6'] = '6';
			m_chartable['7'] = '7';
			m_chartable['8'] = '8';
			m_chartable['9'] = '9';

			m_chartable['A'] = 'a';
			m_chartable['B'] = 'b';
			m_chartable['C'] = 'c';
			m_chartable['D'] = 'd';
			m_chartable['E'] = 'e';
			m_chartable['F'] = 'f';
			m_chartable['G'] = 'g';
			m_chartable['H'] = 'h';
			m_chartable['I'] = 'i';
			m_chartable['J'] = 'j';
			m_chartable['K'] = 'k';
			m_chartable['L'] = 'l';
			m_chartable['M'] = 'm';
			m_chartable['N'] = 'n';
			m_chartable['O'] = 'o';
			m_chartable['P'] = 'p';
			m_chartable['Q'] = 'q';
			m_chartable['R'] = 'r';
			m_chartable['S'] = 's';
			m_chartable['T'] = 't';
			m_chartable['U'] = 'u';
			m_chartable['W'] = 'w';
			m_chartable['X'] = 'x';
			m_chartable['Y'] = 'y';
			m_chartable['Z'] = 'z';

			m_chartable[VK_NUMPAD0] =INKEY_NUM0;
			m_chartable[VK_NUMPAD1] =INKEY_NUM1;
			m_chartable[VK_NUMPAD2] =INKEY_NUM2;
			m_chartable[VK_NUMPAD3] =INKEY_NUM3;
			m_chartable[VK_NUMPAD4] =INKEY_NUM4;
			m_chartable[VK_NUMPAD5] =INKEY_NUM5;
			m_chartable[VK_NUMPAD6] =INKEY_NUM6;
			m_chartable[VK_NUMPAD7] =INKEY_NUM7;
			m_chartable[VK_NUMPAD8] =INKEY_NUM8;
			m_chartable[VK_NUMPAD9] =INKEY_NUM9;
			
			m_chartable[VK_MULTIPLY] =	INKEY_NUMSTAR;
			m_chartable[VK_ADD] =		INKEY_NUMPLUS;
		//	m_chartable[VK_SEPARATOR] =
			m_chartable[VK_SUBTRACT] =	INKEY_NUMMINUS;
			m_chartable[VK_DECIMAL] =	INKEY_NUMPERIOD;
			m_chartable[VK_DIVIDE] =		INKEY_NUMSLASH;

			m_chartable[VK_F1] = INKEY_F1;
			m_chartable[VK_F2] = INKEY_F2;
			m_chartable[VK_F3] = INKEY_F3;
			m_chartable[VK_F4] = INKEY_F4;
			m_chartable[VK_F5] = INKEY_F5;
			m_chartable[VK_F6] = INKEY_F6;
			m_chartable[VK_F7] = INKEY_F7;
			m_chartable[VK_F8] = INKEY_F8;
			m_chartable[VK_F9] = INKEY_F9;
			m_chartable[VK_F10]= INKEY_F10;
			m_chartable[VK_F11]= INKEY_F11;
			m_chartable[VK_F12]= INKEY_F12;

			m_chartable[VK_NUMLOCK] = INKEY_NUMLOCK;
			m_chartable[VK_SCROLL] =  INKEY_SCROLLLOCK;

			m_chartable[VK_SEMICOLON] =  ';';
			m_chartable[VK_EQUALS] =  '=';
			m_chartable[VK_COMMA] =  ',';
			m_chartable[VK_MINUS] =  '-';
			m_chartable[VK_PERIOD] =  '.';
			m_chartable[VK_SLASH] =  '/';
			m_chartable[VK_GRAVE] =  '`';
			m_chartable[VK_LBRACKET] =  '[';
			m_chartable[VK_BACKSLASH] =  '\\';
			m_chartable[VK_RBRACKET] =  ']';
			m_chartable[VK_QUOTE] =  '\'';

			m_chartable[VK_CLEAR] =

			m_chartable[VK_LWIN] = 0;
			m_chartable[VK_RWIN] = 0;
			m_chartable[VK_APPS] = 0;

			m_chartable[VK_HELP] =   0;
			m_chartable[VK_SELECT] = 0;

			m_chartable[VK_LSHIFT] = INKEY_LEFTSHIFT;
			m_chartable[VK_RSHIFT] = INKEY_RIGHTSHIFT;
			m_chartable[VK_RCONTROL] = INKEY_RIGHTCTRL;
			m_chartable[VK_LCONTROL] = INKEY_LEFTCTRL;
			m_chartable[VK_LMENU]	= INKEY_LEFTALT;
			m_chartable[VK_RMENU]	= INKEY_RIGHTALT;
			break;
		}
	case CKeyboard::KB_DIBUFFERED:
	case CKeyboard::KB_DIIMMEDIATE:
		{
			m_chartable[DIK_ESCAPE] = INKEY_ESCAPE;
			m_chartable[DIK_1] = '1';
			m_chartable[DIK_2] = '2';
			m_chartable[DIK_3] = '3';
			m_chartable[DIK_4] = '4';
			m_chartable[DIK_5] = '5';
			m_chartable[DIK_6] = '6';
			m_chartable[DIK_7] = '7';
			m_chartable[DIK_8] = '8';
			m_chartable[DIK_9] = '9';
			m_chartable[DIK_0] = '0';
			m_chartable[DIK_MINUS] = '-';
			m_chartable[DIK_EQUALS] = '=';
			m_chartable[DIK_BACK] = INKEY_BACKSPACE;
			m_chartable[DIK_TAB]  = INKEY_TAB;
			m_chartable[DIK_Q] = 'q';
			m_chartable[DIK_W] = 'w';
			m_chartable[DIK_E] = 'e';
			m_chartable[DIK_R] = 'r';
			m_chartable[DIK_T] = 't';
			m_chartable[DIK_Y] = 'y';
			m_chartable[DIK_U] = 'u';
			m_chartable[DIK_I] = 'i';
			m_chartable[DIK_O] = 'o';
			m_chartable[DIK_P] = 'p';
			m_chartable[DIK_LBRACKET] = '[';
			m_chartable[DIK_RBRACKET] = ']';
			m_chartable[DIK_RETURN] = INKEY_ENTER;
			m_chartable[DIK_LCONTROL] = INKEY_LEFTCTRL;
			m_chartable[DIK_A] = 'a';
			m_chartable[DIK_S] = 's';
			m_chartable[DIK_D] = 'd';
			m_chartable[DIK_F] = 'f';
			m_chartable[DIK_G] = 'g';
			m_chartable[DIK_H] = 'h';
			m_chartable[DIK_J] = 'j';
			m_chartable[DIK_K] = 'k';
			m_chartable[DIK_L] = 'l';
			m_chartable[DIK_SEMICOLON] = ';';
			m_chartable[DIK_APOSTROPHE] = '\'';
			m_chartable[DIK_GRAVE] = '`';
			m_chartable[DIK_LSHIFT] = INKEY_LEFTSHIFT;
			m_chartable[DIK_BACKSLASH] = '\\';
			m_chartable[DIK_Z] = 'z';
			m_chartable[DIK_X] = 'x';
			m_chartable[DIK_C] = 'c';
			m_chartable[DIK_V] = 'v';
			m_chartable[DIK_B] = 'b';
			m_chartable[DIK_N] = 'n';
			m_chartable[DIK_M] = 'm';
			m_chartable[DIK_COMMA] = ',';
			m_chartable[DIK_PERIOD] = '.';
			m_chartable[DIK_SLASH] = '/';
			m_chartable[DIK_RSHIFT] = INKEY_RIGHTSHIFT;
			m_chartable[DIK_MULTIPLY] = INKEY_NUMSTAR;
			m_chartable[DIK_LMENU] = INKEY_LEFTALT;
			m_chartable[DIK_SPACE] = INKEY_SPACE;
			m_chartable[DIK_CAPITAL] = INKEY_CAPSLOCK;
			
			m_chartable[DIK_F1] = INKEY_F1;
			m_chartable[DIK_F2] = INKEY_F2;
			m_chartable[DIK_F3] = INKEY_F3;
			m_chartable[DIK_F4] = INKEY_F4;
			m_chartable[DIK_F5] = INKEY_F5;
			m_chartable[DIK_F6] = INKEY_F6;
			m_chartable[DIK_F7] = INKEY_F7;
			m_chartable[DIK_F8] = INKEY_F8;
			m_chartable[DIK_F9] = INKEY_F9;
			m_chartable[DIK_F10] = INKEY_F10;
			
			m_chartable[DIK_NUMLOCK] = INKEY_NUMLOCK;
			m_chartable[DIK_SCROLL] = INKEY_SCROLLLOCK;
			m_chartable[DIK_NUMPAD7] = INKEY_NUM7;
			m_chartable[DIK_NUMPAD8] = INKEY_NUM8;
			m_chartable[DIK_NUMPAD9] = INKEY_NUM9;
			m_chartable[DIK_SUBTRACT] = INKEY_NUMMINUS;
			m_chartable[DIK_NUMPAD4] = INKEY_NUM4;
			m_chartable[DIK_NUMPAD5] = INKEY_NUM5;
			m_chartable[DIK_NUMPAD6] = INKEY_NUM6;
			m_chartable[DIK_ADD] = INKEY_NUMPLUS;
			m_chartable[DIK_NUMPAD1] = INKEY_NUM1;
			m_chartable[DIK_NUMPAD2] = INKEY_NUM2;
			m_chartable[DIK_NUMPAD3] = INKEY_NUM3;
			m_chartable[DIK_NUMPAD0] = INKEY_NUM0;
			m_chartable[DIK_DECIMAL] = INKEY_NUMPERIOD;
			m_chartable[DIK_F11] = INKEY_F11;
			m_chartable[DIK_F12] = INKEY_F12; 
			//0x58
			//0x9C
			m_chartable[DIK_NUMPADENTER] = INKEY_NUMENTER;
			m_chartable[DIK_RCONTROL] = INKEY_RIGHTCTRL;
		//	m_chartable[DIK_NUMPADCOMMA] = INKEY_NUMCOMMA
			m_chartable[DIK_DIVIDE] = INKEY_NUMSLASH;
			m_chartable[DIK_SYSRQ] = INKEY_PRINTSCRN;
			m_chartable[DIK_RMENU] = INKEY_RIGHTALT;
			m_chartable[DIK_HOME] = INKEY_HOME;
			m_chartable[DIK_UP] = INKEY_UPARROW;
			m_chartable[DIK_PRIOR] = INKEY_PGUP;
			m_chartable[DIK_LEFT] = INKEY_LEFTARROW;
			m_chartable[DIK_RIGHT] = INKEY_RIGHTARROW;
			m_chartable[DIK_END] = INKEY_END;
			m_chartable[DIK_DOWN] = INKEY_DOWNARROW;
			m_chartable[DIK_NEXT] = INKEY_PGDN;
			m_chartable[DIK_INSERT] = INKEY_INS;
			m_chartable[DIK_DELETE] = INKEY_DEL;
			m_chartable[DIK_LWIN] = 0;
			m_chartable[DIK_RWIN] = 0;
			m_chartable[DIK_APPS] = 0;
			break;
		}
	}
}
