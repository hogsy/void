#ifndef VOID_INPUT_INTERNAL
#define VOID_INPUT_INTERNAL


#include <dinput.h>

//Device states used by the Input 
//object classes as a crude error check
//================================
enum EDeviceState
{
	DEVNONE			=0,		//Device has not been created
	DEVINITIALIZED	=1,		//Device exists, but has not been acquired
	DEVACQUIRED		=2		//Device exits and is acquired
};

//Flag to determine availability of DirectInput
//Everything defaults to Win32 if it doesnt find DI
extern bool g_bDIAvailable;

//Private Input Functions
//================================

extern LPDIRECTINPUT7  In_GetDirectInput();						//Return the Direct Input object
extern void In_DIErrorMessageBox(HRESULT err, char * msg=0);	//Throws messagebox with error message

//Implemented in In_kb, allows other devices to send Button Events
//Automatically takes care of held events.
extern void In_UpdateKey(int keyid, EButtonState keyState);

#endif