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
class CKeyboard : public I_InKeyListener,
				  public I_CVarHandler	
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

	//CVar Handler
	bool HandleCVar(const CVar * cvar, int numArgs, char ** szArgs);

	//Toggle Exclusive mode
	HRESULT	SetExclusive(bool exclusive);

	void SetKeyListener(I_InKeyListener * plistener,
						bool bRepeatEvents,
						float fRepeatRate);
	void HandleKeyEvent(const KeyEvent_t &kevent) {}
	
	void SendKeyEvent(int &keyid, EButtonState &keyState);

	void Update();
	int	 GetDeviceState();

private:

	#define KB_DIBUFFERSIZE	16

	//========================================================================
	//Private Member Vars
	
	//Event handle
	HANDLE			m_hDIKeyboardEvent;	
	
	//Device State amd Mode
	EDeviceState	m_eKbState;
	EKbMode			m_eKbMode;
	bool			m_bExclusive;
	
	float			m_fRepeatRate;		//Current repeat rate
	bool			m_bRepeatEvents;	//Report Held events ?
	I_InKeyListener* m_pKeyHandler;		//Key listener object

	CVar 			m_pVarKbMode;

	//Key Translation table		
	uint			m_aCharVal[IN_NUMKEYS]; 	
	
	//Receives DI buffered data 
	DIDEVICEOBJECTDATA	 m_aDIBufKeydata[KB_DIBUFFERSIZE]; 
	
	//The DirectInputDevice Object
	LPDIRECTINPUTDEVICE7 m_pDIKb;	

	//========================================================================
	//Private Member funcs

	//Directinput init and shutdown funcs
	HRESULT	DI_Init(EKbMode mode);
	bool	DI_Shutdown();

	//Win32 Init/Shutdown
	HRESULT	Win32_Init(EKbMode mode);
	bool	Win32_Shutdown();

	void	FlushKeyBuffer();
	void	SetCharTable(EKbMode mode);

	//Update functions
	void	Update_DIBuffered();
	void	Update_DIImmediate();
	void	Update_Win32();

	static void	ShiftCharacter(int &val);

	friend LRESULT CALLBACK Win32_KeyboardProc(int code,       // hook code
											   WPARAM wParam,  // virtual-key code
											   LPARAM lParam); // keystroke-message information

	friend bool	CKBMode(const CVar * var, int argc, char** argv);
};

#endif