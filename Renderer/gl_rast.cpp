

#include "Standard.h"
#include "gl_rast.h"
//#include <gl/glu.h>

#include "Ren_exp.h"
extern CRenExp		  * g_pRenExp;

extern 	CVar * g_varGLExtensions;


/*
=======================================
Constructor 
=======================================
*/
COpenGLRast::COpenGLRast()
{
	m_CVAsupported = false;
	m_vsynchsupported = false;
	m_MultiSupported = false;

	m_nummodes = 0;
	m_devmodes = 0;

	glMultiTexCoord2fARB	= NULL;
	glActiveTextureARB		= NULL;
	glLockArraysEXT			= NULL;
	glUnlockArraysEXT		= NULL;
	wglSwapIntervalEXT		= NULL;

	//Enumerate the available display modes
	EnumDisplayModes();
}


/*
==========================================
Destructor
==========================================
*/
COpenGLRast::~COpenGLRast()
{
	if (m_bInitialized)
		Shutdown();

	if(m_devmodes)
		delete [] m_devmodes;
	m_devmodes = NULL;

	if (mMaxElements >= MAX_ELEMENTS)
		ComPrintf("**** mMaxElements = %d ****\n", mMaxElements);
	if (mMaxIndices >= MAX_INDICES)
		ComPrintf("**** mMaxIndices = %d ****\n", mMaxIndices);

	ComPrintf("GL::Final Shutdown OK\n");
}


/*
==========================================
Init
==========================================
*/
bool COpenGLRast::Init()
{
	// load the driver
	glsSetBehavior(glsGetBehavior() | GLS_BEHAVIOR_NEVER_LOAD_GLU);

	int unsigned num_drivers = glsGetNumberOfDrivers();
	ComPrintf("%d available OpenGL drivers\n", num_drivers);

	int driver = 0;
	if (g_varGLDriver->ival>0 && g_varGLDriver->ival<num_drivers)
		driver = g_varGLDriver->ival;


	gls_error GLSError;
	GLSError = glsLoadDriver(driver);
    if (GLSError != GLS_ERROR_OK)
	{
		Error("Couldnt load gl driver %d\n", driver);
		return false;
	}

	gls_driver_info dinfo;
	glsGetCurrentDriverInfo(&dinfo);


	ComPrintf("GL:Init GL driver: %s\n%s\n", dinfo.aDriverDescription,
											 dinfo.GLDriver.aDriverFilePath);



	ComPrintf("CGLUtil::Init:Res: %d %d\n",g_rInfo.width, g_rInfo.height);
	ComPrintf("CGLUtil::Init:Pos: %d %d\n",g_varWndX->ival,g_varWndY->ival);


	// change display before we do anything with gl
	if (g_pRenExp->m_varFull->bval)
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
	hRC = glsCreateContext(hDC);
	glsMakeCurrent(hDC, hRC);

	//Get GL Extentions
	GetExtensions();


	//Check for GL flags
	ComPrintf("\nGL_VENDOR: %s\n",glGetString(GL_VENDOR));
	ComPrintf("GL_RENDERER: %s\n",glGetString(GL_RENDERER));
	ComPrintf("GL_VERSION: %s\n", glGetString(GL_VERSION));


	const char * ext = (const char*)glGetString(GL_EXTENSIONS);
	if(!ext)
		return true;

//This is causing weird problems after you enter it into the console
//	g_varGLExtensions.ForceSet(ext);

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
				m_MultiSupported = true;

			else if (!strcmp(start, "WGL_EXT_swap_control"))
				m_vsynchsupported = true;

			else if (!strcmp(start, "GL_EXT_compiled_vertex_array"))
				m_CVAsupported = true;

			start = &ext2[i+1];
		}
	}
	delete [] ext2;

	// enable arrays
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, sizeof(rast_vertex_t), &mVerts[0].pos[0]);
	glNormalPointer(GL_FLOAT, sizeof(rast_vertex_t), &mVerts[0].norm[0]);
	glColorPointer(4, GL_UNSIGNED_BYTE, sizeof(rast_vertex_t), &mVerts[0].color);
	glTexCoordPointer(2, GL_FLOAT, sizeof(rast_vertex_t), &mVerts[0].tex1[0]);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	// get max lights
	glGetIntegerv(GL_MAX_LIGHTS, &mLightMax);

	// have material color follow face color
	glEnable(GL_COLOR_MATERIAL);

	m_bInitialized = true;
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool COpenGLRast::Shutdown()
{
	if (!hRC || !hDC || !m_bInitialized)
		return true;

	m_bInitialized = false;
	::ChangeDisplaySettings(NULL, 0);


	//Update Win Pos
	RECT rect;
	if(GetWindowRect(g_rInfo.hWnd, &rect))
	{
		if(rect.left < 40)	rect.left= 40;
		if(rect.top  < 20)	rect.top = 20;

		SetWindowCoords(rect.left, rect.top);
	}

	glsMakeCurrent(NULL, NULL);
	glsDeleteContext(hRC);

	// unload the driver
	glsUnloadDriver();

	::ReleaseDC(g_rInfo.hWnd, hDC);

	hRC = NULL;
	hDC = NULL;

	return true;
}


/*
==========================================
set all extension pointers
==========================================
*/
void COpenGLRast::GetExtensions()
{
	// ARB multitexture
	glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)		glsGetProcAddress("glMultiTexCoord2fARB");
	glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)		glsGetProcAddress("glActiveTextureARB");

	// CVA
	glLockArraysEXT			= (PFNGLLOCKARRAYSEXTPROC)			glsGetProcAddress("glLockArraysEXT");
	glUnlockArraysEXT		= (PFNGLUNLOCKARRAYSEXTPROC)		glsGetProcAddress("glUnlockArraysEXT");

	// vsynch
	wglSwapIntervalEXT		= (WGLSWAPINTERVALEXT)				glsGetProcAddress("wglSwapIntervalEXT");
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

	RECT wrect;
	wrect.left = g_varWndX->ival;
	wrect.top = g_varWndY->ival;
	wrect.right = g_varWndX->ival + g_rInfo.width;
	wrect.bottom= g_varWndY->ival + g_rInfo.height;
	
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
	int bpp   = g_pRenExp->m_varBpp->ival;


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
	g_rInfo.width  = m_devmodes[best_mode].dmPelsWidth;
	g_rInfo.height = m_devmodes[best_mode].dmPelsHeight;
	g_pRenExp->m_varBpp->ForceSet((int)m_devmodes[best_mode].dmBitsPerPel);

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
	// dont store if it's at (0,0) - most likely from fullscreen
//	if ((wndX>=0) && (wndX<=8) && (wndY>=0) && (wndY<=30))
//		return;

	//Dont bother with initial co-ords
	if(!m_bInitialized || g_pRenExp->m_varFull->bval)
		return;

	g_varWndX->Set(wndX);
	g_varWndY->Set(wndY);
}

/*
==========================================
Resize the Window
==========================================
*/
void COpenGLRast::Resize()
{
	if (!hDC || !hRC || !m_bInitialized)
		return;


	RECT crect;
	GetClientRect(g_rInfo.hWnd, &crect);
	g_rInfo.width  = crect.right - crect.left;
	g_rInfo.height = crect.bottom - crect.top;

	glsMakeCurrent(hDC, hRC);
	glViewport(0, 0, g_rInfo.width, g_rInfo.height);

}

/*
==========================================
Updates display settings
==========================================
*/
bool COpenGLRast::UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)
{
	if (!hDC || !hRC || !m_bInitialized)
		return false;

	glsMakeCurrent(hDC, hRC);


	//Shutdown openGL first
	Shutdown();

	// record old stats
	bool oldfull  =	g_pRenExp->m_varFull->bval;
	uint oldwidth = g_rInfo.width;
	uint oldheight= g_rInfo.height;
	uint oldbpp	  = g_pRenExp->m_varBpp->ival;

	g_pRenExp->m_varBpp->ForceSet(bpp);
	g_rInfo.width	= width;
	g_rInfo.height	= height;

	if (fullscreen)
		g_pRenExp->m_varFull->ForceSet("1");
	else
		g_pRenExp->m_varFull->ForceSet("0");

	if (!Init())
	{
		ComPrintf("GL::UpdateDisplaySettings: Unable to change to new settings\n");

		// switch everythign back;
		if (oldfull)
			g_pRenExp->m_varFull->ForceSet("1");
		else
			g_pRenExp->m_varFull->ForceSet("0");

		g_pRenExp->m_varBpp->ForceSet((int)oldbpp);
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
		g_pRenExp->m_varBpp->ival,					// bit depth
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
	if (!(selected_pf = ChoosePixelFormat(hDC, &pfd)))
	{
		ComPrintf("GL::SetupPixelFormat:Couldn't find acceptable pixel format\n");
		return false;
	}

	if (!SetPixelFormat(hDC, selected_pf, &pfd))
	{
		ComPrintf("GL::SetupPixelFormat::Couldn't set pixel format\n");
		return false;
	}

	// record what was selected
	g_pRenExp->m_varBpp->ForceSet(pfd.cColorBits);

	DescribePixelFormat(hDC, selected_pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);
	ComPrintf("CGLUtil::SetupPixelFormat:\nBit Depth: %d\nZ Depth: %d\nStencil Depth: %d\n",
			  pfd.cColorBits, pfd.cDepthBits, pfd.cStencilBits);
	return true;
}



/*
==========================================
SetFocus
==========================================
*/
void COpenGLRast::SetFocus()
{
	if (!m_bInitialized)
		return;

	if (!hDC || !hRC)
		ComPrintf("setting current without hDC or hRC!!!\n");
	glsMakeCurrent(hDC, hRC);
}



//=========================================================================================================================
// OpenGL implementation of drawing functions
//=========================================================================================================================



void COpenGLRast::DepthFunc(EDepthFunc func)
{
	mCurDepthFunc = func;

	switch (func)
	{
	case VRAST_DEPTH_NONE:
		glDisable(GL_DEPTH_TEST);
		return;
	case VRAST_DEPTH_ALWAYS:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_ALWAYS);
		return;
	case VRAST_DEPTH_LEQUAL:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);
		return;
	case VRAST_DEPTH_EQUAL:
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_EQUAL);
		return;
	}
}

void COpenGLRast::DepthWrite(bool write)
{
	mCurDepthWrite = write;

	if (write)
		glDepthMask(GL_TRUE);
	else
		glDepthMask(GL_FALSE);
}

void COpenGLRast::BlendFunc(ESourceBlend src, EDestBlend dest)
{
	int source = 0;
	int destination = 0;

	mCurSrcBlend = src;
	mCurDstBlend = dest;

	switch (src)
	{
	case VRAST_SRC_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_SRC_BLEND_ONE:
		source = GL_ONE;
		break;
	case VRAST_SRC_BLEND_ZERO:
		source = GL_ZERO;
		break;
	case VRAST_SRC_BLEND_SRC_ALPHA:
		source = GL_SRC_ALPHA;
		break;
	case VRAST_SRC_BLEND_DST_COLOR:
		source = GL_DST_COLOR;
		break;
	case VRAST_SRC_BLEND_INV_DST_COLOR:
		source = GL_ONE_MINUS_DST_COLOR;
		break;
	case VRAST_SRC_BLEND_INV_SRC_ALPHA:
		source = GL_ONE_MINUS_SRC_ALPHA;
		break;
	}

	switch (dest)
	{
	case VRAST_DST_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_DST_BLEND_ONE:
		destination = GL_ONE;
		break;
	case VRAST_DST_BLEND_ZERO:
		destination = GL_ZERO;
		break;
	case VRAST_DST_BLEND_SRC_COLOR:
		destination = GL_SRC_COLOR;
		break;
	case VRAST_DST_BLEND_INV_SRC_COLOR:
		destination = GL_ONE_MINUS_SRC_COLOR;
		break;
	case VRAST_DST_BLEND_SRC_ALPHA:
		destination = GL_SRC_ALPHA;
		break;
	case VRAST_DST_BLEND_INV_SRC_ALPHA:
		destination = GL_ONE_MINUS_SRC_ALPHA;
		break;
	}

	glEnable(GL_BLEND);
	glBlendFunc(source, destination);
}


void COpenGLRast::TextureSet(hTexture texnum)
{
	if (texnum == -1)
		return;
	glBindTexture(GL_TEXTURE_2D, m_glnames[texnum]);
}

void COpenGLRast::TextureClamp(bool clamp)
{
	if (clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}
}


void COpenGLRast::TextureLoad(hTexture index, const TextureData &texdata)
{
	glGenTextures(1, &m_glnames[index]);
	glBindTexture(GL_TEXTURE_2D, m_glnames[index]);

	// clamping
	if (texdata.bClamped)
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
	if (texdata.bMipMaps)
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
	if (texdata.format == IMG_RGB)
	{
		ext_format = GL_RGB;
		int_format = g_var32BitTextures->bval ? GL_RGB8 : GL_RGB5;
	}
	else
	{
		ext_format = GL_RGBA;
		int_format = g_var32BitTextures->bval ? GL_RGBA8 : GL_RGBA4;
	}

	int w = texdata.width;
	int h = texdata.height;

	if (texdata.bMipMaps)
	{
		for (int m=texdata.numMipMaps-1; m>=0; m--)
		{
			glTexImage2D(GL_TEXTURE_2D,
					 texdata.numMipMaps - m - 1,
					 int_format,
					 w,
					 h,
					 0,
					 ext_format,
					 GL_UNSIGNED_BYTE,
					 texdata.data[m]);

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
					 texdata.data[texdata.numMipMaps - 1]);
	}
}


void COpenGLRast::TextureUnLoad(hTexture index)
{
	glDeleteTextures(1, &m_glnames[index]);
}
	
	
/*
========
Matrix*
========
*/
void COpenGLRast::MatrixReset(void)
{
	Flush();
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


void COpenGLRast::MatrixTranslate(float x, float y, float z)
{
	glTranslatef(x, y, z);
}


void COpenGLRast::MatrixScale(vector_t &factors)
{
	glScalef(factors.x, factors.y, factors.z);
}



void COpenGLRast::MatrixPush(void)
{
	Flush();
	glPushMatrix();
}

void COpenGLRast::MatrixPop(void)
{
	Flush();
	glPopMatrix();
}



/*
========
PolyEnd
========
*/
void COpenGLRast::PolyDraw(void)
{
	glDrawElements(GL_TRIANGLES, mNumIndices, GL_UNSIGNED_SHORT, mIndices);
}


/*
========
ClearBuffers
========
*/
void COpenGLRast::ClearBuffers(int buffers)
{
	mTrisDrawn = 0;
	glClearColor(1, 1, 1, 1);
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
		x = (float) tan(g_varFov->ival*(PI/180) * 0.5f);
		z = x * 0.75f;						// always render in a 3:4 aspect ratio
		glFrustum(-x, x, -z, z, 1, 10000);

		// switch to +y = north, +z = up coordinate system
		MatrixRotateX(-90);
		MatrixRotateZ(90);
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

	if (err == GL_NO_ERROR)
		return;

	ComPrintf("gl error %d\n", err);
//	ComPrintf("gl error %s\n", gluErrorString(err));
}


/*
========
ClearBuffers
========
*/
void COpenGLRast::FrameEnd(void)
{
	// make sure everything's been drawn
	Flush();

	glFlush();
	SwapBuffers(hDC);

	// drop all the lights - have to be added every frame
	mLightNum = 0;
}


void COpenGLRast::ScreenShot(unsigned char *dest)
{
	glReadPixels(0, 0, g_rInfo.width, g_rInfo.height, GL_RGB, GL_UNSIGNED_BYTE, dest);
}


/*
========
SetVidSynch - assumes that it can be done
========
*/
void COpenGLRast::SetVidSynch(int v)
{
	if (m_vsynchsupported)
		wglSwapIntervalEXT(v);
}

/*
========
LockVerts - allows optimization of cva's
========
*/
void COpenGLRast::LockVerts(void)
{
	if (!m_CVAsupported)
		return;

	// only have vector data enabled when we lock
	glDisableClientState(GL_COLOR_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);

	glLockArraysEXT(0, mNumElements);

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
}


/*
========
UnLockVerts
========
*/
void COpenGLRast::UnLockVerts(void)
{
	if (m_CVAsupported)
		glUnlockArraysEXT();
}


/*
========
UnLockVerts
========
*/
void COpenGLRast::LightSet(bool enable)
{
	// dont need to do anything
	if (!enable && !mLighting)
		return;

	if (!enable && mLighting)
	{
		glDisable(GL_LIGHTING);
		mLighting = false;
		return;
	}

	if (enable && !mLighting)
		glEnable(GL_LIGHTING);

	for (int i=0; i<mLightMax; i++)
	{
		// don't have to use all available lights
		if ((i>=g_varNumLights->ival) || (i>mLightNum))
			glDisable(GL_LIGHT0 + i);

		else
		{
			glEnable(GL_LIGHT0 + i);
			
			float v[4];
			v[3] = 0;

			vector_t diff = mLights[i].origin - mLightOrigin;
			float dist = diff.Length();

			v[0] = diff.x;
			v[1] = diff.y;
			v[2] = diff.z;
			glLightfv(GL_LIGHT0+i, GL_POSITION, v);


			vector_t color(mLights[i].color);
			float scale = dist / mLights[i].rad;
			if (scale < 0) scale = 0;
			if (scale > 1) scale = 1;
			color.Scale(1-scale);
			if (i != 0)
				color.Scale(0);

			v[0] = color.x;
			v[1] = color.y;
			v[2] = color.z;
			glLightfv(GL_LIGHT0+i, GL_DIFFUSE, v);
			glLightfv(GL_LIGHT0+i, GL_SPECULAR, v);

		}
	}

	mLighting = true;
}


