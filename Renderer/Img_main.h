#ifndef IMG_MAIN_H
#define IMG_MAIN_H

#include "I_clientRenderer.h"

class CImageCacheEntry;

class CImageManager
{
public:
	CImageManager();
	virtual ~CImageManager();

	/* Interface functions */

	// load a model into memory
	int LoadImage(const char *image, CacheType cache, int imgIndex=-1);

	// unload images from memory
	void UnloadImage(CacheType cache, int index);
	void UnloadImageCache(CacheType cache);
	void UnloadImageAll(void);


	// funcs for vid restarts
	void UnLoadTextures(void);
	void LoadTextures(void);

	void Set(CacheType cache, int index);

private:

	bool ready;
	CImageCacheEntry * caches[CACHE_NUMCACHES][GAME_MAXIMAGES];
};

#endif