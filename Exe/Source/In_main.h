#ifndef VOID_INPUT_INTERFACE
#define VOID_INPUT_INTERFACE

#include "Sys_hdr.h"
#include "In_defs.h"

//Heh, is this overkill ?
namespace VoidInput
{
class CInputState;
class CMouse;
}

/*
===========================================
The Input Interface Class
is only exposed to Sys_main so it can
initialize, destroy and run updates
===========================================
*/
class CInput : public I_CVarHandler
{
public:

	//Public Interface
	CInput();
	~CInput();

	bool Init();				//Initialize the Input System
	void Shutdown();			//Shutdown the Input System

	void Acquire();				//Acquire all devices
	bool AcquireMouse();		//Unacquire Mouse
	bool AcquireKeyboard();		//Unacquire KB

	void UnAcquire();			//Unacquire all devices
	bool UnAcquireMouse();		//Unacquire Mouse
	bool UnAcquireKeyboard();	//Unacquire Mouse

	void UpdateDevices();		//Update Input states for all devices
	void UpdateCursor();		//Update the cursor position
	void UpdateKeys();			//Just update keys

	//Handle resizes, needed for Win32 mouse clipping
	void Resize(int x, int y, int w, int h);

	//CVar Handler
	bool HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);

	I_InputFocusManager * GetFocusManager();

private:

	VoidInput::CInputState	* m_pStateManager;
	VoidInput::CMouse		* m_pMouse;

	CVar m_pVarExclusive;
	bool CSetExclusive(const CVar * var, int argc, char** argv);	
};

#endif