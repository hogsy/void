#include "Sys_hdr.h"
#include "In_main.h"
#include "In_state.h"
#include "In_kb.h"
#include "In_mouse.h"
#include "Com_util.h"

using namespace VoidInput;

LPDIRECTINPUT7	m_pDInput=0;//The direct input object



/*
=====================================
Constructor
=====================================
*/
CInput::CInput() : 
				m_pVarExclusive("in_ex","false", CVAR_BOOL,CVAR_ARCHIVE),
				m_pVarXSens("in_xsens","0.2",CVAR_FLOAT,CVAR_ARCHIVE),
				m_pVarYSens("in_ysens","0.2",CVAR_FLOAT,CVAR_ARCHIVE),
				m_pVarSens ("in_sens","5.0",CVAR_FLOAT,CVAR_ARCHIVE),
				m_pVarInvert("in_invert","0",CVAR_BOOL,CVAR_ARCHIVE),
				m_pVarMouseMode("in_mousemode","1",CVAR_INT,CVAR_ARCHIVE),
				m_pVarMouseFilter("in_filter","0",CVAR_BOOL, CVAR_ARCHIVE)
{
	m_pStateManager = new CInputState();

	m_pMouse = new CMouse(m_pStateManager);
	m_pKb = new CKeyboard(m_pStateManager);

	//Register CVars
	System::GetConsole()->RegisterCVar(&m_pVarExclusive,this);
	System::GetConsole()->RegisterCVar(&m_pVarXSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarYSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarSens,this);
	System::GetConsole()->RegisterCVar(&m_pVarInvert,this);
	System::GetConsole()->RegisterCVar(&m_pVarMouseMode,this);
	System::GetConsole()->RegisterCVar(&m_pVarMouseFilter, this);
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

	m_pMouse->SetExclusive(m_pVarExclusive.bval);
	m_pMouse->SetMouseMode((CMouse::EMouseMode)m_pVarMouseMode.ival);
	
	if(!m_pMouse->Init())
	{
		Shutdown();
		return false;
	}


	hr =m_pKb->Init(m_pVarExclusive.ival, CKeyboard::KB_NONE); 
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
	ComPrintf("CInput::Shutdown: OK\n");
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
	if((m_pMouse->GetDeviceState() == DEVACQUIRED) || m_pMouse->Acquire())
		return true;

	ComPrintf("CInput::Acquire::Couldn't acquire Mouse\n");
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

	ComPrintf("CInput::Acquire::Couldn't acquire Keyboard\n");
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
{	m_pMouse->Resize(x,y,w,h);
}


bool CInput::SetExclusive(bool on)
{
	if(!m_pMouse->SetExclusive(on) ||
	   FAILED(m_pKb->SetExclusive(on)))
	{
		if(on)
			ComPrintf("Failed to change Input mode to Exclusive\n");
		else
			ComPrintf("Failed to change Input mode to NonExclusive\n");
		return false;
	}
	return true;
}

bool CInput::GetExclusiveVar()
{	return m_pVarExclusive.bval;
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

I_InputFocusManager * CInput::GetFocusManager() { return m_pStateManager; }

//========================================================================================
//========================================================================================

/*
================================
Console Loopback Func
Set exclusive access
================================
*/
bool CInput::CSetExclusive(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		int temp= parms.IntTok(1);
		if(temp >= 0)
		{
			if(temp)
				return SetExclusive(true);
			else
				return SetExclusive(false);
		}
	}
	if(m_pVarExclusive.ival)
		ComPrintf("Input in Exclusive mode\n");
	else
		ComPrintf("Input in NonExclusive mode\n");
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
bool CInput::CMouseMode(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		int mode= parms.IntTok(1);
		
		if(mode < CMouse::M_DIIMMEDIATE || mode > CMouse::M_WIN32)
		{
			ComPrintf("Invalid Mouse Mode specified\n");
			return false;
		}
			
		if(!m_pMouse->SetMouseMode((CMouse::EMouseMode)mode))
		{
			ComPrintf("CMouse:CMouseMode: Couldn't change to mode %d\n",mode);
			return false;
		}
		return true;
	}

	//Show current info
	ComPrintf("MouseMode is %d\n", m_pMouse->GetMouseMode());

	switch(m_pMouse->GetMouseMode())
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
bool CInput::CXSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
		{
			m_pMouse->SetXSensitivity(sens);
			return true;
		}
		ComPrintf("CMouse::CXSens:Invalid Value entered");
	}
	ComPrintf("X-axis Sensitivity: %.2f\n", m_pMouse->GetXSens());
	return false;
}

/*
======================================
Console Func - Sets Y-axis Sensitivity
======================================
*/
bool CInput::CYSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
		{
			m_pMouse->SetYSensitivity(sens);
			return true;
		}
		ComPrintf("CMouse::CYSens:Invalid Value entered");
	}
	ComPrintf("Y-axis Sensitivity: %.2f\n", m_pMouse->GetYSens());
	return false;
}

/*
======================================
Console Func - Sets master Sensitivity
======================================
*/
bool CInput::CSens(const CVar * var, const CParms &parms)
{
	if(parms.NumTokens() > 1)
	{
		float sens = parms.FloatTok(1);
		if(sens > 0.0 && sens < 30.0)
		{
			m_pMouse->SetSensitivity(sens);
			return true;
		}
		ComPrintf("CMouse::CSens:Invalid Value entered");
	}
	ComPrintf("Mouse Sensitivity: %.2f\n", m_pMouse->GetSens());
	return false;
}


/*
================================================
Handle Cvar change notifications
================================================
*/
bool CInput::HandleCVar(const CVarBase * cvar, const CParms &parms)
{
	if(cvar == &m_pVarExclusive)
		return CSetExclusive((CVar*)cvar,parms);
	else if(cvar == &m_pVarXSens)
		return CXSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarYSens)
		return CYSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarSens)
		return CSens((CVar*)cvar,parms);
	else if(cvar == &m_pVarMouseMode)
		return CMouseMode((CVar*)cvar,parms);
	else if(cvar == &m_pVarMouseFilter)
	{
		if(parms.NumTokens() > 1)
		{
			if(parms.IntTok(1))
				m_pMouse->SetFilter(true);
			else
				m_pMouse->SetFilter(false);
			return true;
		}
		if(m_pMouse->GetFilter())
			ComPrintf("Mouse input is filtered\n");
		else
			ComPrintf("Mouse input is not filtered\n");
	}
	else if(cvar == &m_pVarInvert)
	{
		if(parms.NumTokens() > 1)
		{
			if(parms.IntTok(1))
				m_pMouse->SetInvert(true);
			else
				m_pMouse->SetInvert(false);
			return true;
		}
		if(m_pMouse->GetInvert())
			ComPrintf("Mouse is inverted\n");
		else
			ComPrintf("Mouse is not inverted\n");
	}
	return false;
}




//======================================================================================
//======================================================================================



namespace VoidInput {

LPDIRECTINPUT7  GetDirectInput() 
{	return m_pDInput;  
}

/*
=====================================
Throws an error message box
=====================================
*/
void DIErrorMessageBox(HRESULT err, const char * msg)
{
	char error[128];
	if(msg)
	{
		strcpy(error,msg);
		strcat(error," Error:\n");
	}
	else
		strcpy(error,"Error:\n");
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

void PrintDIError(HRESULT err, const char * msg)
{
	char error[128];
	if(msg)
	{
		strcpy(error,msg);
		strcat(error,"Error:");
	}
	else
		strcpy(error,"Error:");

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
	ComPrintf("%s\n",error);
}

}