#ifndef VOID_KEYBOARD_INTERFACE
#define VOID_KEYBOARD_INTERFACE

#include "In_main.h"
#include "In_hdr.h"

/*
===========================================
The Keyboard interface Class
Inherits from listener interface so we can have 
a default handler implementation
===========================================
*/

class CKeyboard : public I_InKeyListener	
{
public:

	enum EKbMode
	{
		KB_NONE = 0,
		KB_DIBUFFERED =  1,
		KB_DIIMMEDIATE = 2,
		KB_WIN32HOOK = 3,
		KB_WIN32POLL =4
	};
	
	CKeyboard();
	~CKeyboard();

	HRESULT	Init(int exclusive,EKbMode mode);
	void	Shutdown();

	HRESULT	Acquire();
	bool	UnAcquire();

	//Toggle Exclusive mode
	HRESULT	SetExclusive(bool exclusive);

	void SetKeyListener(I_InKeyListener * plistener,
						bool bRepeatEvents,
						float fRepeatRate);
	void HandleKeyEvent(const KeyEvent_t &kevent) {}

	void Update();
	int	 GetDeviceState();
	
private:

	//Mode specific poll functions
	void	(*PollKeyboard) ();

	//Directinput init and shutdown funcs
	HRESULT	DI_Init(EKbMode mode);
	bool	DI_Shutdown();

	//Win32 Init/Shutdown
	HRESULT	Win32_Init(EKbMode mode);
	bool	Win32_Shutdown();
};

#endif