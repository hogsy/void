#include "Cl_defs.h"
#include "I_clientRenderer.h"
#include "Img_main.h"

/*
=======================================
Constructor 
=======================================
*/
CImageManager::CImageManager()
{
	ready = true;
	// reset
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXIMAGES; e++)
		{	caches[c][e] = NULL;
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
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXIMAGES; e++)
		{
			if (caches[c][e])
			{
				if (caches[c][e]->Release() == 0)
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
int CImageManager::LoadImage(const char *image, CacheType cache, int index)
{
	// find the first available spot in this cache
	if (index == -1)
	{
		for (int i=0; i<GAME_MAXIMAGES; i++)
		{
			if (!caches[cache][i])
			{
				index = i;
				break;
			}
		}

		if (i==GAME_MAXIMAGES)
		{
			ComPrintf("CImageManager::LoadImage: no available cache entries for  %s\n", image);
			return -1;
		}
	}

	// make sure our spot is free
	if (caches[cache][index])
	{
		ComPrintf("image already in specified index\n");
		delete caches[cache][index];
	}

	// search all caches to see if it is already loaded somewhere
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int i=0; i<GAME_MAXIMAGES; i++)
		{
			if (caches[c][i] && caches[c][i]->IsFile(image))
			{
				caches[cache][index] = caches[c][i];
				caches[c][i]->AddRef();
				return index;
			}
		}
	}
	// else create a new one
	caches[cache][index] =  new CImageCacheEntry(image);
	return index;
}

/*
=======================================
UnloadImage 
=======================================
*/
void CImageManager::UnloadImage(CacheType cache, int index)
{
	if (!caches[cache][index])
		ComPrintf("CImageManager::UnloadImage: Image not loaded\n");
	else
	{
		if (caches[cache][index]->Release() == 0)
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
	for (int i=0; i<GAME_MAXIMAGES; i++)
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
	for (int c=0; c<CACHE_NUMCACHES; c++)
		UnloadImageCache((CacheType)c);
}

/*
=======================================
LoadTextures
=======================================
*/
void CImageManager::LoadTextures(void)
{
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXIMAGES; e++)
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
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXIMAGES; e++)
		{
			if (caches[c][e])
				caches[c][e]->UnLoadTexture();
		}
	}
	ready = false;
}



