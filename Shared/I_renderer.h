#ifndef INC_RENDERER_INTERFACE
#define INC_RENDERER_INTERFACE

#include "I_hud.h"
#include "I_void.h"
#include "Game_defs.h"
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
==========================================
Renderer Model Interface
==========================================
*/
enum
{
	MODEL_CACHE_NUM	= 3,
	MODEL_CACHE_SIZE =256
};

enum
{
	MODEL_SKIN_BOUND = 0,
	MODEL_SKIN_UNBOUND_GAME  = 0X80000000,
	MODEL_SKIN_UNBOUND_LOCAL = 0X40000000
};

struct R_EntState 
{
	int			num_skins;
	int			num_frames;

	CacheType	cache;
	hMdl		index;
	int			skinnum;

	int			frame;
	int			nextframe;
	float		frac;

	vector_t origin;
	vector_t angle;
};
/*
struct I_Model
{
	virtual hMdl LoadModel(const char *model, hMdl index, CacheType cache)=0;
	virtual void DrawModel(const R_EntState &state)=0;
	virtual void UnloadModel(CacheType cache, hMdl index)=0;
	virtual void UnloadModelCache(CacheType cache)=0;
	virtual void UnloadModelAll(void)=0;
	virtual void GetInfo(R_EntState &state)=0;
};
*/

/*
==========================================
Renderer Image Interface
==========================================
*/
enum
{
	IMAGE_CACHE_NUM	= 2,
	IMAGE_CACHE_SIZE =256
};
/*
struct I_Image
{
	virtual hImg LoadImage(const char *image, hImg index, CacheType cache)=0;
	virtual void UnloadImage(CacheType cache, hImg index)=0;
	virtual void UnloadImageCache(CacheType cache)=0;
	virtual void UnloadImageAll(void)=0;
};
*/


/*
==========================================
Renderer Model/Image/Hud Interface
==========================================
*/
struct I_ClientRenderer
{
	/* Model Interface */
	virtual hMdl LoadModel(const char *model, CacheType cache, hMdl index=-1)=0;
	virtual void DrawModel(const R_EntState &state)=0;
	virtual void UnloadModel(CacheType cache, hMdl index)=0;
	virtual void UnloadModelCache(CacheType cache)=0;
	virtual void UnloadModelAll(void)=0;
	virtual void GetInfo(R_EntState &state)=0;

	/* Image Interface */
	virtual hImg LoadImage(const char *image, CacheType cache, hImg index=-1)=0;
	virtual void UnloadImage(CacheType cache, hImg index)=0;
	virtual void UnloadImageCache(CacheType cache)=0;
	virtual void UnloadImageAll(void)=0;

	/* Hud Interface */
	virtual void __stdcall HudPrintf(int x, int y, float time,char *msg,...) =0;
	virtual void __stdcall HudPrint(char *msg, int x, int y, float time =0.0, int color=0) =0;
	virtual void __stdcall PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME) =0;
};




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

	virtual void Draw(const CCamera * camera)=0;
	virtual void DrawConsole()= 0;
	
	//Get other interfaces
	virtual I_ClientRenderer  * GetClient()=0;
	virtual I_ConsoleRenderer * GetConsole()=0;
/*
	virtual I_RHud *			GetHud()=0;
	virtual I_Model	*			GetModel()=0;
	virtual I_Image *			GetImage()=0;
*/
	//Windowing
	virtual void MoveWindow(int x, int y) = 0;
	virtual void Resize() =0;
	virtual void ChangeDispSettings(unsigned int width, unsigned int height, 
									unsigned int bpp, bool fullscreen)=0;
	
	//World
	virtual bool LoadWorld(world_t *level, int reload) =0;
	virtual bool UnloadWorld() = 0;
};


RENDERER_API I_Renderer   * RENDERER_Create(I_Void * vexp);
RENDERER_API RenderInfo_t * RENDERER_GetParms();
RENDERER_API void RENDERER_Free();

#endif

