#include "Gl_main.h"


CGLUtil * g_pGL=0;

/*
==========================================
Constructor/Destructor
==========================================
*/
CGLUtil::CGLUtil() : m_cWndX("r_wndx","80",CVar::CVAR_INT,CVar::CVAR_ARCHIVE),
					 m_cWndY("r_wndy","40",CVar::CVAR_INT,CVar::CVAR_ARCHIVE)
{
	m_bInitialized = false;
	
	m_nummodes = 0;
	m_devmodes = 0;

	//Enumerate the available display modes
	EnumDisplayModes();

	//load the driver
	OpenGLFindDriver(m_gldriver);
	if (!(OpenGLInit(m_gldriver)==0))
	{
		ConPrint("GL::Unable to load opengl dll\n");
		m_loadeddriver = false;
		return;
	}
	ConPrint("GL:Loaded GL driver: %s\n", m_gldriver);
	m_loadeddriver = true;

	g_pConsole->RegisterCVar(&m_cWndX);
	g_pConsole->RegisterCVar(&m_cWndY);
}


CGLUtil::~CGLUtil()
{
	if(m_devmodes)
		delete [] m_devmodes;
	m_nummodes = 0;
	OpenGLUnInit();
	ConPrint("GL::Final Shutdown OK\n");
}

/*
==========================================
Enumerate Display modes
==========================================
*/
void CGLUtil::EnumDisplayModes()
{
	DEVMODE devmode;
	
	//get the total number of modes so we can allocate memory for all of 'em.
	m_nummodes = 0;
	while (::EnumDisplaySettings(NULL, m_nummodes, &devmode))
		m_nummodes++;

	// fill an array with all the devmodes so we don't have to keep grabbing 'em.
	m_devmodes = new DEVMODE[m_nummodes];
	if (m_devmodes == NULL) 
	{
		Error("GL::EnumDisplayModes: Out of Memory for display modes");
		return;
	}

	m_nummodes = 0;
	while (::EnumDisplaySettings(NULL, m_nummodes, &m_devmodes[m_nummodes]))
		m_nummodes++;
}


/*
==========================================
Start up OpenGL
==========================================
*/
bool CGLUtil::Init()
{
	m_bInitialized = true;

	ConPrint("CGLUtil::Init:Res: %d %d\n",g_rInfo.width, g_rInfo.height);
	ConPrint("CGLUtil::Init:Pos: %d %d\n",m_cWndX.ival,m_cWndY.ival);


#ifdef DYNAMIC_GL
	//3dfx 3d only card. default to fullscreen mode and 16 bit
	if(strcmp(m_gldriver,SZ_3DFX_3DONLY_GLDRIVER)==0)
	{
		g_rInfo.rflags |= RFLAG_FULLSCREEN;
		g_rInfo.bpp = 16;
	}
#endif

	// change display before we do anything with gl
	if (g_rInfo.rflags & RFLAG_FULLSCREEN)
		g_pGL->GoFull(g_rInfo.width, g_rInfo.height, g_rInfo.bpp);
	else
		g_pGL->GoWindowed(g_rInfo.width, g_rInfo.height);

	//Get Pixel Format
	g_rInfo.hDC = ::GetDC(g_rInfo.hWnd);
	
	if (!SetupPixelFormat())
	{
		ConPrint("GL::Init: Failed to set PixelFormat\n");
		return false;
	}

	//Finally create GL context
	g_rInfo.hRC = _wglCreateContext(g_rInfo.hDC);
	_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);

	//Get GL Extentions
	//get extension pointers
	OpenGLGetExtensions();

	//Check for GL flags
	ConPrint("\nGL_VENDOR: %s\n",glGetString(GL_VENDOR));
	ConPrint("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
	ConPrint("GL_VERSION: %s\n", glGetString(GL_VERSION));
	

	const char * ext = (const char*)glGetString(GL_EXTENSIONS);
	if(!ext)
		return true;

	ConPrint("GL_EXTENSIONS:\n");
	int l = strlen(ext) + 1;
	char *ext2 = new char[l];
	ext2[l-1] = '\0';
	memcpy(ext2, ext, l);
	
	char *start = ext2;
	for (int i = 0; i < l; i++)
	{
		if (ext2[i] == ' ')
		{
			ext2[i] = NULL;
			ConPrint("%s\n", start);

			// check for extensions we want
			if (!strcmp(start, "GL_ARB_multitexture"))
				g_rInfo.rflags |= RFLAG_MULTITEXTURE;

			else if (!strcmp(start, "WGL_EXT_swap_control"))
				g_rInfo.rflags |= RFLAG_SWAP_CONTROL;

			start = &ext2[i+1];
		}
	}
	delete [] ext2;
	return true;
}

/*
==========================================
Shutdown opengl
==========================================
*/
bool CGLUtil::Shutdown()
{
	//Update Win Pos
	RECT rect;
	if(GetWindowRect(g_rInfo.hWnd, &rect))
	{
		if(rect.left < 40)	rect.left= 40;
		if(rect.top  < 20)	rect.top = 20;

		m_cWndX.Set((float)rect.left);
		m_cWndY.Set((float)rect.top);
	}

	_wglMakeCurrent(NULL, NULL);
	::ReleaseDC(g_rInfo.hWnd, g_rInfo.hDC);

	_wglDeleteContext(g_rInfo.hRC);
	::ChangeDisplaySettings(NULL, 0);
	g_rInfo.ready = false;
	return true;
}


/*
==========================================
Change to Windowed Mode
==========================================
*/
bool CGLUtil::GoWindowed(unsigned int width, unsigned int height)
{
	//3dfx 3d only card. default to fullscreen mode
	g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

	RECT wrect;
	wrect.left = 0;
	wrect.top = 0;
	wrect.right = g_rInfo.width;
	wrect.bottom= g_rInfo.height;
	
	//Adjusts Client Size
	::AdjustWindowRect(&wrect, 
					   WS_CAPTION,
					   FALSE);

	g_rInfo.width  = width;
	g_rInfo.height = height;

	width = wrect.right - wrect.left;
	height = wrect.bottom - wrect.top;

	::SetWindowPos(g_rInfo.hWnd,
				   HWND_TOP,
				   m_cWndX.ival,
				   m_cWndY.ival,
			       width,
			       height,
				   0);
	return true;
}

/*
==========================================
Change to FullScreen Mode
==========================================
*/
bool CGLUtil::GoFull(unsigned int width, unsigned int height, unsigned int bpp)
{
	//minimum requirements
	if (width < 640)
		width = 640;
	if (height < 480)
		height = 480;
	if (bpp < 16)
		bpp = 16;

	//find the mode that matches what we want the closest
	int best_mode=0, mode=0;

	for (mode = 0; mode < m_nummodes; mode++)
	{
		if ((m_devmodes[mode].dmPelsWidth  >= m_devmodes[best_mode].dmPelsWidth) &&
			(m_devmodes[mode].dmPelsWidth  <= width) &&
			(m_devmodes[mode].dmPelsHeight >= m_devmodes[best_mode].dmPelsHeight) &&
			(m_devmodes[mode].dmPelsHeight <= height) &&
			(m_devmodes[mode].dmBitsPerPel >= m_devmodes[best_mode].dmBitsPerPel) &&
			(m_devmodes[mode].dmBitsPerPel <= bpp))
			best_mode = mode;

		//hit 2 birds with 1 stone
		m_devmodes[mode].dmFields = DM_PELSWIDTH | DM_PELSHEIGHT | DM_BITSPERPEL;
	}

	ConPrint("GL::GoFull:Looking for %d x %d x %d\n", width, height, bpp);
	ConPrint("GL::GoFull:Changing Display to %d x %d x %d\n", 
				m_devmodes[best_mode].dmPelsWidth,
				m_devmodes[best_mode].dmPelsHeight,
				m_devmodes[best_mode].dmBitsPerPel);

	//try and change the mode
	LONG rv;
    if ((rv = ::ChangeDisplaySettings(&m_devmodes[best_mode], CDS_TEST)) != DISP_CHANGE_SUCCESSFUL)
	{
		ConPrint("GL:GoFull::Change to Fullscreen mode failed: ");
		switch(rv)
		{
		case DISP_CHANGE_RESTART:
			ConPrint("The computer must be restarted in order for the graphics mode to work.\n");
			break;

		case DISP_CHANGE_BADFLAGS:
			ConPrint("An invalid set of flags was passed in.\n");
			break;

		case DISP_CHANGE_FAILED:
			ConPrint("The display driver failed the specified graphics mode.\n");
			break;

		case DISP_CHANGE_BADMODE:
		case DISP_CHANGE_BADPARAM:
			ConPrint("The graphics mode is not supported.\n");
			break;

		case DISP_CHANGE_NOTUPDATED:
			ConPrint("Unable to write settings to the registry.\n");
			break;

		default:
			ConPrint("Unknown error: %d\n", rv);
			break;

		}
		return false;
	}

	if(::ChangeDisplaySettings(&m_devmodes[best_mode], CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		ConPrint("GL:GoFull:ChangeDisplaySettings not caught by test!!\n");
		return false;
	}

	//Record our changes
	g_rInfo.rflags|= RFLAG_FULLSCREEN;
	g_rInfo.width  = m_devmodes[best_mode].dmPelsWidth;
	g_rInfo.height = m_devmodes[best_mode].dmPelsHeight;
	g_rInfo.bpp	   = m_devmodes[best_mode].dmBitsPerPel;

	// put the window so the client area matches the size of the entire screen
	
	// calculate the size of window we need
	RECT wrect;
	wrect.left = wrect.top = 0;
	wrect.right = g_rInfo.width;
	wrect.bottom = g_rInfo.height;

	::AdjustWindowRect(&wrect, 
					   WS_CAPTION,
					   FALSE);

	wrect.left < 0 ? wrect.right -= wrect.left : wrect.right += wrect.left;
	wrect.top < 0 ? wrect.bottom -= wrect.top : wrect.bottom += wrect.top;

	::SetWindowPos(g_rInfo.hWnd,
				   HWND_TOPMOST,
				   wrect.left,
			       wrect.top,
				   wrect.right,
			       wrect.bottom,
			       0);
	return true;
}

/*
==========================================
Update Default window coords
==========================================
*/
void CGLUtil::SetWindowCoords(int wndX, int wndY)
{
	//Dont bother with initial co-ords
	if(!m_bInitialized)
		return;
	m_cWndX.Set(wndX);
	m_cWndY.Set(wndY);
}

/*
==========================================
Resize the Window
==========================================
*/
void CGLUtil::Resize()
{
	RECT crect;
	GetClientRect(g_rInfo.hWnd, &crect);
	g_rInfo.width  = crect.right - crect.left;
	g_rInfo.height = crect.bottom - crect.top;

	_wglMakeCurrent(g_rInfo.hDC, g_rInfo.hRC);
	glViewport(0, 0, g_rInfo.width, g_rInfo.height);
}

/*
==========================================
Updates display settings
==========================================
*/
bool CGLUtil::UpdateDisplaySettings(unsigned int width, 
									unsigned int height, 
									unsigned int bpp, 
									bool fullscreen)
{
	//Shutdown openGL first
	Shutdown();

	// record old stats
	bool oldfull = g_rInfo.rflags & RFLAG_FULLSCREEN;
	uint oldwidth= g_rInfo.width;
	uint oldheight= g_rInfo.height;
	uint oldbpp	  = g_rInfo.bpp;

	g_rInfo.bpp		= bpp;
	g_rInfo.width	= width;
	g_rInfo.height	= height;

	if (fullscreen)
		g_rInfo.rflags |= RFLAG_FULLSCREEN;
	else
		g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

	if (!Init())
	{
		ConPrint("GL::UpdateDisplaySettings: Unable to change to new settings\n");

		// switch everythign back;
		if (oldfull)
			g_rInfo.rflags |= RFLAG_FULLSCREEN;
		else
			g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

		g_rInfo.bpp		= oldbpp;
		g_rInfo.width	= oldwidth;
		g_rInfo.height	= oldheight;

		Init();
		return false;
	}
	ConPrint("GL::UpdateDisplaySettings::Display change successful\n");
	return true;
}


/*
==========================================
Setup Pixel Format for GL intialization
==========================================
*/
bool CGLUtil::SetupPixelFormat()
{
	PIXELFORMATDESCRIPTOR pfd = 
	{
		sizeof( PIXELFORMATDESCRIPTOR ),
		1,					// version
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		g_rInfo.bpp,					// bit depth
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,//g_rInfo.zdepth,                 // 16-bit depth buffer
		0, //g_rInfo.stencil,                 // no stencil buffer
		0,                  // no aux buffers
		PFD_MAIN_PLANE,		/* main layer */
		0,	
		0, 0, 0
	};

	int  selected_pf=0;
	if (!(selected_pf = _ChoosePixelFormat(g_rInfo.hDC, &pfd)))
	{
		ConPrint("GL::SetupPixelFormat:Couldn't find acceptable pixel format\n");
		return false;
	}

	if (!_SetPixelFormat(g_rInfo.hDC, selected_pf, &pfd))
	{
		ConPrint("GL::SetupPixelFormat::Couldn't set pixel format\n");
		return false;
	}

	// record what was selected
	g_rInfo.bpp	   = pfd.cColorBits;
	g_rInfo.zdepth  = pfd.cDepthBits;
	g_rInfo.stencil = pfd.cStencilBits;

	_DescribePixelFormat(g_rInfo.hDC, selected_pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ConPrint("CGLUtil::SetupPixelFormat:Set Pixel Format:\nBit Depth: %d\nZ Depth: %d\nStencil Depth: %d\n",
			  pfd.cColorBits, pfd.cDepthBits, pfd.cStencilBits);
	return true;
}

