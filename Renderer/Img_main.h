#ifndef IMG_MAIN_H
#define IMG_MAIN_H

#include "Standard.h"
#include "Img_entry.h"
#include "I_clientRenderer.h"


class CImageManager
{
public:
	CImageManager();
	~CImageManager();

	/* Interface functions */

	// load a model into memory
	hImg LoadImage(const char *image, CacheType cache, hImg index=-1);

	// unload images from memory
	void UnloadImage(CacheType cache, hImg index);
	void UnloadImageCache(CacheType cache);
	void UnloadImageAll(void);


	// funcs for vid restarts
	void UnLoadTextures(void);
	void LoadTextures(void);

	void Set(CacheType cache, hImg index) { caches[cache][index]->Set(); }

private:

	bool ready;
	CImageCacheEntry *caches[CACHE_NUMCACHES][CACHE_NUMIMAGES];
};

#endif