
#include "Img_main.h"

//CImageManager	*g_pImage=0;




/*
=======================================
Constructor 
=======================================
*/
CImageManager::CImageManager()
{
	ready = true;

	// reset
	for (int c=0; c<IMAGE_CACHE_NUM; c++)
	{
		for (int e=0; e<IMAGE_CACHE_SIZE; e++)
		{
			caches[c][e] = NULL;
		}
	}
}

/*
=======================================
Destructor 
=======================================
*/
CImageManager::~CImageManager()
{
	for (int c=0; c<IMAGE_CACHE_NUM; c++)
	{
		for (int e=0; e<IMAGE_CACHE_SIZE; e++)
		{
			if (caches[c][e])
			{
				delete caches[c][e];
				caches[c][e] = NULL;
			}
		}
	}
}

/*
=======================================
LoadModel 
=======================================
*/
hMdl CImageManager::LoadImage(const char *image, CacheType cache, hImg index)
{
	// find the first available spot in this cache
	if (index == -1)
	{
		for (int i=0; i<IMAGE_CACHE_SIZE; i++)
		{
			if (!caches[cache][i])
			{
				index = i;
				break;
			}
		}

		if (i==IMAGE_CACHE_SIZE)
		{
			ComPrintf("no available cache entries for image %s\n", image);
			return -1;
		}
	}

	// add it in the specified spot
	if (caches[cache][index])
	{
		ComPrintf("image already in specified index\n");
		delete caches[cache][index];
	}

	caches[cache][index] =  new CImageCacheEntry(image);

	return index;
}


/*
=======================================
UnloadImage 
=======================================
*/
void CImageManager::UnloadImage(CacheType cache, hImg index)
{
	if (!caches[cache][index])
		ComPrintf("CImageManager::UnloadImage - image not loaded\n");

	else
	{
		delete caches[cache][index];
		caches[cache][index] = NULL;
	}
}



/*
=======================================
UnloadImageCache 
=======================================
*/
void CImageManager::UnloadImageCache(CacheType cache)
{
	for (int i=0; i<MODEL_CACHE_SIZE; i++)
	{
		if (caches[cache][i])
			UnloadImage(cache, i);
	}

}



/*
=======================================
UnloadImageAll 
=======================================
*/
void CImageManager::UnloadImageAll(void)
{
	for (int c=0; c<MODEL_CACHE_NUM; c++)
		UnloadImageCache((CacheType)c);
}


/*
=======================================
LoadTextures
=======================================
*/
void CImageManager::LoadTextures(void)
{
	for (int c=0; c<IMAGE_CACHE_NUM; c++)
	{
		for (int e=0; e<IMAGE_CACHE_SIZE; e++)
		{
			if (caches[c][e])
				caches[c][e]->LoadTexture();
		}
	}

	ready = true;
}


/*
=======================================
UnLoadTextures
=======================================
*/
void CImageManager::UnLoadTextures(void)
{
	for (int c=0; c<IMAGE_CACHE_NUM; c++)
	{
		for (int e=0; e<IMAGE_CACHE_SIZE; e++)
		{
			if (caches[c][e])
				caches[c][e]->UnLoadTexture();
		}
	}

	ready = false;
}




