
#ifndef CLIENTRENDERER_H
#define CLIENTRENDERER_H

#include "I_Renderer.h"
#include "Hud_main.h"
#include "Img_main.h"
#include "Mdl_main.h"


class CClientRenderer : public I_ClientRenderer
{
public:
	/* Model Interface */
	inline hMdl LoadModel(const char *model, CacheType cache, hMdl index=-1)
		{	return mModel.LoadModel(model, cache, index);	}
	inline void DrawModel(const R_EntState &state)
		{	mModel.DrawModel(state);	}
	inline void UnloadModel(CacheType cache, hMdl index)
		{	mModel.UnloadModel(cache, index);	}
	inline void UnloadModelCache(CacheType cache)
		{	mModel.UnloadModelCache(cache);	}
	inline void UnloadModelAll(void)
		{	mModel.UnloadModelAll();	}
	inline void GetInfo(R_EntState &state)
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
	inline void __stdcall HudPrintf(int x, int y, float time,char *msg,...)
		{	
			char buff[256];
			va_list args;
			va_start(args, msg);
			vsprintf(buff, msg, args);
			va_end(args);
			mHud.HudPrintf(x, y, time, buff);	
		}
	inline void __stdcall HudPrint(char *msg, int x, int y, float time =0.0, int color=0)
		{	mHud.HudPrint(msg, x, y, time, color);	}
	inline void __stdcall PrintMessage(char *msg, int color=0, float time=HUD_DEFAULTMSGTIME)
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


extern CClientRenderer *g_pClient;

#endif

