#ifndef VOID_KEYBOARD_INTERFACE
#define VOID_KEYBOARD_INTERFACE

#include "In_main.h"

/*
===========================================
The Keyboard interface Class
===========================================
*/

class CKeyboard
{
public:

	enum EKbMode
	{
		KB_NONE = 0,
		KB_DIBUFFERED =  1,
		KB_DIIMMEDIATE = 2,
		KB_WIN32 =3
	};
	
	CKeyboard();
	~CKeyboard();

	HRESULT	Init(int exclusive,EKbMode mode=KB_NONE);
	void	Shutdown();

	HRESULT	Acquire();
	bool	UnAcquire();

	int		GetDeviceState();
	
	//This is always set to something, its whats called to update
	void	(*UpdateKeys) ();

private:

	//Direct input init and shutdown funcs
	HRESULT	DI_Init(EKbMode mode);
	bool	DI_Shutdown();

	//Win32 Init/Shutdown
	HRESULT	Win32_Init();
	bool	Win32_Shutdown();
};

extern CKeyboard	* g_pKb;		//Keyboard object


#endif