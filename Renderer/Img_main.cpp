#include "Standard.h"
#include "ShaderManager.h"
#include "Img_entry.h"
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
	int c, e;
	for (c=0; c<CACHE_NUMCACHES; c++)
	{
		for (e=0; e<GAME_MAXIMAGES; e++)
			caches[c][e] = NULL;
	}
}

/*
=======================================
Destructor 
=======================================
*/
CImageManager::~CImageManager()
{
	UnloadImageAll();
}

void CImageManager::Set(CacheType cache, int index)
{	caches[cache][index]->Set(); 
}

/*
=======================================
LoadModel 
=======================================
*/
int CImageManager::LoadImage(const char *image, CacheType cache, int imgIndex)
{
	if(imgIndex == -1)
	{
		for(int i=0; i<GAME_MAXMODELS; i++)
		{
			if (!caches[cache][i])
			{
				if(imgIndex == -1)
					imgIndex = i;
			}
			else if(caches[cache][i]->IsFile(image))
			{
				caches[cache][i]->AddRef();
				return i;
			}
		}

		if(imgIndex == -1)
		{
			ComPrintf("CImageManager::LoadImage::No space to load %s\n", image);
			return -1;
		}
	}

	// make sure our spot is free
	if (caches[cache][imgIndex])
	{
		ComPrintf("CImageManager::LoadImage:: Error loading %s. Slot occupied by %s\n", image,
				caches[cache][imgIndex]->GetFileName());
		return -1;
	}

	caches[cache][imgIndex] =  new CImageCacheEntry(image);
	return imgIndex;


/*	// else create a new one
	if (_stricmp("md2", &model[strlen(model)-3])==0)
		caches[mdlCache][mdlIndex] =  new CModelMd2();
	else
		caches[mdlCache][mdlIndex] =  new CModelSp2();

	caches[mdlCache][mdlIndex]->LoadModel(model);
	return mdlIndex;
*/

	// find the first available spot in this cache
/*	if (index == -1)
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
*/
}

/*
=======================================
UnloadImage 
delete only if ref count is 0
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
Delete no matter waht
=======================================
*/
void CImageManager::UnloadImageCache(CacheType cache)
{
	for (int i=0; i<GAME_MAXIMAGES; i++)
	{
		if (caches[cache][i])
		{
			delete caches[cache][i];
			caches[cache][i] = NULL;
		}
			//UnloadImage(cache, i);
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