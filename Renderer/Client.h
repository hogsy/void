#ifndef CLIENTRENDERER_H
#define CLIENTRENDERER_H

#include "Hud_main.h"
#include "Img_main.h"
#include "Mdl_main.h"
#include "I_clientRenderer.h"


class CClientRenderer : public I_ClientRenderer,
						private CModelManager,
						private CImageManager,
						private CRHud
					    
{
public:
	CClientRenderer() {}
	virtual ~CClientRenderer() { }

	// Model Interface
	inline int LoadModel(const char *model, CacheType cache, int index=-1)
	{	return CModelManager::LoadModel(model, cache, index);	
	}
	inline void DrawModel(const ClEntity &state)
	{	CModelManager::DrawModel(state);	
	}
	inline void UnloadModel(CacheType cache, int index)
	{	CModelManager::UnloadModel(cache, index);	
	}
	inline void UnloadModelCache(CacheType cache)
	{	CModelManager::UnloadModelCache(cache);	
	}
	inline void UnloadModelAll(void)
	{	CModelManager::UnloadModelAll();	
	}
	inline void GetInfo(ClEntity &state)
	{	CModelManager::GetInfo(state);	
	}

	// Image Interface
	inline int LoadImage(const char *image, CacheType cache, int index=-1)
	{	return CImageManager::LoadImage(image, cache, index);	
	}
	inline void UnloadImage(CacheType cache, int index)
	{	CImageManager::UnloadImage(cache, index);	
	}
	inline void UnloadImageCache(CacheType cache)
	{	CImageManager::UnloadImageCache(cache);	
	}
	inline void UnloadImageAll(void)
	{	CImageManager::UnloadImageAll();	
	}

	/* Hud Interface */
	inline void  HudPrintf(int x, int y, float time,char *msg,...)
	{	
		va_list args;
		va_start(args, msg);
		vsprintf(m_hudBuffer, msg, args);
		va_end(args);
		CRHud::HudPrintf(x, y, time, m_hudBuffer);	
	}
	inline void  HudPrint(char *msg, int x, int y, float time =0.0, int color=0)
	{	CRHud::HudPrint(msg, x, y, time, color);	
	}
	inline void  PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME)
	{	CRHud::PrintMessage(msg, color, time);	
	}

	// non interface funcs
	inline void Purge(void)
	{	CModelManager::Purge();	
	}
	// funcs for vid restarts
	inline void LoadSkins(void)
	{	CModelManager::LoadSkins();	
	}
	inline void UnLoadSkins(void)
	{	CModelManager::UnLoadSkins();	
	}
	inline void LoadTextures(void)
	{   CImageManager::LoadTextures();	
	}
	inline void UnLoadTextures(void)
	{   CImageManager::UnLoadTextures();	
	}

	inline void Set(CacheType cache, int index)
	{	CImageManager::Set(cache, index);	
	}
	inline 	void DrawHud()
	{	CRHud::DrawHud();	
	}

private:
	char m_hudBuffer[1024];
};

extern CClientRenderer *g_pClient;

#endif

