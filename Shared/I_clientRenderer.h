#ifndef VOID_CLIENT_RENDERER
#define VOID_CLIENT_RENDERER

#include "Cl_base.h"

/*
==========================================
Renderer Model/Image/Hud Interface
==========================================
*/
struct I_ClientRenderer
{
	/* Model Interface */
	virtual int  LoadModel(const char *model, CacheType cache, int index=-1)=0;
	virtual void DrawModel(ClEntity &state)=0;
	virtual void UnloadModel(CacheType cache, int index)=0;
	virtual void UnloadModelCache(CacheType cache)=0;
	virtual void UnloadModelAll(void)=0;

	/* Image Interface */
	virtual int  LoadImage(const char *image, CacheType cache, int index=-1)=0;
	virtual void UnloadImage(CacheType cache, int index)=0;
	virtual void UnloadImageCache(CacheType cache)=0;
	virtual void UnloadImageAll(void)=0;
};

#endif