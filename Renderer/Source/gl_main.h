#ifndef GLUTIL_MAIN_H
#define GLUTIL_MAIN_H

#include "Standard.h"

/*
==========================================
Utility class to handle OpenGL 
Init/Shutdown
Resizes
Resolution/BPP changes etc
==========================================
*/

class CGLUtil
{
public:

	CGLUtil();
	~CGLUtil();

	bool Init();
	bool Shutdown();

	void Resize();
	void SetWindowCoords(int wndX, int wndY);
	void SetInitializePos();

	bool UpdateDisplaySettings(unsigned int width, 
							   unsigned int height, 
							   unsigned int bpp, 
							   bool fullscreen);
	bool IsDriverLoaded() { return m_loadeddriver; }

private:

	bool GoFull(uint width, uint height, uint bpp);
	bool GoWindowed(uint width, uint height);

	void EnumDisplayModes();
	bool SetupPixelFormat();

	DEVMODE*m_devmodes;		//all available display modes
	int		m_nummodes;		//Number of display modes

	bool	m_loadeddriver;
	bool	m_initialized;
	char	m_gldriver[256];
	
	CVar    m_cWndX;	//Windowed X pos
	CVar    m_cWndY;	//Windowed Y pos
};

extern CGLUtil * g_pGL;

#endif