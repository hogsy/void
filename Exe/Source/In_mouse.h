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

class CMouse : public I_InCursorListener,
			   public I_CVarHandler	
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

	//CVar Handler
	bool HandleCVar(const CVar * cvar, int numArgs, char ** szArgs);

	//Toggle Exclusive mode
	HRESULT	SetExclusive(bool exclusive);						

	//Empty func for default cursor handler
	void HandleCursorEvent(const float &ix, const float &iy, const float &iz) {}
	
	//Set Listener object
	void SetCursorListener( I_InCursorListener * plistener);	
	
	void Update();

	//Needed by the Win32 handler to calc center co-drds
	void Resize(int x, int y, int w, int h);

	EDeviceState GetDeviceState(); 

private:

 CVar		m_pVarXSens;
 CVar		m_pVarYSens;
 CVar		m_pVarSens;
 CVar		m_pVarInvert;
 CVar		m_pVarMode;
 CVar 	m_pVarFilter;

 bool CXSens(const CVar * var, int argc, char** argv);
 bool CYSens(const CVar * var, int argc, char** argv);
 bool CSens(const CVar *var, int argc, char** argv);
 bool CInvert(const CVar *var, int argc, char** argv);
 bool CMouseMode(const CVar *var, int argc, char** argv);
 bool CMouseFilter(const CVar *var, int argc, char** argv);

	//This is what gets called to update the mouse
//	void (*PollMouse)();

void Update_DIBuffered();
void Update_DIImmediate();	
void Update_Win32();	

	//Initialize to a given mode
	HRESULT Win32_Init();
	bool	Win32_Shutdown();

	HRESULT DI_Init(EMouseMode mode);
	bool	DI_Shutdown();
};

#endif