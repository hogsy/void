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
	inline hMdl LoadModel(const char *model, CacheType cache, hMdl index=-1)
	{	return CModelManager::LoadModel(model, cache, index);	
	}
	inline void DrawModel(const EntState &state)
	{	CModelManager::DrawModel(state);	
	}
	inline void UnloadModel(CacheType cache, hMdl index)
	{	CModelManager::UnloadModel(cache, index);	
	}
	inline void UnloadModelCache(CacheType cache)
	{	CModelManager::UnloadModelCache(cache);	
	}
	inline void UnloadModelAll(void)
	{	CModelManager::UnloadModelAll();	
	}
	inline void GetInfo(EntState &state)
	{	CModelManager::GetInfo(state);	
	}

	// Image Interface
	inline hImg LoadImage(const char *image, CacheType cache, hImg index=-1)
	{	return CImageManager::LoadImage(image, cache, index);	
	}
	inline void UnloadImage(CacheType cache, hImg index)
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

	inline void Set(CacheType cache, hImg index)
	{	CImageManager::Set(cache, index);	
	}
	inline 	void DrawHud()
	{	CRHud::DrawHud();	
	}

private:
	char m_hudBuffer[1024];
};



#if 0

class CClientRenderer : public I_ClientRenderer
{
public:
	CClientRenderer() {}
	virtual ~CClientRenderer() { }

	/* Model Interface */
	inline hMdl LoadModel(const char *model, CacheType cache, hMdl index=-1)
		{	return mModel.LoadModel(model, cache, index);	}
	inline void DrawModel(const EntState &state)
		{	mModel.DrawModel(state);	}
	inline void UnloadModel(CacheType cache, hMdl index)
		{	mModel.UnloadModel(cache, index);	}
	inline void UnloadModelCache(CacheType cache)
		{	mModel.UnloadModelCache(cache);	}
	inline void UnloadModelAll(void)
		{	mModel.UnloadModelAll();	}
	inline void GetInfo(EntState &state)
		{	mModel.GetInfo(state);	}

	/* Image Interface */
	inline hImg LoadImage(const char *image, CacheType cache, hImg index=-1)
		{	return mImage.LoadImage(image, cache, index);	}
	inline void UnloadImage(CacheType cache, hImg index)
		{	mImage.UnloadImage(cache, index);	}
	inline void UnloadImageCache(CacheType cache)
		{	mImage.UnloadImageCache(cache);	}
	inline void UnloadImageAll(void)
		{	mImage.UnloadImageAll();	}

	/* Hud Interface */
	inline void  HudPrintf(int x, int y, float time,char *msg,...)
		{	
			char buff[256];
			va_list args;
			va_start(args, msg);
			vsprintf(buff, msg, args);
			va_end(args);
			mHud.HudPrintf(x, y, time, buff);	
		}
	inline void  HudPrint(char *msg, int x, int y, float time =0.0, int color=0)
		{	mHud.HudPrint(msg, x, y, time, color);	}
	inline void  PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME)
		{	mHud.PrintMessage(msg, color, time);	}


	// non interface funcs
	inline void Purge(void)
		{	mModel.Purge();	}

	// funcs for vid restarts
	inline void LoadSkins(void)
		{	mModel.LoadSkins();	}
	inline void UnLoadSkins(void)
		{ mModel.UnLoadSkins();	}
	inline void LoadTextures(void)
		{ mImage.LoadTextures();	}
	inline void UnLoadTextures(void)
		{ mImage.UnLoadTextures();	}

	inline void Set(CacheType cache, hImg index)
		{	mImage.Set(cache, index);	}

	inline 	void DrawHud()
		{	mHud.DrawHud();	}


private:
	CRHud			mHud;
	CImageManager	mImage;
	CModelManager	mModel;
};

#endif


extern CClientRenderer *g_pClient;

#endif

