#ifndef _IN_DEFS
#define _IN_DEFS

//state flags - can be changed to right/left
//================================
enum
{
	NONE = 0,
	SHIFTDOWN = 1,
	ALTDOWN = 2,
	CTRLDOWN = 3
};

//Generic button state flags,
//================================
enum EButtonState
{
	BUTTONUP		= 0,	
	BUTTONDOWN		= 1,	
	BUTTONHELD		= 2,	
	BUTTONDBLCLICK  = 4
};


//Device states used by the Input 
//object classes as a crude error check
//================================
enum EDeviceState
{
	DEVNONE			=0,		//Device has not been created
	DEVINITIALIZED	=1,		//Device exists, but has not been acquired
	DEVACQUIRED		=2		//Device exits and is acquired
};
//================================================================


/*
===========================================
custom key constants
===========================================
*/
enum
{
	// --------------------
	// characters 0-127 map to their ascii characters
	// --------------------

	INKEY_NULL			= 0x00000000,
    INKEY_BACKSPACE		= 0x00000008,
    INKEY_TAB			= 0x00000009,
    INKEY_ENTER			= 0x0000000D,
    INKEY_ESCAPE		= 0x0000001B,
    INKEY_SPACE			= 0x00000020, // same as ' ', here for convenience
	
	// regular characters don't need constants here; use 'a' 'b' etc.

	// --------------------
	// characters 128-255 are used for extended keys
	// --------------------
    
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

	INKEY_MOUSE1,
	INKEY_MOUSE2,
	INKEY_MOUSE3,
	INKEY_MOUSE4
};


//================================================================
//================================================================

typedef struct
{
	EButtonState state;
	int			 id;
	float		 time;
	byte		 flags;
}KeyEvent_t;


//Generic function which receives input
typedef void (*IN_KEYHANDLER)(const KeyEvent_t *kevent);
typedef void (*IN_CURSORHANDLER)(const float &x, const float &y, const float &z);


#endif