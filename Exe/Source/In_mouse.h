#ifndef VOID_MOUSE_INTERFACE
#define VOID_MOUSE_INTERFACE

#include "In_main.h"

/*
================================
The Mouse Interface class
================================
*/

class CMouse
{
public:

	enum EMouseMode
	{
		M_NONE		  = 0,
		M_DIBUFFERED  = 1,
		M_DIIMMEDIATE = 2,
		M_WIN32       = 3
	};

	CMouse();
	~CMouse();

	HRESULT Init(int exclusive, EMouseMode mode=M_NONE);
	void	Shutdown();

	//Acquire the mouse
	HRESULT	Acquire();
	bool	UnAcquire();
	
	//This is what gets called to update the mouse
	void	(*UpdateMouse)();

	//Needed by the Win32 handler to calc center co-drds
	void	Resize();

	HRESULT	SetExclusive();
	HRESULT	LoseExclusive();

	EDeviceState GetDeviceState(); 

private:

	//Initialize to a given mode
	HRESULT Win32_Init();
	bool	Win32_Shutdown();

	HRESULT DI_Init(EMouseMode mode);
	bool	DI_Shutdown();
};

extern CMouse	 * g_pMouse;	//pointer to Mouse class

#endif