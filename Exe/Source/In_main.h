#ifndef VOID_INPUT_INTERFACE
#define VOID_INPUT_INTERFACE

#include "Sys_hdr.h"
#include "In_defs.h"

/*
===========================================
The Input Interface Class
is only exposed to Sys_main so it can
initialize, destroy and run updates
===========================================
*/

class CInput : public I_InputFocusManager,
			   public I_CVarHandler
{
public:

	//Public Interface
	CInput();
	~CInput();

	bool Init();				//Initialize the Input System
	void Shutdown();			//Shutdown the Input System

	void Resize(int x, int y, int w, int h);				//Handle resizes, needed for Win32 mouse clipping
	
	void Acquire();				//Acquire all devices
	bool AcquireMouse();		//Unacquire Mouse
	bool AcquireKeyboard();		//Unacquire KB

	void UnAcquire();			//Unacquire all devices
	bool UnAcquireMouse();		//Unacquire Mouse
	bool UnAcquireKeyboard();	//Unacquire Mouse

	void UpdateDevices();		//Update Input states for all devices
	void UpdateKeys();			//Just update keys
	void UpdateCursor();		//Update the cursor position

	//CVar Handler
	bool HandleCVar(const CVar * cvar, int numArgs, char ** szArgs);

	//Input Focus Manager Implementation

	void SetKeyListener(I_InKeyListener * plistener,
						bool bRepeatEvents,
						float fRepeatRate);

	void SetCursorListener(I_InCursorListener * plistener);

private:
	CVar m_pVarExclusive;

	bool CSetExclusive(const CVar * var, int argc, char** argv);	
};

#endif

