#ifndef INC_RENDERER_INTERFACE
#define INC_RENDERER_INTERFACE

#include "I_console.h"
#include "I_hud.h"
#include "I_void.h"
#include "World.h"

#ifdef RENDERER_EXPORTS
#define RENDERER_API __declspec(dllexport)
#else
#define RENDERER_API __declspec(dllimport)
#endif


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
		fov = PI/2;
		rflags = 0;
	}

	//==========================================

	HWND		hWnd;		//Window information

	HDC			hDC;		//device context
	HGLRC		hRC;		//the gl rendering context

	bool		active;		//in the window active?
	bool		ready;

	//==========================================
	//Renderering Information
	
	float			fov;	//fov in radians
	unsigned char	rflags;

	unsigned int    width, 
					height, 
					bpp,
					zdepth,
					stencil;
}RenderInfo_t;


/*
==========================================
Renderer Interface
==========================================
*/
struct I_Renderer
{
	//Startup/Shutdown
	virtual bool InitRenderer()=0;
	virtual bool Shutdown()=0;

	virtual void DrawFrame(vector_t *origin,vector_t *angles) =0;

	//Get other interfaces
	virtual I_ConsoleRenderer * GetConsole()=0;
	virtual I_RHud *			GetHud()=0;

	//Windowing
	virtual void MoveWindow(int x, int y) = 0;
	virtual void Resize() =0;
	virtual void ChangeDispSettings(unsigned int width, unsigned int height, 
									unsigned int bpp, bool fullscreen)=0;
	
	//World
	virtual bool LoadWorld(world_t *level, int reload) =0;
	virtual bool UnloadWorld() = 0;
};


//RENDERER_API I_Renderer   * RENDERER_Create(VoidExport_t * vexp);
RENDERER_API I_Renderer   * RENDERER_Create(I_Void * vexp);
RENDERER_API RenderInfo_t * RENDERER_GetParms();
RENDERER_API void RENDERER_Free();

#endif

