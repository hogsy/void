
#include <windows.h>
#include "gl.h"

#ifdef DYNAMIC_GL

#include "3dfx/glide.h"		//3dfx card presense test

// gl* funcs
//====================================================================================
GLBEGIN					glBegin				= NULL;
GLBINDTEXTURE			glBindTexture		= NULL;
GLBLENDFUNC				glBlendFunc			= NULL;
GLCLEAR					glClear				= NULL;
GLCLEARCOLOR			glClearColor		= NULL;
GLCOLOR3F				glColor3f			= NULL;
GLCOLOR4F				glColor4f			= NULL;
GLCOLORPOINTER			glColorPointer		= NULL;
GLCULLFACE				glCullFace			= NULL;
GLDELETETEXTURES		glDeleteTextures	= NULL;
GLDEPTHFUNC				glDepthFunc			= NULL;
GLDEPTHMASK				glDepthMask			= NULL;
GLDISABLE				glDisable			= NULL;
GLDISABLECLIENTSTATE	glDisableClientState= NULL;
GLDRAWELEMENTS			glDrawElements		= NULL;
GLENABLE				glEnable			= NULL;
GLENABLECLIENTSTATE		glEnableClientState	= NULL;
GLEND					glEnd				= NULL;
GLFLUSH					glFlush				= NULL;
GLFRONTFACE				glFrontFace			= NULL;
GLFRUSTUM				glFrustum			= NULL;
GLGENTEXTURES			glGenTextures		= NULL;
GLGETERROR				glGetError			= NULL;
GLGETINTEGERV			glGetIntegerv		= NULL;
GLGETSTRING				glGetString			= NULL;
GLHINT					glHint				= NULL;
GLLOADIDENTITY			glLoadIdentity		= NULL;
GLMATRIXMODE			glMatrixMode		= NULL;
GLORTHO					glOrtho				= NULL;
GLPOPMATRIX				glPopMatrix			= NULL;
GLPUSHMATRIX			glPushMatrix		= NULL;
GLREADPIXELS			glReadPixels		= NULL;
GLROTATEF				glRotatef			= NULL;
GLSCALEF				glScalef			= NULL;
GLTEXCOORD2F			glTexCoord2f		= NULL;
GLTEXCOORDPOINTER		glTexCoordPointer	= NULL;
GLTEXENVF				glTexEnvf			= NULL;
GLTEXIMAGE2D			glTexImage2D		= NULL;
GLTEXPARAMETERI			glTexParameteri		= NULL;
GLTRANSLATEF			glTranslatef		= NULL;
GLVERTEX2F				glVertex2f			= NULL;
GLVERTEX2I				glVertex2i			= NULL;
GLVERTEX3F				glVertex3f			= NULL;
GLVERTEXPOINTER			glVertexPointer		= NULL;
GLVIEWPORT				glViewport			= NULL;

#endif

// gl extensions
//====================================================================================
GLMULTITEXCOORD2FARB	glMultiTexCoord2fARB	= NULL;
GLACTIVETEXTUREARB		glActiveTextureARB		= NULL;
WGLSWAPINTERVALEXT		wglSwapIntervalEXT		= NULL;


// wgl* functions
//====================================================================================
WGLCREATECONTEXT	_wglCreateContext	= NULL;
WGLDELETECONTEXT	_wglDeleteContext	= NULL;
WGLGETPROCADDRESS	_wglGetProcAddress	= NULL;
WGLMAKECURRENT		_wglMakeCurrent		= NULL;


// other gdi funcs
//====================================================================================
SWAPBUFFERS				fnSwapBuffers			= NULL;
SETPIXELFORMAT			fnSetPixelFormat		= NULL;
DESCRIBEPIXELFORMAT		fnDescribePixelFormat	= NULL;
CHOOSEPIXELFORMAT		fnChoosePixelFormat		= NULL;


//====================================================================================


/*
==========================================
List of FULL openGL ICDs
==========================================
*/

static bool		 bypassGDI = false;
static HINSTANCE openglInst = NULL;



#ifdef DYNAMIC_GL

const char * openglICDs[] =
{
	SZ_NVIDIA_GLDRIVER,			
	SZ_NVIDIA_GLDRIVER_NT,		
	SZ_MATROX_G200_GLDRIVER,	
	0
};



//====================================================================================
//Glide Func Prototypes

BOOL (__stdcall* GLIDEFUN_grSstQueryBoards) (GrHwConfiguration* hwconfig);
void (__stdcall* GLIDEFUN_grGlideInit) (void);
void (__stdcall* GLIDEFUN_grGlideShutdown) (void);
BOOL (__stdcall* GLIDEFUN_grSstQueryHardware) (GrHwConfiguration* hwconfig);


/*
==========================================
Returns the proper 3dfx driver string
or NULL if there is no card
==========================================
*/

const char * Get3dfxOpenGLDriver()
{
	// load glide
	HMODULE hmGlide = LoadLibrary(SZ_GLIDEDRIVER);

	if (hmGlide)
	{
		// Glide2x driver dll found

		// get functions
		GLIDEFUN_grSstQueryBoards = (BOOL (__stdcall*)(GrHwConfiguration* hwconfig)) GetProcAddress(hmGlide, "_grSstQueryBoards@4");
		GLIDEFUN_grGlideInit = (void (__stdcall*)(void)) GetProcAddress(hmGlide, "_grGlideInit@0");
		GLIDEFUN_grGlideShutdown = (void (__stdcall*)(void)) GetProcAddress(hmGlide, "_grGlideShutdown@0");
		GLIDEFUN_grSstQueryHardware = (BOOL (__stdcall*)(GrHwConfiguration* hwconfig)) GetProcAddress(hmGlide, "_grSstQueryHardware@4");

		if ((GLIDEFUN_grSstQueryBoards) &&
			(GLIDEFUN_grGlideInit) &&
			(GLIDEFUN_grGlideShutdown) &&
			(GLIDEFUN_grSstQueryHardware))
		{
			GrHwConfiguration GlideHwConfig;

			GLIDEFUN_grSstQueryBoards(&GlideHwConfig);

			//Query boards
			if (GlideHwConfig.num_sst > 0)
			{
				// hardware present; we can initialise Glide
				GLIDEFUN_grGlideInit();

				// fill out hardware structure properly
				GLIDEFUN_grSstQueryHardware(&GlideHwConfig);

				// search for a Voodoo 1, Rush or 2 (standalone) type
				// in data for all boards found
				bool StandaloneCardPresent = false;

				for (int i=0; ((i<GlideHwConfig.num_sst) && (!StandaloneCardPresent)); i++)
				{
					GrSstType type = (GlideHwConfig.SSTs[i]).type;
					int fbiRev = 0;

					switch(type)
					{
						case GR_SSTTYPE_VOODOO:
							fbiRev = (((GlideHwConfig.SSTs[i]).sstBoard).VoodooConfig).fbiRev;
							break;

						case GR_SSTTYPE_Voodoo2:
							fbiRev = (((GlideHwConfig.SSTs[i]).sstBoard).Voodoo2Config).fbiRev;
							break;

						case GR_SSTTYPE_SST96:
						case GR_SSTTYPE_AT3D:
						default:
							break;
					}

					if ((type == GR_SSTTYPE_VOODOO) || // Voodoo / Banshee / Voodoo 3?
						(type == GR_SSTTYPE_Voodoo2)) // Voodoo 2
					{
						if (fbiRev < 0x1000) // Banshee / Voodoo 3 has fbiRev > 0x1000
						{
							// standalone card type Voodoo 1 / 2
							StandaloneCardPresent = true;
						}
					}
					else if (type == GR_SSTTYPE_SST96) // Voodoo Rush
				 	{
					 	// standalone card type Voodoo Rush
					 	StandaloneCardPresent = true;
				 	}
				}

				// shut down glide
				GLIDEFUN_grGlideShutdown();

				FreeLibrary(hmGlide);
				if(StandaloneCardPresent)
					return SZ_3DFX_3DONLY_GLDRIVER;
				else
					return SZ_3DFX_COMBO_GLDRIVER;
			}
			else
			{
				FreeLibrary(hmGlide);
				return 0;
			}
		}
		else
		{
			FreeLibrary(hmGlide);
			return 0;
		}
	}
	return 0;
}

#endif

/*
==========================================
OpenGLFindDriver
==========================================
*/
void OpenGLFindDriver(char *openglLib)
{
#ifdef DYNAMIC_GL

	//Check for 3dfx hardware first
	const char * gl3dfxdriver = Get3dfxOpenGLDriver();
	if(gl3dfxdriver)
	{
		strcpy(openglLib,gl3dfxdriver);
		return;
	}

	const char win9xopenglkey[] = "Software\\Microsoft\\Windows\\CurrentVersion\\OpenGLdrivers\\";
	
	char regkeyval[MAX_PATH] ="";
	unsigned long regkeyvallen=sizeof(regkeyval);
	
	DWORD    dwcSubKeys;               // Number of sub keys.
	DWORD    dwcMaxSubKey;             // Longest sub key size.
	DWORD    dwcValues;                // Number of values for this key.
	DWORD    dwcMaxValueName;          // Longest Value name.
	DWORD    dwcMaxValueData;          // Longest Value data.
	BYTE  szdata[128];
	DWORD dwsize = sizeof(szdata);
	DWORD dwtype;

	long err=0;
	HKEY hogl;
	
	
	openglLib[0]=0;

	err = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
					   win9xopenglkey,
					   0,
                       KEY_ENUMERATE_SUB_KEYS | KEY_EXECUTE | KEY_QUERY_VALUE,
					   &hogl);

	//try for WinNT/Win2l
	if(err != ERROR_SUCCESS)
	{
	}
						
	if(err != ERROR_SUCCESS)
	{
		strcpy(openglLib, "opengl32.dll");
		return;
	}

	err = RegQueryInfoKey(	hogl,				// handle to key to query
							0,					// address of buffer for class string
							0,					// address of size of class string buffer
							0,					// reserved
							&dwcSubKeys,        // Number of sub keys.
							&dwcMaxSubKey,      // Longest sub key size.
							0,					// Longest class string.
							&dwcValues,         // Number of values for this key.
							&dwcMaxValueName,   // Longest Value name.
							&dwcMaxValueData,   // Longest Value data.
							0,					// Security descriptor.
							0);					// Last write time.

	if(err != ERROR_SUCCESS)
	{

		LPVOID lpMsgBuf;
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
						NULL,
						GetLastError(),
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
						(LPTSTR) &lpMsgBuf,
						0,
						NULL );
		
		// Display the string.
		MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Void Renderer Error", MB_OK);
		// Free the buffer.
		LocalFree( lpMsgBuf );
		MessageBox(0,"OpenGLFindDriver::Couldnt query regkey", "Void Renderer Error", MB_OK); 
		strcpy(openglLib, "opengl32.dll");
		return;
	}

    if (dwcValues)
	{
       for (unsigned int j = 0, err = ERROR_SUCCESS; j < dwcValues; j++)
       {
            err = RegEnumValue ( hogl, 
								 j, 
								 regkeyval,
                                 &regkeyvallen,
                                 NULL,
                                 &dwtype,               
                                 szdata,               
                                 &dwsize);              

			if(err != ERROR_SUCCESS &&
			   err != ERROR_INSUFFICIENT_BUFFER)
			{
				strcpy(openglLib, "opengl32.dll");
				return;
			}

			char message[128];
			strcpy(message,regkeyval);
			if(dwsize && dwtype == REG_SZ)
			{
				strcat(message,":");
				strcat(message,(char*)szdata);
			}
       }
	}


	for(int i=0;openglICDs[i];i++)
	{
		if(!strcmp((char*)szdata,openglICDs[i]))
		{	strcpy(openglLib, "opengl32.dll");
		}
	}

  	if (!*openglLib) 
	{
		if((_stricmp(regkeyval,"3dfx") == 0) ||
		   (_stricmp((char *)szdata,"3dfxvgl.dll")==0))
			strcpy(openglLib, "3dfxvgl.dll");
		else
			strcpy(openglLib, "opengl32.dll");		//default
	}

#else
	strcpy(openglLib, "static");

#endif

}



/*
=============
OpenGLInit
=============
*/
int OpenGLInit(char *lib)
{
#ifdef DYNAMIC_GL

//	openglInst = LoadLibrary(lib); 
	openglInst = ::LoadLibraryEx(lib, NULL,0);

 
	if (openglInst == NULL) 
	{ 
		MessageBox(GetFocus(), "OpenGLInit::Could not open OpenGL Library", "Void Renderer Error", MB_OK); 
		openglInst = NULL; 
		return -1; 
	}

	if (_stricmp(lib, "opengl32.dll")) 
		bypassGDI = true;

// gl* funcs
	glBegin				= (GLBEGIN)				GetProcAddress(openglInst, "glBegin");
	glBindTexture		= (GLBINDTEXTURE)		GetProcAddress(openglInst, "glBindTexture");
	glBlendFunc			= (GLBLENDFUNC)			GetProcAddress(openglInst, "glBlendFunc");
	glClear				= (GLCLEAR)				GetProcAddress(openglInst, "glClear");
	glClearColor		= (GLCLEARCOLOR)		GetProcAddress(openglInst, "glClearColor");
	glColor3f			= (GLCOLOR3F)			GetProcAddress(openglInst, "glColor3f");
	glColor4f			= (GLCOLOR4F)			GetProcAddress(openglInst, "glColor4f");
	glColorPointer		= (GLCOLORPOINTER)		GetProcAddress(openglInst, "glColorPointer");
	glCullFace			= (GLCULLFACE)			GetProcAddress(openglInst, "glCullFace");
	glDeleteTextures	= (GLDELETETEXTURES)	GetProcAddress(openglInst, "glDeleteTextures");
	glDepthFunc			= (GLDEPTHFUNC)			GetProcAddress(openglInst, "glDepthFunc");
	glDepthMask			= (GLDEPTHMASK)			GetProcAddress(openglInst, "glDepthMask");
	glDisable			= (GLDISABLE)			GetProcAddress(openglInst, "glDisable");
	glDisableClientState= (GLDISABLECLIENTSTATE)GetProcAddress(openglInst, "glDisableClientState");
	glDrawElements		= (GLDRAWELEMENTS)		GetProcAddress(openglInst, "glDrawElements");
	glEnable			= (GLENABLE)			GetProcAddress(openglInst, "glEnable");
	glEnableClientState	= (GLENABLECLIENTSTATE)	GetProcAddress(openglInst, "glEnableClientState");
	glEnd				= (GLEND)				GetProcAddress(openglInst, "glEnd");
	glFlush				= (GLFLUSH)				GetProcAddress(openglInst, "glFlush");
	glFrontFace			= (GLFRONTFACE)			GetProcAddress(openglInst, "glFrontFace");
	glFrustum			= (GLFRUSTUM)			GetProcAddress(openglInst, "glFrustum");
	glGenTextures		= (GLGENTEXTURES)		GetProcAddress(openglInst, "glGenTextures");
	glGetError			= (GLGETERROR)			GetProcAddress(openglInst, "glGetError");
	glGetIntegerv		= (GLGETINTEGERV)		GetProcAddress(openglInst, "glGetIntegerv");
	glGetString			= (GLGETSTRING)			GetProcAddress(openglInst, "glGetString");
	glHint				= (GLHINT)				GetProcAddress(openglInst, "glHint");
	glLoadIdentity		= (GLLOADIDENTITY)		GetProcAddress(openglInst, "glLoadIdentity");
	glMatrixMode		= (GLMATRIXMODE)		GetProcAddress(openglInst, "glMatrixMode");
	glOrtho				= (GLORTHO)				GetProcAddress(openglInst, "glOrtho");
	glPopMatrix			= (GLPOPMATRIX)			GetProcAddress(openglInst, "glPopMatrix");
	glPushMatrix		= (GLPUSHMATRIX)		GetProcAddress(openglInst, "glPushMatrix");
	glReadPixels		= (GLREADPIXELS)		GetProcAddress(openglInst, "glReadPixels");
	glRotatef			= (GLROTATEF)			GetProcAddress(openglInst, "glRotatef");
	glScalef			= (GLSCALEF)			GetProcAddress(openglInst, "glScalef");
	glTexCoord2f		= (GLTEXCOORD2F)		GetProcAddress(openglInst, "glTexCoord2f");
	glTexCoordPointer	= (GLTEXCOORDPOINTER)	GetProcAddress(openglInst, "glTexCoordPointer");
	glTexEnvf			= (GLTEXENVF)			GetProcAddress(openglInst, "glTexEnvf");
	glTexImage2D		= (GLTEXIMAGE2D)		GetProcAddress(openglInst, "glTexImage2D");
	glTexParameteri		= (GLTEXPARAMETERI)		GetProcAddress(openglInst, "glTexParameteri");
	glTranslatef		= (GLTRANSLATEF)		GetProcAddress(openglInst, "glTranslatef");
	glVertex2f			= (GLVERTEX2F)			GetProcAddress(openglInst, "glVertex2f");
	glVertex2i			= (GLVERTEX2I)			GetProcAddress(openglInst, "glVertex2i");
	glVertex3f			= (GLVERTEX3F)			GetProcAddress(openglInst, "glVertex3f");
	glVertexPointer		= (GLVERTEXPOINTER)		GetProcAddress(openglInst, "glVertexPointer");
	glViewport			= (GLVIEWPORT)			GetProcAddress(openglInst, "glViewport");

	// wgl* functions
	_wglCreateContext	= (WGLCREATECONTEXT)	GetProcAddress(openglInst, "wglCreateContext");
	_wglDeleteContext	= (WGLDELETECONTEXT)	GetProcAddress(openglInst, "wglDeleteContext");
	_wglGetProcAddress	= (WGLGETPROCADDRESS)	GetProcAddress(openglInst, "wglGetProcAddress");
	_wglMakeCurrent		= (WGLMAKECURRENT)		GetProcAddress(openglInst, "wglMakeCurrent");

	// other gdi funcs
	fnSwapBuffers			= (SWAPBUFFERS)			GetProcAddress(openglInst, "wglSwapBuffers");
	fnSetPixelFormat		= (SETPIXELFORMAT)		GetProcAddress(openglInst, "wglSetPixelFormat");
	fnDescribePixelFormat	= (DESCRIBEPIXELFORMAT)	GetProcAddress(openglInst, "wglDescribePixelFormat");
	fnChoosePixelFormat		= (CHOOSEPIXELFORMAT)	GetProcAddress(openglInst, "wglChoosePixelFormat");

#else

	// wgl* functions
	_wglCreateContext	= wglCreateContext;
	_wglDeleteContext	= wglDeleteContext;
	_wglGetProcAddress	= wglGetProcAddress;
	_wglMakeCurrent		= wglMakeCurrent;

	// other gdi funcs
	fnSwapBuffers			= SwapBuffers;
	fnSetPixelFormat		= SetPixelFormat;
	fnDescribePixelFormat	= DescribePixelFormat;
	fnChoosePixelFormat		= ChoosePixelFormat;

#endif


	return 0;
}

/*
=============
OpenGLGetExtensions - must be separated cause the gl rendering context must be current
=============
*/
void OpenGLGetExtensions(void)
{
	glMultiTexCoord2fARB	= (GLMULTITEXCOORD2FARB)	_wglGetProcAddress("glMultiTexCoord2fARB");
	glActiveTextureARB		= (GLACTIVETEXTUREARB)		_wglGetProcAddress("glActiveTextureARB");
	wglSwapIntervalEXT		= (WGLSWAPINTERVALEXT)		_wglGetProcAddress("wglSwapIntervalEXT");
}


/*
=============
OpenGLUnInit
=============
*/
void OpenGLUnInit(void)
{
#ifdef DYNAMIC_GL

	if(openglInst)
		FreeLibrary(openglInst); 
	openglInst = NULL; 
	bypassGDI = false;
#endif
} 



//=====================================================================================================
// replacement gdi funcs


BOOL  WINAPI _SwapBuffers(HDC hdc)
{
	BOOL ret;

	// try gdi version
	if (!bypassGDI)
	{
		ret = SwapBuffers(hdc);
		if (ret)
			return ret;
	}

	//  if failed, always bypass from now on
//	bypassGDI = true;
	return fnSwapBuffers(hdc);
}


int WINAPI _DescribePixelFormat(HDC hdc, int iPixelFormat, UINT nBytes, LPPIXELFORMATDESCRIPTOR ppfd)
{
	int ret;

	// try gdi version
	if (!bypassGDI)
	{
		ret = DescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);
		if (ret)
			return ret;
	}

	//  if failed, always bypass from now on
//	bypassGDI = true;
	return fnDescribePixelFormat(hdc, iPixelFormat, nBytes, ppfd);
}


int WINAPI _SetPixelFormat(HDC hdc, int iPixelFormat, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
	int ret;

	// try gdi version
	if (!bypassGDI)
	{
		ret = SetPixelFormat(hdc, iPixelFormat, ppfd);
		if (ret)
			return ret;
	}

	//  if failed, always bypass from now on
//	bypassGDI = true;
	return fnSetPixelFormat(hdc, iPixelFormat, ppfd);
}


int WINAPI _ChoosePixelFormat(HDC hdc, CONST PIXELFORMATDESCRIPTOR *ppfd)
{
	int ret;

	// try gdi version
	if (!bypassGDI)
	{
		ret = ChoosePixelFormat(hdc, ppfd);
		if (ret)
			return ret;
	}

	//  if failed, always bypass from now on
//	bypassGDI = true;
	return fnChoosePixelFormat(hdc, ppfd);
}
