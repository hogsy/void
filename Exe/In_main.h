#ifndef VOID_INPUT_INTERFACE
#define VOID_INPUT_INTERFACE

#include "In_defs.h"

//Forward declarations
namespace VoidInput
{
	class CInputState;
	class CMouse;
	class CKeyboard;
}

/*
===========================================
The Input Interface Class
is only exposed to Sys_main so it can
initialize, destroy and run updates
===========================================
*/
class CInput : public I_ConHandler
{
public:

	//Public Interface
	CInput();
	~CInput();

	bool Init();				//Initialize the Input System
	void Shutdown();			//Shutdown the Input System

	void ShowMouse(bool show);

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

	//Console Handler
	bool HandleCVar(const CVarBase * cvar, const CStringVal &strVal);
	void HandleCommand(int cmdId, const CParms &parms){}

	I_InputFocusManager * GetFocusManager();

private:

	VoidInput::CInputState	* m_pStateManager;
	VoidInput::CMouse		* m_pMouse;
	VoidInput::CKeyboard	* m_pKb;

	bool m_bCursorVisible;

	CVar m_pVarExclusive;
	
	CVar m_pVarXSens;
	CVar m_pVarYSens;
	CVar m_pVarSens;
	CVar m_pVarInvert;
	
	CVar m_pVarMouseFilter;
	CVar m_pVarMouseMode;
	CVar m_pVarKbMode;
	
	bool SetExclusive(bool on);

	bool CSetExclusive(const CStringVal &strVal);
	bool CXSens(const CStringVal &strVal);
	bool CYSens(const CStringVal &strVal);
	bool CSens(const CStringVal &strVal);
	bool CMouseMode(const CStringVal &strVal);
	bool CKBMode(const CStringVal &strVal);
};

#endif