#ifndef VOID_INPUT_INTERFACE
#define VOID_INPUT_INTERFACE

#include "Sys_hdr.h"
#include "In_defs.h"
#include <dinput.h>

/*
===========================================
The Input Interface Class
===========================================
*/

class CInput
{
public:
		
	CInput();
	~CInput();

	bool Init(); 
	void Release();				

	void SetKeyHandler(IN_KEYHANDLER pfnkeys, float repeatrate=0.0f);
	void SetCursorHandler(IN_CURSORHANDLER pfncursor);

	void Resize();				//Handle resizes, needed for Win32 mouse clipping
	
	void Acquire();				//Acquire all devices
	bool AcquireMouse();		//Unacquire Mouse
	bool AcquireKeyboard();		//Unacquire KB

	void UnAcquire();			//Unacquire all devices
	bool UnAcquireMouse();		//Unacquire Mouse
	bool UnAcquireKeyboard();	//Unacquire Mouse

	bool InputFrame();			//Update Input states for the frame

	LPDIRECTINPUT7  GetDirectInput();

	//Throws messagebox with error message
	void InputError(HRESULT err, char * msg=0);	
};

extern CInput	* g_pInput;	

#endif

