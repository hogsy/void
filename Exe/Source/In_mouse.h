#ifndef VOID_MOUSE_INTERFACE
#define VOID_MOUSE_INTERFACE

#include "In_main.h"
#include "In_hdr.h"

/*
================================
The Mouse Interface class
Inherits from listener interface so we can have 
a default handler implementation
================================
*/

class CMouse : public I_InCursorListener
{
public:

	enum EMouseMode
	{
		M_NONE		  = 0,
		M_DIIMMEDIATE = 1,
		M_DIBUFFERED  = 2,
		M_WIN32       = 3
	};

	CMouse();
	~CMouse();

	HRESULT Init(int exclusive, EMouseMode mode);
	void	Shutdown();

	HRESULT	Acquire();		
	bool	UnAcquire();

	HRESULT	SetExclusive(bool exclusive);						//Toggle Exclusive mode

	//Empty func for default cursor handler
	void HandleCursorEvent(const float &ix, const float &iy, const float &iz) {}
	void SetCursorListener( I_InCursorListener * plistener);	//Set Listener object
	
	void Update();	//Update Mouse

	void Resize();	//Needed by the Win32 handler to calc center co-drds

	EDeviceState GetDeviceState(); 

private:

	//This is what gets called to update the mouse
	void (*PollMouse)();

	//Initialize to a given mode
	HRESULT Win32_Init();
	bool	Win32_Shutdown();

	HRESULT DI_Init(EMouseMode mode);
	bool	DI_Shutdown();
};

#endif