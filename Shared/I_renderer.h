#ifndef INC_RENDERER_INTERFACE
#define INC_RENDERER_INTERFACE

#include "I_void.h"
#include "Com_vector.h"

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

	HWND		hWnd;		//Window information

	bool		active;		//in the window active?
	bool		ready;

	//==========================================
	//Renderering Information
	
	unsigned char	rflags;

	unsigned int    width, 
					height, 
					bpp,
					zdepth,
					stencil;
}RenderInfo_t;

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
======================================
The camera class.
keeps refs to client angle/position data. 
Client creates this locally when starting into a game

TODO, change this to subclass EntityState ?
======================================
*/
class CCamera
{
public:

	CCamera(vector_t & rorigin,
			vector_t & rangles,
			vector_t & rblend
//			,vector_t & rforward,
//			vector_t & rright,
			//vector_t & rup
			): origin(rorigin), angles(rangles), blend(rblend)
							 //,forward(rforward), right(rright), up(rup)
	{}
	
	~CCamera() {} 

	vector_t & origin;
	vector_t & angles;
	vector_t & blend;
	
/*	vector_t & forward;
	vector_t & right;
	vector_t & up;
*/
};


/*
==========================================
Renderer Interface
==========================================
*/
struct I_ClientRenderer;
class  CWorld;

struct I_Renderer
{
	//Startup/Shutdown
	virtual bool InitRenderer()=0;
	virtual bool Shutdown()=0;

	virtual void Draw(const CCamera * camera)=0;
	virtual void DrawConsole()= 0;
	
	//Get other interfaces
	virtual I_ClientRenderer  * GetClient()=0;
	virtual I_ConsoleRenderer * GetConsole()=0;

	//Windowing
	virtual void MoveWindow(int x, int y) = 0;
	virtual void Resize() =0;
	virtual void ChangeDispSettings(unsigned int width, unsigned int height, 
									unsigned int bpp, bool fullscreen)=0;
	
	//World
	virtual bool LoadWorld(CWorld *level, int reload) =0;
	virtual bool UnloadWorld() = 0;
};


RENDERER_API I_Renderer   * RENDERER_Create(I_Void * vexp);
RENDERER_API RenderInfo_t * RENDERER_GetParms();
RENDERER_API void RENDERER_Free();

#endif

