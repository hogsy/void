#ifndef VOID_MOUSE_INTERFACE
#define VOID_MOUSE_INTERFACE

#include "In_main.h"
#include "In_hdr.h"

namespace VoidInput {

/*
================================
The Mouse Interface class
Inherits from listener interface so we can have 
a default handler implementation
================================
*/
class CMouse : public I_CVarHandler	
{
public:
	
	enum EMouseMode
	{
		M_NONE		  = 0,
		M_DIIMMEDIATE = 1,
		M_DIBUFFERED  = 2,
		M_WIN32       = 3
	};
	//=============================================

	CMouse(CInputState * pStateManager);
	~CMouse();

	HRESULT Init(int exclusive, EMouseMode mode);
	void	Shutdown();
	HRESULT	SetExclusive(bool exclusive);	//Toggle Exclusive mode
	HRESULT	Acquire();						//Acquire the Mouse
	bool	UnAcquire();					//Unacquire
	void	Update();						//Update

	//Needed by the Win32 handler to calc center co-drds
	void	Resize(int x, int y, int w, int h);
	bool	HandleCVar(const CVarBase * cvar, int numArgs, char ** szArgs);
	
	EDeviceState 
			GetDeviceState() const; 
private:

	//=============================================
	enum
	{	M_DIBUFFER_SIZE	  =	16,
		M_MOUSEBUTTONS	  =	4,
		M_W32MOUSEBUTTONS =	3
	};
	//=============================================

	CInputState  * m_pStateManager;

	//Mouse State and Mode
	EDeviceState		m_eMouseState;
	EMouseMode			m_eMouseMode;

	//DirectInput Device
	LPDIRECTINPUTDEVICE7 m_pDIMouse;	

	//Input buffers
	DIMOUSESTATE2	  * m_pDIState;	
	DIDEVICEOBJECTDATA	m_aDIMouseBuf[M_DIBUFFER_SIZE];
	POINT				m_w32Pos;
	short				m_w32Buttons[M_W32MOUSEBUTTONS];

	HANDLE				m_hDIMouseEvent;

	//Current mouse co-ords
	float	m_fXPos, m_fYPos, m_fZPos;		
	//Last mouse co-ords		
	float	m_fLastXPos, m_fLastYPos, m_fLastZPos;	
	//Center of the screen, Win32 mouse routines need these
	int		m_dCenterX, m_dCenterY;
	//Other flags
	bool	m_bExclusive;

	CVar	m_pVarXSens;
	CVar	m_pVarYSens;
	CVar	m_pVarSens;
	CVar	m_pVarInvert;
	CVar	m_pVarMode;
	CVar	m_pVarFilter;

	//=============================================
	
	bool	CXSens(const CVar * var, int argc, char** argv);
	bool	CYSens(const CVar * var, int argc, char** argv);
	bool	CSens(const CVar *var, int argc, char** argv);
	bool	CMouseMode(const CVar *var, int argc, char** argv);

	void	Update_DIBuffered();
	void	Update_DIImmediate();	
	void	Update_Win32();	

	//Initialize to a given mode
	HRESULT Win32_Init();
	bool	Win32_Shutdown();

	HRESULT DI_Init(EMouseMode mode);
	bool	DI_Shutdown();

	void	DI_FlushMouseData();
};

}
#endif