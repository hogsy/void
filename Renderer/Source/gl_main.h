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

	bool UpdateDisplaySettings(unsigned int width, 
							   unsigned int height, 
							   unsigned int bpp, 
							   unsigned int fullscreen);
	
	bool IsDriverLoaded() { return m_loadeddriver; }

private:

	bool GoFull(unsigned int width, 
				unsigned int height, 
				unsigned int bpp);
	
	bool GoWindowed(unsigned int width, 
					unsigned int height);

	void EnumDisplayModes();
	bool SetupPixelFormat();

	bool			m_loadeddriver;
	char			m_gldriver[256];
	
	DEVMODE	   *	m_devmodes;	//all available display modes
	int				m_nummodes; //Number of display modes

	//Default to these if any windowed modes fail
	unsigned int	m_safeX;
	unsigned int    m_safeY;

	//Windowed Screen Co-ordinates
	int	m_wndXpos;
	int    m_wndYpos;
};

extern CGLUtil * g_pGL;

#endif