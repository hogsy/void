#include "In_main.h"
#include "In_hdr.h"
#include "In_kb.h"
#include "In_mouse.h"

//========================================================================================
//========================================================================================

CMouse		  *			g_pMouse=0;	//Pointer to Mouse class
CKeyboard	  *			g_pKb=0;	//Keyboard object

extern CInput *			g_pInput;	//The Input object, Need this here for Cvar funcs, and getfocusmanager

static LPDIRECTINPUT7	m_pDInput=0;	//The direct input object

static CVar	 *			m_pVarExclusive=0;
static bool	CSetExclusive(const CVar * var, int argc, char** argv);	


bool g_bDIAvailable = false;

//========================================================================================
//========================================================================================

/*
=====================================
Constructor
=====================================
*/
CInput::CInput()
{
	g_pMouse = new CMouse();
	g_pKb = new CKeyboard();

	m_pDInput = 0;

	//Register CVars
	g_pCons->RegisterCVar(&m_pVarExclusive,"in_ex","0", CVar::CVAR_INT,CVar::CVAR_ARCHIVE,&CSetExclusive);
}

/*
=====================================
Destructor
=====================================
*/
CInput::~CInput()
{
	if(g_pMouse)
	{
		delete g_pMouse;
		g_pMouse = 0;
	}
	if(g_pKb)
	{
		delete g_pKb;
		g_pKb = 0;
	}
	m_pDInput = 0;
}

/*
=====================================
Initialize the input subsystem
=====================================
*/
bool CInput::Init()  
{
	if(m_pDInput != NULL)
	{
		ComPrintf("CInput::Init:Already Intialized DirectInput\n");	
		return true;
	}

	HRESULT hr = DirectInputCreateEx(g_hInst,
									 DIRECTINPUT_VERSION, 
									 IID_IDirectInput7, 
									 (void**)&m_pDInput, NULL); 
	if(FAILED(hr))
	{
		ComPrintf("CInput::Init:DirectInput Intialization Failed\n");
		g_bDIAvailable = false;
	}
	else
	{
		g_bDIAvailable = true;
		ComPrintf("CInput::Init:DirectInput Intialized\n");
	}

	//Initialize Devices

	//Are Initialized without specifying any modes, so that they
	//can default to what they read from config files

	hr = g_pMouse->Init((int)m_pVarExclusive->value, CMouse::M_NONE); 
	if(FAILED(hr))
	{
		Shutdown();
		In_DIErrorMessageBox(hr,"CInput::Init:Mouse Intialization failed\n");
		return false;
	}
	
	hr =g_pKb->Init((int)m_pVarExclusive->value, CKeyboard::KB_NONE); 
	if(FAILED(hr))
	{
		Shutdown();
		In_DIErrorMessageBox(hr,"CInput::Init:Keyboard Intialization failed");
		return false;
	}
	return true;
}

/*
=====================================
Shuts down the Input System
=====================================
*/
void CInput::Shutdown()
{
	//Release all Devices here
	g_pMouse->Shutdown();
	g_pKb->Shutdown();
	
	if(m_pDInput) 
	{
		m_pDInput->Release();
		ComPrintf("CInput::Release :DirectInput Released\n");
	}
	ComPrintf("CInput::Shutdown - OK\n");
}


/*
=====================================
Acquire the mouse
=====================================
*/
bool CInput::AcquireMouse()
{
	//If device has not been initialized then just return
	if((!g_pMouse) || g_pMouse->GetDeviceState() == DEVNONE)
		return false;
	
	//Already acquired, or acquring was successful
	if((g_pMouse->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(g_pMouse->Acquire())))
		return true;

	ComPrintf("CInput::Acquire::Couldnt acquire mouse\n");
	return false;
}

/*
=====================================
Acquire the Keyboard
=====================================
*/
bool CInput::AcquireKeyboard()
{
	//If device has not been initialized then just return
	if((!g_pKb) || g_pKb->GetDeviceState() == DEVNONE)
		return false;
	
	//Already acquired, or acquring was successful
	if((g_pKb->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(g_pKb->Acquire())))
		return true;

	ComPrintf("CInput::Acquire::Couldnt acquire keyboard\n");
	return false;
}


/*
=====================================
Acquire Input Devices
=====================================
*/
void CInput :: Acquire()
{
	AcquireMouse();
	AcquireKeyboard();
}	


/*
=====================================
Unacquire the Mouse
=====================================
*/
bool CInput::UnAcquireMouse()
{
	if(!g_pMouse)
		return true;

	if(g_pMouse->GetDeviceState() == DEVINITIALIZED) 
		return true;

	if(g_pMouse->UnAcquire())
		return true;
	return false;
}

/*
=====================================
Unacquire the keyboard
=====================================
*/
bool CInput::UnAcquireKeyboard()
{
	if(!g_pKb)
		return true;

	if(g_pKb->GetDeviceState() == DEVINITIALIZED)
		return true;

	if(g_pKb->UnAcquire())
		return true;
	return false;
}

/*
=====================================
Unacquire Input devices
=====================================
*/
void CInput :: UnAcquire()
{
	UnAcquireKeyboard();
	UnAcquireMouse();
}

/*
=====================================
call resize event for devices which
need it
=====================================
*/
void CInput::Resize()
{
	g_pMouse->Resize();
}


/*
=====================================
Update functions
=====================================
*/
void CInput::UpdateKeys() 
{ 	g_pKb->Update(); 
}			

void CInput::UpdateCursor() 
{ 	g_pMouse->Update();
}

void CInput::UpdateDevices()  
{
	g_pMouse->Update();
	g_pKb->Update();
}

/*
=====================================
Handle Listener requests
=====================================
*/
void CInput::SetKeyListener( I_InKeyListener * plistener,
							bool bRepeatEvents,
							float fRepeatRate)
{
	g_pKb->SetKeyListener(plistener,bRepeatEvents,fRepeatRate);
}

void CInput::SetCursorListener( I_InCursorListener * plistener)
{
	g_pMouse->SetCursorListener(plistener);
}


//========================================================================================
//========================================================================================

/*
================================
Console Loopback Func
Set exclusive access
================================
*/
bool CSetExclusive(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp)
			{
				if(FAILED(g_pMouse->SetExclusive(true)) ||
				   FAILED(g_pKb->SetExclusive(true)))
				{
					ComPrintf("Failed to change Input mode to Exclusive\n");
					return false;
				}
			}
			else 
			{
				if(FAILED(g_pMouse->SetExclusive(false)) ||
				   FAILED(g_pKb->SetExclusive(false)))
				{
					ComPrintf("Failed to change Input mode to NonExclusive\n");
					return false;
				}
			}
			return true;
		}
	}
	if(m_pVarExclusive->value)
		ComPrintf("Input in Exclusive mode\n");
	else
		ComPrintf("Input in NonExclusive mode\n");
	return false;
}


//========================================================================================

/*
=====================================
Returns the DI object for Mouse/Keyboard
=====================================
*/
LPDIRECTINPUT7  In_GetDirectInput()
{	return m_pDInput;
}


/*
=====================================
Throws an error message box
=====================================
*/
void In_DIErrorMessageBox(HRESULT err, char * msg)
{
	char error[128];
	if(msg)
	{
		strcpy(error,msg);
		strcat(error,"DirectInput Error:\n");
	}
	else
		strcpy(error,"DirectInput Error:\n");
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


/*
=====================================
Returns Input focus manager
i.e The Input object
=====================================
*/
I_InputFocusManager * GetInputFocusManager()
{ return g_pInput; 
}




