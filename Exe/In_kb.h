#ifndef VOID_KEYBOARD_INTERFACE
#define VOID_KEYBOARD_INTERFACE

#include "In_main.h"
#include "In_state.h"

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
class CKeyboard : public I_ConHandler
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

	HRESULT	Init(int exclusive,EKbMode mode);
	void	Shutdown();

	HRESULT	Acquire();
	bool	UnAcquire();
	HRESULT	SetExclusive(bool exclusive);	//Toggle Exclusive mode
	void	Update();
	int		GetDeviceState() const;

	//Console Handler
	bool	HandleCVar(const CVarBase * cvar, const CParms &parms);
	void	HandleCommand(HCMD cmdId, const CParms &parms){}

private:
	enum
	{	KB_DIBUFFERSIZE	= 16
	};

	//========================================================================
	//Private Member Vars

	CVar 	m_pVarKbMode;

	EKbMode	m_eKbMode;
	bool	m_bExclusive;

	HANDLE	m_hDIKeyboardEvent;			//Event handle
	HHOOK	hWinKbHook;					//Keyboard Hook handle

	BYTE	m_aKeyState[IN_NUMKEYS];	//Receives immediate and Win32 data
	int		m_aCharVal[IN_NUMKEYS]; 	//Key Translation table		
	
	EDeviceState			m_eKbState;			//Device State
	
	CInputState *			m_pStateManager;
	
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

	bool CKBMode(const CVar * cvar, const CParms &parms);

	friend LRESULT CALLBACK VoidInput::Win32_KeyboardProc(int code,       // hook code
											   WPARAM wParam,  // virtual-key code
											   LPARAM lParam); // keystroke-message information
};

}

#endif