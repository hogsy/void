
#include "Standard.h"
#include "gl_rast.h"



/*
=======================================
Constructor 
=======================================
*/
COpenGLRast::COpenGLRast()
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
		ComPrintf("GL::Unable to load opengl dll\n");
		m_loadeddriver = false;
		return;
	}
	ComPrintf("GL:Loaded GL driver: %s\n", m_gldriver);
	m_loadeddriver = true;
}


/*
==========================================
Destructor
==========================================
*/
COpenGLRast::~COpenGLRast()
{
	if(m_devmodes)
		delete [] m_devmodes;
	m_devmodes = NULL;

	// unload the driver
	OpenGLUnInit();

	// why the hell does this crash when quitting from fullscreen ?
	::ChangeDisplaySettings(NULL, 0);
	ComPrintf("GL::Final Shutdown OK\n");
}


/*
==========================================
Init
==========================================
*/
bool COpenGLRast::Init()
{
	if (!m_loadeddriver)
		return false;


	m_bInitialized = true;


	ComPrintf("CGLUtil::Init:Res: %d %d\n",g_rInfo.width, g_rInfo.height);
	ComPrintf("CGLUtil::Init:Pos: %d %d\n",m_cWndX.ival,m_cWndY.ival);


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
		GoFull();
	else
		GoWindowed();

	//Get Pixel Format
	hDC = ::GetDC(g_rInfo.hWnd);

	if (!SetupPixelFormat())
	{
		ComPrintf("GL::Init: Failed to set PixelFormat\n");
		return false;
	}

	//Finally create GL context
	hRC = _wglCreateContext(hDC);
	_wglMakeCurrent(hDC, hRC);

	//Get GL Extentions
	OpenGLGetExtensions();


	//Check for GL flags
	ComPrintf("\nGL_VENDOR: %s\n",glGetString(GL_VENDOR));
	ComPrintf("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
	ComPrintf("GL_VERSION: %s\n", glGetString(GL_VERSION));


	const char * ext = (const char*)glGetString(GL_EXTENSIONS);
	if(!ext)
		return true;

	ComPrintf("GL_EXTENSIONS:\n");
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
			ComPrintf("%s\n", start);

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
Shutdown
==========================================
*/
bool COpenGLRast::Shutdown()
{

	//Update Win Pos
	RECT rect;
	if(GetWindowRect(g_rInfo.hWnd, &rect))
	{
		if(rect.left < 40)	rect.left= 40;
		if(rect.top  < 20)	rect.top = 20;

		SetWindowCoords(rect.left, rect.top);
	}

	_wglMakeCurrent(NULL, NULL);
	_wglDeleteContext(hRC);

	::ReleaseDC(g_rInfo.hWnd, hDC);


	g_rInfo.ready = false;
	return true;
}


/*
==========================================
Enumerate Display modes
==========================================
*/
void COpenGLRast::EnumDisplayModes()
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
Change to Windowed Mode
==========================================
*/
bool COpenGLRast::GoWindowed(void)
{
	// make sure we have our regular desktop resolution
	::ChangeDisplaySettings(NULL, 0);

	//3dfx 3d only card. default to fullscreen mode
	g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

	RECT wrect;
	wrect.left = m_cWndX.ival;
	wrect.top = m_cWndY.ival;
	wrect.right = m_cWndX.ival + g_rInfo.width;
	wrect.bottom= m_cWndY.ival + g_rInfo.height;
	
	//Adjusts Client Size
	::AdjustWindowRect(&wrect, 
					   WS_CAPTION,
					   FALSE);

	int width = wrect.right - wrect.left;
	int height = wrect.bottom - wrect.top;

	::SetWindowPos(g_rInfo.hWnd,
				   HWND_TOP,
				   wrect.left,
				   wrect.top,
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
bool COpenGLRast::GoFull(void)
{
	int width = g_rInfo.width;
	int height= g_rInfo.height;
	int bpp   = g_rInfo.bpp;


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

	ComPrintf("GL::GoFull:Looking for %d x %d x %d\n", width, height, bpp);
	ComPrintf("GL::GoFull:Changing Display to %d x %d x %d\n", 
				m_devmodes[best_mode].dmPelsWidth,
				m_devmodes[best_mode].dmPelsHeight,
				m_devmodes[best_mode].dmBitsPerPel);

	//try and change the mode
	LONG rv;
    if ((rv = ::ChangeDisplaySettings(&m_devmodes[best_mode], CDS_TEST)) != DISP_CHANGE_SUCCESSFUL)
	{
		ComPrintf("GL:GoFull::Change to Fullscreen mode failed: ");
		switch(rv)
		{
		case DISP_CHANGE_RESTART:
			ComPrintf("The computer must be restarted in order for the graphics mode to work.\n");
			break;

		case DISP_CHANGE_BADFLAGS:
			ComPrintf("An invalid set of flags was passed in.\n");
			break;

		case DISP_CHANGE_FAILED:
			ComPrintf("The display driver failed the specified graphics mode.\n");
			break;

		case DISP_CHANGE_BADMODE:
		case DISP_CHANGE_BADPARAM:
			ComPrintf("The graphics mode is not supported.\n");
			break;

		case DISP_CHANGE_NOTUPDATED:
			ComPrintf("Unable to write settings to the registry.\n");
			break;

		default:
			ComPrintf("Unknown error: %d\n", rv);
			break;

		}
		return false;
	}

	if(::ChangeDisplaySettings(&m_devmodes[best_mode], CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
	{
		ComPrintf("GL:GoFull:ChangeDisplaySettings not caught by test!!\n");
		return false;
	}

	//Record our changes
 	g_rInfo.rflags|= RFLAG_FULLSCREEN;
	g_rInfo.width  = m_devmodes[best_mode].dmPelsWidth;
	g_rInfo.height = m_devmodes[best_mode].dmPelsHeight;
	g_rInfo.bpp    = m_devmodes[best_mode].dmBitsPerPel;

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
void COpenGLRast::SetWindowCoords(int wndX, int wndY)
{
	//Dont bother with initial co-ords
	if(!m_bInitialized || (g_rInfo.rflags&RFLAG_FULLSCREEN))
		return;
	m_cWndX.Set(wndX);
	m_cWndY.Set(wndY);
}

/*
==========================================
Resize the Window
==========================================
*/
void COpenGLRast::Resize()
{
	RECT crect;
	GetClientRect(g_rInfo.hWnd, &crect);
	g_rInfo.width  = crect.right - crect.left;
	g_rInfo.height = crect.bottom - crect.top;

	_wglMakeCurrent(hDC, hRC);
	glViewport(0, 0, g_rInfo.width, g_rInfo.height);
}

/*
==========================================
Updates display settings
==========================================
*/
bool COpenGLRast::UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)
{
	_wglMakeCurrent(hDC, hRC);

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
		ComPrintf("GL::UpdateDisplaySettings: Unable to change to new settings\n");

		// switch everythign back;
		if (oldfull)
			g_rInfo.rflags |= RFLAG_FULLSCREEN;
		else
			g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

		g_rInfo.bpp	= oldbpp;
		g_rInfo.width	= oldwidth;
		g_rInfo.height	= oldheight;

		Init();
		return false;
	}
	ComPrintf("GL::UpdateDisplaySettings::Display change successful\n");
	return true;
}


/*
==========================================
Setup Pixel Format for GL intialization
==========================================
*/
bool COpenGLRast::SetupPixelFormat()
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
	if (!(selected_pf = _ChoosePixelFormat(hDC, &pfd)))
	{
		ComPrintf("GL::SetupPixelFormat:Couldn't find acceptable pixel format\n");
		return false;
	}

	if (!_SetPixelFormat(hDC, selected_pf, &pfd))
	{
		ComPrintf("GL::SetupPixelFormat::Couldn't set pixel format\n");
		return false;
	}

	// record what was selected
	g_rInfo.bpp	 = pfd.cColorBits;
	g_rInfo.zdepth  = pfd.cDepthBits;
	g_rInfo.stencil = pfd.cStencilBits;

	_DescribePixelFormat(hDC, selected_pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ComPrintf("CGLUtil::SetupPixelFormat:Set Pixel Format:\nBit Depth: %d\nZ Depth: %d\nStencil Depth: %d\n",
			  pfd.cColorBits, pfd.cDepthBits, pfd.cStencilBits);
	return true;
}






//=========================================================================================================================
// OpenGL implementation of drawing functions
//=========================================================================================================================



void COpenGLRast::DepthFunc(EDepthFunc func)
{
	switch (func)
	{
	case VRAST_DEPTH_NONE:
		glDisable(GL_DEPTH_TEST);
		break;

	case VRAST_DEPTH_FILL:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		break;

	case VRAST_DEPTH_LEQUAL:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		break;
	}
}

void COpenGLRast::DepthWrite(bool write)
{
	if (write)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void COpenGLRast::BlendFunc(ESourceBlend src, EDestBlend dest)
{
	int source = 0;
	int destination = 0;

	switch (src)
	{
	case VRAST_SRC_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_SRC_BLEND_ZERO:
		source = GL_ZERO;
		break;
	case VRAST_SRC_BLEND_SRC_ALPHA:
		source = GL_SRC_ALPHA;
		break;
	}

	switch (dest)
	{
	case VRAST_DEST_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_DEST_BLEND_SRC_COLOR:
		destination = GL_SRC_COLOR;
		break;
	case VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA:
		destination = GL_ONE_MINUS_SRC_ALPHA;
		break;
	}

	glEnable(GL_BLEND);
	glBlendFunc(source, destination);
}


/*
========
TextureBinInit
========
*/
int COpenGLRast::TextureBinInit(int num)
{
	for (int i=0; i<MAX_TEXTURE_BINS; i++)
	{
		if (mTexBins[i].num == -1)
		{
			mTexBins[i].num = num;
			mTexBins[i].glnames = new GLuint[num];
			if (!mTexBins[i].glnames)
				FError("not enough mem for gl names");

			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glGenTextures(mTexBins[i].num, mTexBins[i].glnames);
			return i;
		}
	}

	Error("out of texture bins in vgl\n");
	return -1;
}


/*
========
TextureBinDestroy
========
*/
void COpenGLRast::TextureBinDestroy(int bin)
{
	if ((bin < 0) || (bin > MAX_TEXTURE_BINS) || (mTexBins[bin].num == -1))
	{
		Error("destroying non-existant texture bin!");
		return;
	}

	glDeleteTextures(mTexBins[bin].num, mTexBins[bin].glnames);
	delete mTexBins[bin].glnames;
	mTexBins[bin].glnames = NULL;
	mTexBins[bin].num = -1;
}



void COpenGLRast::TextureSet(int bin, int texnum)
{

	glBindTexture(GL_TEXTURE_2D, mTexBins[bin].glnames[texnum]);
}

void COpenGLRast::TextureLoad(int bin, int num, const tex_load_t *texdata)
{
	glBindTexture(GL_TEXTURE_2D, mTexBins[bin].glnames[num]);

	// clamping
	if (texdata->clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// mipmapping
	if (texdata->mipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}


	int ext_format, int_format;
	if (texdata->format == IMG_RGB)
	{
		ext_format = GL_RGB;
		int_format = GL_RGB8;
	}
	else
	{
		ext_format = GL_RGBA;
		int_format = GL_RGBA8;
	}

	int w = texdata->width;
	int h = texdata->height;

	if (texdata->mipmap)
	{
		for (int m=texdata->mipmaps-1; m>=0; m--)
		{
			glTexImage2D(GL_TEXTURE_2D,
					 texdata->mipmaps - m - 1,
					 GL_RGB, //int_format,
					 w,
					 h,
					 0,
					 GL_RGB, //ext_format,
					 GL_UNSIGNED_BYTE,
					 texdata->mipdata[m]);

			w /= 2;
			h /= 2;
			if (w==0) w=1;
			if (h==0) h=1;
		}
	}

	else
	{
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 int_format,
					 w,
					 h,
					 0,
					 ext_format,
					 GL_UNSIGNED_BYTE,
					 texdata->mipdata[texdata->mipmaps - 1]);
	}
}


/*
========
Matrix*
========
*/
void COpenGLRast::MatrixReset(void)
{
	glLoadIdentity();
}
void COpenGLRast::MatrixRotateX(float degrees)
{
	glRotatef(degrees, 1, 0, 0);
}
void COpenGLRast::MatrixRotateY(float degrees)
{
	glRotatef(degrees, 0, 1, 0);
}
void COpenGLRast::MatrixRotateZ(float degrees)
{
	glRotatef(degrees, 0, 0, 1);
}


void COpenGLRast::MatrixTranslate(vector_t &dir)
{
	glTranslatef(-dir.x, -dir.z, dir.y);
}


void COpenGLRast::MatrixScale(vector_t &factors)
{
}



void COpenGLRast::MatrixPush(void)
{
	glPushMatrix();
}

void COpenGLRast::MatrixPop(void)
{
	glPopMatrix();
}



/*
========
Poly*
========
*/
void COpenGLRast::PolyStart(EPolyType type)
{
	switch (type)
	{
	case VRAST_TRIANGLE_FAN:
		glBegin(GL_TRIANGLE_FAN);
		break;

	case VRAST_TRIANGLE_STRIP:
		glBegin(GL_TRIANGLE_STRIP);
		break;

	case VRAST_QUADS:
		glBegin(GL_QUADS);
		break;
	}
}


/*
========
PolyEnd
========
*/
void COpenGLRast::PolyEnd(void)
{
	glEnd();
}

void COpenGLRast::PolyVertexf(vector_t &vert)
{
	glVertex3f(vert.x, vert.z, -vert.y);
}
void COpenGLRast::PolyVertexi(int x, int y)
{
	glVertex2i(x, y);
}
void COpenGLRast::PolyTexCoord(float s, float t)
{
	glTexCoord2f(s, t);
}
void COpenGLRast::PolyColor3f(float r, float g, float b)
{
	glColor3f(r, g, b);
}
void COpenGLRast::PolyColor4f(float r, float g, float b, float a)
{
	glColor4f(r, g, b, a);
}


/*
========
ClearBuffers
========
*/
void COpenGLRast::ClearBuffers(int buffers)
{
	// should be first call of the frame - make current for all other calls
	_wglMakeCurrent(hDC, hRC);

	glClearColor(0, 0, 1, 1);
	int b = 0;

	if (buffers & VRAST_COLOR_BUFFER)
		b |= GL_COLOR_BUFFER_BIT;
	if (buffers & VRAST_DEPTH_BUFFER)
		b |= GL_DEPTH_BUFFER_BIT;
	if (buffers & VRAST_STENCIL_BUFFER)
		b |= GL_STENCIL_BUFFER_BIT;

	glClear(b);
}


/*
========
ProjectionMode
========
*/
void COpenGLRast::ProjectionMode(EProjectionMode mode)
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	float x;
	float z;

	switch (mode)
	{
	case VRAST_PERSPECTIVE:

		// FIXME - access fov here
		x = (float) tan(90.0f*(PI/180) * 0.5f);
		z = x * 0.75f;						// always render in a 3:4 aspect ratio
		glFrustum(-x, x, -z, z, 1, 10000);
		break;

	case VRAST_ORTHO:
		glOrtho(0, g_rInfo.width, 0, g_rInfo.height, -1, 1);
		break;
	}

	glMatrixMode(GL_MODELVIEW);
}


void COpenGLRast::ReportErrors(void)
{
	int err = glGetError();

	if (err != GL_NO_ERROR)
		ComPrintf("gl error %d\n", err);
}


/*
========
ClearBuffers
========
*/
void COpenGLRast::FrameEnd(void)
{
	glFlush();
	_SwapBuffers(hDC);
}


void COpenGLRast::ScreenShot(unsigned char *dest)
{
}


/*
========
SetVidSynch - assumes that it can be done
========
*/
void COpenGLRast::SetVidSynch(int v)
{
	wglSwapIntervalEXT(v);
}