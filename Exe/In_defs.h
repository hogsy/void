#ifndef VOID_INPUT_DEFINITIONS
#define VOID_INPUT_DEFINITIONS

#include "Com_defs.h"

/*
Listeners need these definations to handle input events
*/
const int   IN_NUMKEYS = 256;
const float IN_DEFAULTREPEATRATE = 0.04f;

/*
===========================================
Generic button state flags,
===========================================
*/
enum EButtonState
{
	BUTTONUP		= 0,	
	BUTTONDOWN		= 1,	
	BUTTONHELD		= 2,	
};

/*
===========================================
Key Event flags
===========================================
*/
enum EKeyEventFlags
{
	NONE = 0,
	SHIFTDOWN = 1,
	ALTDOWN = 2,
	CTRLDOWN = 4
};

/*
===========================================
Input Key Constants
===========================================
*/
enum EInKey
{
	// characters 0-127 map to ascii characters

	INKEY_NULL			= 0x00000000,
    INKEY_BACKSPACE		= 0x00000008,
    INKEY_TAB			= 0x00000009,
    INKEY_ENTER			= 0x0000000D,
    INKEY_ESCAPE		= 0x0000001B,
    INKEY_SPACE			= 0x00000020,	// same as ' ', here for convenience
	

	// 128-255 used for extended keys
   
	// arrow keys
	
	INKEY_UPARROW = 128,
    INKEY_DOWNARROW = 129,
	INKEY_LEFTARROW = 130,
    INKEY_RIGHTARROW = 131,

    // function keys
	INKEY_F1	= 132,
    INKEY_F2	= 133,
    INKEY_F3	= 134,
    INKEY_F4	= 135,
    INKEY_F5	= 136,
    INKEY_F6	= 137,
    INKEY_F7	= 138,
    INKEY_F8	= 139,
    INKEY_F9	= 140,
    INKEY_F10	= 141,
    INKEY_F11	= 142,
    INKEY_F12	= 143,
	// cursor control keys
    INKEY_INS	= 144,
    INKEY_DEL	= 145,
    INKEY_HOME	= 146,
    INKEY_END	= 147,
    INKEY_PGUP	= 148,
    INKEY_PGDN	= 149,

	INKEY_LEFTSHIFT		= 150,
    INKEY_RIGHTSHIFT	= 151,
	INKEY_LEFTCTRL		= 152,
	INKEY_RIGHTCTRL		= 153,
    INKEY_LEFTALT		= 154,
    INKEY_RIGHTALT		= 155,

	// numeric keypad
    INKEY_NUMSLASH,
    INKEY_NUMSTAR,
    INKEY_NUMMINUS,
    INKEY_NUMPLUS,
    INKEY_NUMENTER,
    INKEY_NUMPERIOD,
    INKEY_NUM0,
    INKEY_NUM1,
    INKEY_NUM2,
    INKEY_NUM3,
    INKEY_NUM4,
    INKEY_NUM5,
    INKEY_NUM6,
    INKEY_NUM7,
    INKEY_NUM8,
    INKEY_NUM9,
    
	// locks and misc keys
	INKEY_NUMLOCK,
    INKEY_CAPSLOCK,
    INKEY_SCROLLLOCK,
    INKEY_PRINTSCRN,
    INKEY_PAUSE,

	//Mouse buttons
	INKEY_MOUSE1	= 252,
	INKEY_MOUSE2	= 253,
	INKEY_MOUSE3	= 254,
	INKEY_MOUSE4	= 255
};

//======================================================================================
//======================================================================================
/*
===========================================
Key Event Object
===========================================
*/
struct KeyEvent
{
	KeyEvent() 
	{ 
		state = BUTTONUP;
		flags = 0;
		id = 0;
		time = 0.0f;
	}

	EButtonState state;		//Button State
	byte		 flags;		//Button Flags
	int			 id;		//Button Id var
	float		 time;		//Time the event occured
};

/*
===========================================
InputListener Interfaces
===========================================
*/
struct I_InCursorListener
{	
	virtual void HandleCursorEvent(const float &ix, 
								   const float &iy,
								   const float &iz)=0;
};

struct I_InKeyListener
{	
	virtual void HandleKeyEvent(const KeyEvent &kevent)=0;
};

/*
===========================================
Input Focus Manager
This is what the listeners register to
===========================================
*/
struct I_InputFocusManager
{
	virtual void SetKeyListener(I_InKeyListener * plistener,
								bool bRepeatEvents = false,
								float fRepeatRate = IN_DEFAULTREPEATRATE)=0;
	virtual void SetCursorListener(I_InCursorListener * plistener)=0;
};

//======================================================================================
//Defined in SysMain where the Input System is actually created.
namespace System
{
	I_InputFocusManager * GetInputFocusManager();
}

#endif