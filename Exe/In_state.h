#ifndef VOID_INPUT_STATE
#define VOID_INPUT_STATE

#include <dinput.h>
#include "In_defs.h"

namespace VoidInput {

/*
==========================================
Device states used by the Input Devices
==========================================
*/
enum EDeviceState
{
	DEVNONE			=0,		//Device has not been created
	DEVINITIALIZED	=1,		//Device exists, but has not been acquired
	DEVACQUIRED		=2		//Device exits and is acquired
};

/*
==========================================
Input state manager and event dispatcher
==========================================
*/

class CInputState : public I_InputFocusManager
{
public:
	CInputState();
	~CInputState();

	void SetKeyListener(I_InKeyListener * plistener,
						bool bRepeatEvents = false,
						float fRepeatRate = IN_DEFAULTREPEATRATE);
	
	void SetCursorListener(I_InCursorListener * plistener);

	void UpdateKey(int keyid, EButtonState keyState);
	void DispatchKeys();
	void FlushKeys();

	inline void UpdateCursor(const float &ix, const float &iy, const float &iz)
	{	m_pCursorHandler->HandleCursorEvent(ix,iy,iz); 
	}

private:

	struct DefaultCursorListener : public I_InCursorListener
	{	void HandleCursorEvent(const float &ix, const float &iy, const float &iz) {}
	};

	struct DefaultKeyListener : public I_InKeyListener
	{	void HandleKeyEvent(const KeyEvent &kevent){}
	};
	
	DefaultCursorListener	m_defCurHandler;
	DefaultKeyListener		m_defKeyHandler;

	I_InCursorListener *	m_pCursorHandler;
	I_InKeyListener	  *		m_pKeyHandler;

	bool		m_bRepeatEvents;
	float		m_fRepeatRate;

	KeyEvent	m_keyEvent;					//the key event object which gets dispatched.
	KeyEvent	m_aHeldKeys[IN_NUMKEYS];	//Used to store oldstats of the keys

	static void ShiftCharacter(int &val);
};

//Return the DirectInput object
LPDIRECTINPUT7	GetDirectInput();

void DIErrorMessageBox(HRESULT err, const char * msg);
void PrintDIError(HRESULT err, const char * msg);
}

#endif