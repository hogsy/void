#ifndef VOID_MOUSE_INTERFACE
#define VOID_MOUSE_INTERFACE

namespace VoidInput {

/*
================================
The Mouse Interface class
Inherits from listener interface so we can have 
a default handler implementation
================================
*/
class CMouse
{
public:
	
	enum EMouseMode
	{
		M_NONE		  = 0,
		M_DIIMMEDIATE = 1,
		M_DIBUFFERED  = 2,
		M_WIN32       = 3
	};
	
	CMouse(CInputState * pStateManager);
	~CMouse();

	bool Init();
	void Shutdown();
	bool Acquire();
	bool UnAcquire();
	void Update();

	//Needed by the Win32 handler to calc center co-drds
	void Resize(int x, int y, int w, int h);
	
	//Modifiers
	bool SetExclusive(bool exclusive);
	bool SetMouseMode(EMouseMode mode);
	void SetXSensitivity(float val){ m_fXSens = val; }
	void SetYSensitivity(float val){ m_fYSens = val; }
	void SetSensitivity(float val) { m_fSens = val; }
	void SetFilter(bool on) { m_bFilter = on; }
	void SetInvert(bool on) { m_bInvert = on; }

	//Accessors
	float GetXSens()  const { return m_fXSens; }
	float GetYSens()  const { return m_fYSens; }
	float GetSens()   const { return m_fXSens; }
	bool  GetFilter() const { return m_bFilter; }
	bool  GetInvert() const { return m_bInvert; }
	EDeviceState GetDeviceState() const { return m_eMouseState;}
	EMouseMode   GetMouseMode() const { return m_eMouseMode; }

private:

	enum
	{	
		M_DIBUFFER_SIZE	  =	8,
		M_MOUSEBUTTONS	  =	4,
		M_W32MOUSEBUTTONS =	3
	};
	//=============================================

	CInputState     *	m_pStateManager;

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
	//Sens
	float	m_fXSens, m_fYSens, m_fSens;		
	//Center of the screen, Win32 mouse routines need these
	int		m_dCenterX, m_dCenterY;
	//Other flags
	bool	m_bExclusive, m_bFilter, m_bInvert;

	//=============================================

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