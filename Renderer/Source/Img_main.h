
#ifndef IMG_MAIN_H
#define IMG_MAIN_H

#include "Standard.h"
#include "Img_entry.h"


class CImageManager : public I_Image
{
public:
	CImageManager();
	~CImageManager();

	/* Interface functions */

	// load a model into memory
	hImg LoadImage(const char *image, hImg index, CacheType cache);

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
	CImageCacheEntry *caches[IMAGE_CACHE_NUM][IMAGE_CACHE_SIZE];
};


extern	CImageManager	*g_pImage;


#endif


