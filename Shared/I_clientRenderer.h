#ifndef VOID_CLIENT_RENDERER
#define VOID_CLIENT_RENDERER

#include "Cl_defs.h"

/*
============================================================================
This header is shared between the renderer, the exe and the clientside dll.
The struct defs should be considerer as constant by the clientside dll
it can subclass the EntState struct if more functionality is wanted.
============================================================================
*/
enum
{
	MODEL_SKIN_BOUND = 0,
	MODEL_SKIN_UNBOUND_GAME  = 0X80000000,
	MODEL_SKIN_UNBOUND_LOCAL = 0X40000000
};

enum EHudItemType
{
	HUDTEXT,		//passing char data that should be allocated and displayed
	HUDSTRING,		//passing a pointer to a static string
	HUDINT,
	HUDFLOAT,
	HUDIMAGE,
	HUDMODEL
};

const float HUD_DEFAULTMSGTIME = 3.0f;

/*
==========================================
Renderer Model/Image/Hud Interface
==========================================
*/
struct I_ClientRenderer
{
	/* Model Interface */
	virtual int  LoadModel(const char *model, CacheType cache, int index=-1)=0;
	virtual void DrawModel(const ClEntity &state)=0;
	virtual void UnloadModel(CacheType cache, int index)=0;
	virtual void UnloadModelCache(CacheType cache)=0;
	virtual void UnloadModelAll(void)=0;
	virtual void GetInfo(ClEntity &state)=0;

	/* Image Interface */
	virtual int  LoadImage(const char *image, CacheType cache, int index=-1)=0;
	virtual void UnloadImage(CacheType cache, int index)=0;
	virtual void UnloadImageCache(CacheType cache)=0;
	virtual void UnloadImageAll(void)=0;

	/* Hud Interface */
	virtual void HudPrintf(int x, int y, float time,char *msg,...) =0;
	virtual void HudPrint(char *msg, int x, int y, float time =0.0, int color=0) =0;
	virtual void PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME) =0;
};

#endif