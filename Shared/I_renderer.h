#ifndef INC_RENDERER_INTERFACE
#define INC_RENDERER_INTERFACE

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif

/*
======================================
Renderer flags
======================================
*/
#define RFLAG_FULLSCREEN	0x00000001
#define RFLAG_FULLBRIGHT	0x00000002
#define RFLAG_MULTITEXTURE	0x00000004
#define RFLAG_SWAP_CONTROL	0x00000008

/*
==========================================
Renderer info struct
==========================================
*/

typedef struct RenderInfo_t
{
	RenderInfo_t()
	{
		width = 640;
		height = 480;
		bpp = 16;
		zdepth = 16;
		stencil = 0;
		active = false;
		ready = false;
		rflags = 0;
	}

	//==========================================

	HWND	hWnd;		//Window information

	bool	active;		//in the window active?
	bool	ready;

	//==========================================
	//Renderering Information
	
	byte 	rflags;

	uint    width, height, bpp,
			zdepth,	stencil;

}RenderInfo_t;

/*
==========================================
Renderer Interface
==========================================
*/
struct I_ClientRenderer;
struct I_ConsoleRenderer;
struct I_HudRenderer;

class  CCamera;
class  CWorld;

struct I_Renderer
{
	//Startup/Shutdown
	virtual bool InitRenderer()=0;
	virtual bool Shutdown()=0;

	virtual void Draw(const CCamera * camera=0)=0;
	
	//Get other interfaces
	virtual I_ClientRenderer  * GetClient()=0;
	virtual I_ConsoleRenderer * GetConsole()=0;
	virtual I_HudRenderer	  * GetHud()=0;

	//Windowing
	virtual void MoveWindow(int x, int y) = 0;
	virtual void Resize() =0;
	virtual void ChangeDispSettings(uint width, uint height, uint bpp, 
									bool fullscreen)=0;
	//World
	virtual bool LoadWorld(CWorld *level) =0;
	virtual bool UnloadWorld() = 0;
};

/*
==========================================
Renderer Console Interface
==========================================
*/
struct I_ConsoleRenderer
{
	enum LineOffset
	{
		LINE_UP,
		LINE_DOWN,
		PAGE_UP,
		PAGE_DOWN,
		TOP,
		BOTTOM
	};

	enum LineColor
	{
		DEFAULT,
		WHITE,
		BLACK,
		RED,
		BLUE,
		GREEN,
		YELLOW
	};
	
	virtual void Toggle(bool down) = 0;
	virtual void ToggleFullscreen(bool full) = 0;
	virtual void MoveCurrentLine(LineOffset offset) = 0;
	virtual void SetStatusline(const char  *status_line, const int &len) = 0;
	virtual void AddLine(const char *line, LineColor color=DEFAULT, int size=0) = 0;
};


/*
================================================
The Void exe exports these to the Renderer
================================================
*/
struct I_FileReader;
struct I_Console;
struct I_HunkManager;

struct VoidExports
{
	VoidExports()  
	{ 
		pConsole = 0; pHunkManager = 0; 
		
		//Add more as needed
		pfnGetCurTime   = 0;
		pfnGetFrameTime = 0;
		pfnGetCurPath = 0;
		pfnCreateFileReader = 0;
		pfnSystemError = 0;
	}
	
	//Misc func points
	float (*pfnGetCurTime)();
	float (*pfnGetFrameTime)();
	const char * (*pfnGetCurPath)();
	I_FileReader * (*pfnCreateFileReader)(EFileMode mode);
	void  (*pfnSystemError)(const char *message);

	//Pointers to other Interfaces
	I_Console     * pConsole;
	I_HunkManager * pHunkManager;
};


//==========================================================================
//==========================================================================

RENDERER_API I_Renderer   * RENDERER_Create(VoidExports * pExp);
RENDERER_API RenderInfo_t * RENDERER_GetParms();
RENDERER_API void RENDERER_Free();

#endif

