#ifndef VOID_KEYBOARD_INTERFACE
#define VOID_KEYBOARD_INTERFACE

namespace VoidInput{

LRESULT CALLBACK Win32_KeyboardProc(int code,      // hook code
								   WPARAM wParam,  // virtual-key code
								   LPARAM lParam); // keystroke-message information
/*
===========================================
The Keyboard interface Class
Inherits from listener interface so we can have 
a default handler implementation
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
		KB_WIN32HOOK = 3,
		KB_WIN32POLL =4
	};
	
	CKeyboard(CInputState * pStateManager);
	~CKeyboard();

	bool Init(); //int exclusive,EKbMode mode);
	void Shutdown();
	bool Acquire();
	bool UnAcquire();
	void Update();
	
	bool SetKeyboardMode(EKbMode mode);
	bool SetExclusive(bool bExclusive);

	int	 GetDeviceState() const	   { return m_eKbState; }
	EKbMode GetKeyboardMode() const{ return m_eKbMode;  }

private:
	
	enum
	{	
		KB_DIBUFFERSIZE	= 32
	};

	//=============================
	//Private Member Vars

	HANDLE	m_hDIKeyboardEvent;			//Event handle
	HHOOK	hWinKbHook;					//Keyboard Hook handle

	BYTE	m_aKeyState[IN_NUMKEYS];	//Receives immediate and Win32 data
	int		m_aCharVal[IN_NUMKEYS]; 	//Key Translation table		

	EKbMode			m_eKbMode;
	EDeviceState	m_eKbState;			//Device State
	bool			m_bExclusive;
	
	CInputState *	m_pStateManager;
	
	LPDIRECTINPUTDEVICE7	m_pDIKb;	
	DIDEVICEOBJECTDATA		m_aDIBufKeydata[KB_DIBUFFERSIZE]; 
	
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

	friend LRESULT CALLBACK VoidInput::Win32_KeyboardProc(int code,     // hook code
											   WPARAM wParam,  // virtual-key code
											   LPARAM lParam); // keystroke-message information
};

}

#endif