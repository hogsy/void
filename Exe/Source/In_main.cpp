#include "In_main.h"
#include "In_kb.h"
#include "In_mouse.h"

//========================================================================================
//========================================================================================

CMouse		* g_pMouse=0;	//pointer to Mouse class
CKeyboard	* g_pKb=0;		//Keyboard object

static CVar	* m_pexclusive=0;

static LPDIRECTINPUT7	m_pdi=0;

static bool	CSetExclusive(const CVar * var, int argc, char** argv);

static void DefaultCursorHandler(const float &x, const float &y, const float &z);
static void DefaultKeyHandler(const KeyEvent_t *kevent);

float				g_fRepeatRate=0.0f;		//Current repeat rate
IN_KEYHANDLER		g_pfnKeyHandler=0;		//Key handler func
IN_CURSORHANDLER	g_pfnCursorHandler=0;	//Cursor Handler


//========================================================================================
//========================================================================================


//================================
//Constructor
//================================
CInput :: CInput()
{
	g_pMouse = new CMouse;
	g_pKb = new CKeyboard;

	m_pdi = NULL;

	g_pfnKeyHandler = &DefaultKeyHandler;
	g_pfnCursorHandler = &DefaultCursorHandler;

	//Register CVars
	g_pCons->RegisterCVar(&m_pexclusive,"in_ex","0",CVar::CVAR_INT,CVar::CVAR_ARCHIVE,&CSetExclusive);
}

//================================
//Destructor
//================================
CInput :: ~CInput()
{
	if(g_pMouse)
		delete g_pMouse;
	if(g_pKb)
		delete g_pKb;

	g_pMouse = 0;
	g_pKb = 0;

	g_pfnKeyHandler = 0;
	g_pfnCursorHandler = 0;

	m_pdi = 0;
}



LPDIRECTINPUT7  CInput::GetDirectInput()
{	return m_pdi;
}

//================================
//Create DI object and all devices
//================================
bool CInput :: Init()  
{
	if(m_pdi != NULL)
	{
		ComPrintf("CInput::Create :Already Intialized Direct Input\n");	
		return true;
	}

	HRESULT hr = DirectInputCreateEx(g_hInst,
									 DIRECTINPUT_VERSION, 
									 IID_IDirectInput7, 
									 (void**)&m_pdi, NULL); 
	if(FAILED(hr))
	{
		InputError(hr,"CInput::Create :Direct Input Intialization failed");
		return false;
	}
	ComPrintf("CInput::Create :Direct Input Intialized\n");

	//Create Devices
	hr = g_pMouse->Init((int)m_pexclusive->value); 
	if(FAILED(hr))
	{
		Release();
		InputError(hr,"CInput::Create :Mouse Intialization failed\n");
		return false;
	}
	
	hr =g_pKb->Init((int)m_pexclusive->value); 
	if(FAILED(hr))
	{
		Release();
		InputError(hr,"CInput::Create :Keyboard Intialization failed");
		return false;
	}

	return true;
}

//================================
//Release all devices and DI object
//================================
void CInput :: Release()
{
	//Release all Devices here
	g_pMouse->Shutdown();
	g_pKb->Shutdown();
	
	if(m_pdi) 
		m_pdi->Release();

	ComPrintf("CInput::Release :Direct Input Released\n");
}


//================================
//Acquire mouse
//================================
bool CInput :: AcquireMouse()
{
	if(!g_pMouse)
		return false;
	
	//Already acquired, or acquring was successful
	if((g_pMouse->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(g_pMouse->Acquire())))
		return true;

	ComPrintf("CInput::Acquire::Couldnt acquire mouse\n");
	return false;
}

//================================
//Acquire Keyboard
//================================
bool CInput :: AcquireKeyboard()
{
	if(!g_pKb)
		return false;
	
	//Already acquired, or acquring was successful
	if((g_pKb->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(g_pKb->Acquire())))
		return true;

	ComPrintf("CInput::Acquire::COULD NOT ACQUIRE KB\n");
	return false;
}


//================================
//Acquire input devices
//================================
void CInput :: Acquire()
{
	if(!m_pdi)
		return;

	AcquireMouse();
	AcquireKeyboard();
}	


//================================
//Unacquire mouse
//================================
bool CInput :: UnAcquireMouse()
{
	if(g_pMouse->GetDeviceState() == DEVINITIALIZED) 
		return true;

	if(g_pMouse->UnAcquire())
		return true;
	return false;
}

//================================
//Unacquire KB
//================================
bool CInput :: UnAcquireKeyboard()
{
	if(g_pKb->GetDeviceState() == DEVINITIALIZED)
		return true;

	if(g_pKb->UnAcquire())
		return true;
	return false;
}


//================================
//Unacquire input devices
//================================
void CInput :: UnAcquire()
{
	UnAcquireKeyboard();
	UnAcquireMouse();
}

/*
===========================================

===========================================
*/
void CInput::Resize()
{
	g_pMouse->Resize();
}


//================================
//Find and tell the error 
//================================
void CInput ::InputError(HRESULT err, char * msg)
{
	char error[128];
	if(msg)
	{
		strcpy(error,msg);
		strcat(error,"Direct Input Error:\n");
	}
	else
		strcpy(error,"Direct Input Error:\n");
	switch(err)
	{
		case DI_BUFFEROVERFLOW: strcat(error,"NOTATTACHED/PROPNOEFFECT/BUFFEROVERFLOW"); break;
		case DIERR_INPUTLOST: strcat(error,"INPUTLOST"); break;
		case DIERR_INVALIDPARAM: strcat(error,"INVALIDPARAM"); break;
		case DIERR_READONLY: strcat(error,"OTHERAPPHASPRIO/HANDLEEXISTS/READONLY"); break;
		case DIERR_ACQUIRED: strcat(error,"ACQUIRED"); break;
		case DIERR_NOTACQUIRED: strcat(error,"NOTACQUIRED"); break;
		case DIERR_NOAGGREGATION: strcat(error,"NOAGGREGATION"); break;
		case DIERR_ALREADYINITIALIZED: strcat(error,"ALREADYINITIALIZED"); break;
		case DIERR_NOTINITIALIZED: strcat(error,"NOTINITIALIZED"); break;
		case DIERR_UNSUPPORTED: strcat(error,"UNSUPPORTED"); break;
		case DIERR_OUTOFMEMORY: strcat(error,"OUTOFMEMORY"); break;
		case DIERR_GENERIC: strcat(error,"GENERIC"); break;
		case DIERR_NOINTERFACE: strcat(error,"NOINTERFACE"); break;
		case DIERR_DEVICENOTREG: strcat(error,"DEVICENOTREG"); break;
		case DIERR_OBJECTNOTFOUND: strcat(error,"OBJECTNOTFOUND"); break;
		case DIERR_BETADIRECTINPUTVERSION: strcat(error,"BETADIRECTINPUTVERSION"); break;
		case DIERR_BADDRIVERVER: strcat(error,"BADDRIVERVER"); break;
		case DI_POLLEDDEVICE: strcat(error,"POLLEDDEVICE"); break;
		default: strcat(error,"UNKNOWNERROR");	break;
	}
	g_pCons->MsgBox(error);
}


//================================
//The input frame
//================================
bool CInput::InputFrame()  
{
	g_pMouse->UpdateMouse();
	g_pKb->UpdateKeys();

#if 0

	if(m_bmouse)
	{

		g_pMouse->UpdateMouse();
/*		try
		{
			g_pMouse->UpdateMouse();
		}
		catch(HRESULT hr)
		{
			if(hr == DIERR_NOTINITIALIZED)
			{
				m_bmouse = false;
			}
			InputError(hr);
			return false;
		}
*/
	}
	if(m_bkb)
	{
		g_pKb->UpdateKeys();
/*		try
		{
			g_pKb->UpdateKeys();
		}
		catch(HRESULT hr)
		{
			if(hr == DIERR_NOTINITIALIZED)
			{
				m_bkb = false;
			}
			InputError(hr);
			return false;
		}
*/
	}
#endif
	return true;
}





void DefaultCursorHandler(const float &x, const float &y, const float &z)
{
}

void DefaultKeyHandler(const KeyEvent_t *kevent)
{
}



void CInput::SetKeyHandler(IN_KEYHANDLER pfnkeys, float repeatrate)
{
	if(pfnkeys)
		g_pfnKeyHandler =pfnkeys;
	else
		g_pfnKeyHandler =DefaultKeyHandler;
	g_fRepeatRate = repeatrate;

}


void CInput::SetCursorHandler(IN_CURSORHANDLER pfncursor)
{
	if(pfncursor)
		g_pfnCursorHandler = pfncursor;
	else
		g_pfnCursorHandler = DefaultCursorHandler;
		
}



/*
================================
Console Loopback Funcs
================================
*/

bool CSetExclusive(const CVar * var, int argc, char** argv)
{
	if(!g_pInput || !g_pMouse)
		return false;

	if(argc == 2 && argv[1])
	{
		float temp=0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			g_pMouse->UnAcquire();
			if(temp)
			{
				if(FAILED(g_pMouse->SetExclusive()))
				{
					ComPrintf("Failed to change Input mode to Exclusive\n");
					g_pMouse->Acquire();
					return false;
				}
			}
			else 
			{
				if(FAILED(g_pMouse->LoseExclusive()))
				{
					ComPrintf("Failed to change Input mode to NonExclusive\n");
					g_pMouse->Acquire();
					return false;
				}
			}
			g_pMouse->Acquire();
			return true;
		}
	}
	if(m_pexclusive->value)
		ComPrintf("Input in Exclusive mode\n");
	else
		ComPrintf("Input in NonExclusive mode\n");
	return false;
}


