// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
//														//
//	I know that when the game dll's are made, this		//
//	will have to be pretty much totally rewritten.		//
//														//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //


#include "Mdl_main.h"

CModelManager	*g_pModel=0;




/*
=======================================
Constructor 
=======================================
*/
CModelManager::CModelManager()
{
	num_drawmodel_allocs = 0;
	free_drawmodels = NULL;
	drawmodels = NULL;

	// reset
	for (int c=0; c<MODEL_CACHE_NUM; c++)
	{
		for (int e=0; e<MODEL_CACHE_SIZE; e++)
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
CModelManager::~CModelManager()
{
	for (int c=0; c<MODEL_CACHE_NUM; c++)
	{
		for (int e=0; e<MODEL_CACHE_SIZE; e++)
		{
			if (caches[c][e])
			{
				delete caches[c][e];
				caches[c][e] = NULL;
			}
		}
	}

	// free any drawmodel structs we have allocated
	for (c=0; c<num_drawmodel_allocs; c++)
		delete drawmodel_allocs[c];
}

/*
=======================================
LoadModel 
=======================================
*/
hMdl CModelManager::LoadModel(const char *model, hMdl index, CacheType cache)
{
	// find the first available spot in this cache
	if (index == -1)
	{
		for (int i=0; i<MODEL_CACHE_SIZE; i++)
		{
			if (!caches[cache][i])
			{
				index = i;
				break;
			}
		}

		if (i==MODEL_CACHE_SIZE)
		{
			ComPrintf("no available cache entries for model %s\n", model);
			return -1;
		}
	}

	// add it in the specified spot
	if (caches[cache][index])
	{
		ComPrintf("model already in specified index\n");
		delete caches[cache][index];
	}

	caches[cache][index] =  new CModelCacheEntry(model);

	return index;
}


/*
=======================================
DrawModel 
=======================================
*/
void CModelManager::DrawModel(hMdl index, CacheType cache, const R_EntState &state)
{
	// add model to list to be drawn
	drawmodel_t *ndm = drawmodelGet();
	ndm->angles	= state.angle;
	ndm->cache	= cache;
	ndm->frame	= state.frame;
	ndm->index	= index;
	ndm->origin = state.origin;
	ndm->skin	= state.skinnum;

	ndm->next	= drawmodels;
	drawmodels	= ndm;
}


/*
=======================================
UnloadModel 
=======================================
*/
void CModelManager::UnloadModel(CacheType cache, int index)
{
	if (!caches[cache][index])
		ComPrintf("CModelManager::UnloadModel - model not loaded\n");

	else
	{
		delete caches[cache][index];
		caches[cache][index] = NULL;
	}
}



/*
=======================================
UnloadModelCache 
=======================================
*/
void CModelManager::UnloadModelCache(CacheType cache)
{
	for (int i=0; i<MODEL_CACHE_SIZE; i++)
	{
		if (caches[cache][i])
			UnloadModel(cache, i);
	}

}



/*
=======================================
UnloadModelAll 
=======================================
*/
void CModelManager::UnloadModelAll(void)
{
	for (int c=0; c<MODEL_CACHE_NUM; c++)
		UnloadModelCache((CacheType)c);
}


/*
=======================================
Purge
=======================================
*/
void CModelManager::Purge(void)
{
	drawmodel_t *next, *walk;

	for (walk=drawmodels; walk; walk=next)
	{
		next = walk->next;
		g_pRast->MatrixPush();
		
		g_pRast->MatrixRotateZ( walk->angles.ROLL  * 180/PI);
		g_pRast->MatrixRotateX(-walk->angles.PITCH * 180/PI);
		g_pRast->MatrixRotateY( walk->angles.YAW   * 180/PI);

		g_pRast->MatrixTranslate(walk->origin);

		caches[walk->cache][walk->index]->Draw(walk->skin, walk->frame);

		g_pRast->MatrixPop();

		drawmodelRelease(walk);
	}

	drawmodels = NULL;
}





/*
=========
drawmodelGet - allocate a drawmodel
=========
*/
void CModelManager::drawmodelAlloc(void)
{
	if (num_drawmodel_allocs == MAX_DRAWMODEL_ALLOCS)
		FError("too many drawmodel allocs! Tell Ripper\n");

	drawmodel_allocs[num_drawmodel_allocs] = new drawmodel_t[DRAWMODELS_PER_ALLOC];
	if (!drawmodel_allocs[num_drawmodel_allocs])
		FError("not enough mem for drawmodels! - %d allocated", DRAWMODELS_PER_ALLOC*num_drawmodel_allocs);

	free_drawmodels = drawmodel_allocs[num_drawmodel_allocs];
	num_drawmodel_allocs++;

	for (int a=0; a<DRAWMODELS_PER_ALLOC-1; a++)
		free_drawmodels[a].next = &free_drawmodels[a+1];
	free_drawmodels[a].next = NULL;
}


CModelManager::drawmodel_t* CModelManager::drawmodelGet(void)
{
	if (!free_drawmodels)
		drawmodelAlloc();

	drawmodel_t *ret = free_drawmodels;
	free_drawmodels = free_drawmodels->next;
	return ret;
}


/*
=========
drawmodelRelease - return the drawmodel to the list
=========
*/
void CModelManager::drawmodelRelease(drawmodel_t *d)
{
	d->next = free_drawmodels;
	free_drawmodels = d;
}






