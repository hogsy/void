#include "In_main.h"
#include "In_state.h"
#include "In_kb.h"
#include "In_mouse.h"

using namespace VoidInput;

LPDIRECTINPUT7	m_pDInput=0;//The direct input object

namespace VoidInput
{	LPDIRECTINPUT7  GetDirectInput() {	return m_pDInput;  }
}

static void DIErrorMessageBox(HRESULT err, char * msg);

//========================================================================================
//========================================================================================

/*
=====================================
Constructor
=====================================
*/
CInput::CInput() : m_pVarExclusive("in_ex","0", CVar::CVAR_INT,CVar::CVAR_ARCHIVE)
{
	m_pStateManager = new CInputState();

	m_pMouse = new CMouse(m_pStateManager);
	m_pKb = new CKeyboard(m_pStateManager);

	//Register CVars
	System::GetConsole()->RegisterCVar(&m_pVarExclusive,this);
}

/*
=====================================
Destructor
=====================================
*/
CInput::~CInput()
{
	if(m_pMouse)
	{
		delete m_pMouse;
		m_pMouse = 0;
	}
	if(m_pKb)
	{
		delete m_pKb;
		m_pKb = 0;
	}
	delete m_pStateManager;
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

	HRESULT hr = DirectInputCreateEx(System::GetHInstance(),
									 DIRECTINPUT_VERSION, 
									 IID_IDirectInput7, 
									 (void**)&m_pDInput, NULL); 
	if(FAILED(hr))
		ComPrintf("CInput::Init:DirectInput Intialization Failed\n");
	else
		ComPrintf("CInput::Init:DirectInput Intialized\n");

	//Initialize Devices

	//Are Initialized without specifying any modes, so that they
	//can default to what they read from config files

	hr = m_pMouse->Init((int)m_pVarExclusive.value, CMouse::M_NONE); 
	if(FAILED(hr))
	{
		Shutdown();
		DIErrorMessageBox(hr,"CInput::Init:Mouse Intialization failed\n");
		return false;
	}
	
	hr =m_pKb->Init((int)m_pVarExclusive.value, CKeyboard::KB_NONE); 
	if(FAILED(hr))
	{
		Shutdown();
		DIErrorMessageBox(hr,"CInput::Init:Keyboard Intialization failed");
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
	m_pMouse->Shutdown();
	m_pKb->Shutdown();
	
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
	if((!m_pMouse) || m_pMouse->GetDeviceState() == DEVNONE)
		return false;
	
	//Already acquired, or acquring was successful
	if((m_pMouse->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(m_pMouse->Acquire())))
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
	if((!m_pKb) || m_pKb->GetDeviceState() == DEVNONE)
		return false;
	
	//Already acquired, or acquring was successful
	if((m_pKb->GetDeviceState() == DEVACQUIRED) || (SUCCEEDED(m_pKb->Acquire())))
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
	if(!m_pMouse)
		return true;

	if(m_pMouse->GetDeviceState() == DEVINITIALIZED) 
		return true;

	if(m_pMouse->UnAcquire())
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
	if(!m_pKb)
		return true;

	if(m_pKb->GetDeviceState() == DEVINITIALIZED)
		return true;

	if(m_pKb->UnAcquire())
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
void CInput::Resize(int x, int y, int w, int h)
{
	m_pMouse->Resize(x,y,w,h);
}


/*
=====================================
Update functions
=====================================
*/
void CInput::UpdateKeys() 
{ 	m_pKb->Update(); 
}			

void CInput::UpdateCursor() 
{ 	m_pMouse->Update();
}

void CInput::UpdateDevices()  
{
	m_pMouse->Update();
	m_pKb->Update();
}

/*
=====================================
=====================================
*/
I_InputFocusManager * CInput::GetFocusManager() { return m_pStateManager; }

//========================================================================================
//========================================================================================

bool CInput::HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs)
{
	if(cvar == &m_pVarExclusive)
		return CSetExclusive((CVar*)cvar,numArgs,szArgs);
	return false;
}

/*
================================
Console Loopback Func
Set exclusive access
================================
*/
bool CInput::CSetExclusive(const CVar * var, int argc, char** argv)
{
	if(argc == 2 && argv[1])
	{
		float temp=0;
		if(argv[1] && sscanf(argv[1],"%f",&temp))
		{
			if(temp)
			{
				if(FAILED(m_pMouse->SetExclusive(true)) ||
				   FAILED(m_pKb->SetExclusive(true)))
				{
					ComPrintf("Failed to change Input mode to Exclusive\n");
					return false;
				}
			}
			else 
			{
				if(FAILED(m_pMouse->SetExclusive(false)) ||
				   FAILED(m_pKb->SetExclusive(false)))
				{
					ComPrintf("Failed to change Input mode to NonExclusive\n");
					return false;
				}
			}
			return true;
		}
	}
	if(m_pVarExclusive.value)
		ComPrintf("Input in Exclusive mode\n");
	else
		ComPrintf("Input in NonExclusive mode\n");
	return false;
}

//======================================================================================
//======================================================================================

/*
=====================================
Throws an error message box
=====================================
*/
void DIErrorMessageBox(HRESULT err, char * msg)
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
	Util::ShowMessageBox(error);
}