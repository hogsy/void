#ifndef INC_RENDERER_INTERFACE
#define INC_RENDERER_INTERFACE

#include "I_console.h"
#include "I_hud.h"
#include "I_void.h"
#include "World.h"


/*
==========================================
Renderer info struct
==========================================
*/
#define RFLAG_FULLSCREEN	0x00000001
#define RFLAG_FULLBRIGHT	0x00000002
#define RFLAG_MULTITEXTURE	0x00000004
#define RFLAG_SWAP_CONTROL	0x00000008


typedef struct RenderInfo_t
{
	HWND		hWnd;		//Window information
	HINSTANCE	hInst;		//Why is this here ? 

	HDC			hDC;		//device context
	HGLRC		hRC;		//the gl rendering context

	bool		active;		//in the window active?
	bool		ready;

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
	virtual I_RConsole * GetConsole()=0;
	virtual I_RHud *	 GetHud()=0;

	//Windowing
	virtual void MoveWindow(int x, int y) = 0;
	virtual void Resize() =0;
	virtual void ChangeDispSettings(unsigned int width, unsigned int height, 
										unsigned int bpp, unsigned int fullscreen)=0;
	
	//World
	virtual bool LoadWorld(world_t *level, int reload) =0;
	virtual bool UnloadWorld() = 0;
};


extern "C"
{

HRESULT  __stdcall GetRendererAPI(I_Renderer ** pRender, RenderInfo_t *rinfo, VoidExport_t * vexp);
typedef HRESULT (*GETRENDERERAPI)(I_Renderer ** pRender, RenderInfo_t *rinfo, VoidExport_t * vexp);

HRESULT  __stdcall FreeRenderer(I_Renderer ** pRender);
typedef HRESULT (*FREERENDERER)(I_Renderer ** pRender);

}


#endif

