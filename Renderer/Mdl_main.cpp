#include "Standard.h"
#include "Mdl_main.h"
#include "Mdl_md2.h"
#include "Mdl_sp2.h"

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
	ready = true;

	// reset
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXMODELS; e++)
		{	caches[c][e] = NULL;
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
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXMODELS; e++)
		{
			if (caches[c][e])
			{
				if (caches[c][e]->Release() == 0)
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
int CModelManager::LoadModel(const char *model, CacheType mdlCache, int mdlIndex)
{
	// find the first available spot in this mdlCache
	if (mdlIndex == -1)
	{
		for (int i=0; i<GAME_MAXMODELS; i++)
		{
			if (!caches[mdlCache][i])
			{
				mdlIndex = i;
				break;
			}
		}

		if (i==GAME_MAXMODELS)
		{
			ComPrintf("no available mdlCache entries for model %s\n", model);
			return -1;
		}
	}

	// make sure our spot is free
	if (caches[mdlCache][mdlIndex])
	{
		ComPrintf("model already in specified mdlIndex\n");
		delete caches[mdlCache][mdlIndex];
	}


	// search all caches to see if it is already loaded somewhere
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int i=0; i<GAME_MAXMODELS; i++)
		{
			if (caches[c][i] && caches[c][i]->IsFile(model))
			{
				caches[mdlCache][mdlIndex] = caches[c][i];
				caches[c][i]->AddRef();
				return mdlIndex;
			}
		}
	}

	// else create a new one
	if (_stricmp("md2", &model[strlen(model)-3])==0)
		caches[mdlCache][mdlIndex] =  new CModelMd2();
	else
		caches[mdlCache][mdlIndex] =  new CModelSp2();

	caches[mdlCache][mdlIndex]->LoadModel(model);
	return mdlIndex;
}


/*
=======================================
DrawModel 
=======================================
*/
void CModelManager::DrawModel(const ClEntity &state)
{
	// add model to list to be drawn
	drawmodel_t *ndm = drawmodelGet();
	ndm->state = &state;
	ndm->next	= drawmodels;
	drawmodels	= ndm;
}


/*
=======================================
UnloadModel 
=======================================
*/
void CModelManager::UnloadModel(CacheType mdlCache, int mdlIndex)
{
	if (!caches[mdlCache][mdlIndex])
		ComPrintf("CModelManager::UnloadModel - model not loaded\n");

	else
	{
		if (caches[mdlCache][mdlIndex]->Release() == 0)
			delete caches[mdlCache][mdlIndex];
		caches[mdlCache][mdlIndex] = NULL;
	}
}



/*
=======================================
UnloadModelCache 
=======================================
*/
void CModelManager::UnloadModelCache(CacheType mdlCache)
{
	for (int i=0; i<GAME_MAXMODELS; i++)
	{
		if (caches[mdlCache][i])
			UnloadModel(mdlCache, i);
	}

}

/*
=======================================
UnloadModelAll 
=======================================
*/
void CModelManager::UnloadModelAll(void)
{
	for (int c=0; c<CACHE_NUMCACHES; c++)
		UnloadModelCache((CacheType)c);
}


/*
=======================================
GetInfo 
=======================================
*/
void CModelManager::GetInfo(ClEntity &state)
{
	state.numFrames = caches[state.mdlCache][state.mdlIndex]->GetNumFrames();
	state.numSkins  = caches[state.mdlCache][state.mdlIndex]->GetNumSkins();
}


/*
=======================================
Purge
=======================================
*/
void CModelManager::Purge(void)
{
	if (!ready && drawmodels)
	{
		ComPrintf("drawing models when not ready!!!\n");
		return;
	}

	drawmodel_t *next, *walk;
	vector_t trans;

	g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
	g_pRast->BlendFunc(VRAST_SRC_BLEND_NONE, VRAST_DST_BLEND_NONE);
	g_pRast->PolyColor3f(1, 1, 1);

	for (walk=drawmodels; walk; walk=next)
	{
		next = walk->next;
		g_pRast->MatrixPush();

		VectorInv2(walk->state->origin, trans);
		g_pRast->MatrixTranslate(trans);

		g_pRast->MatrixRotateY(-walk->state->angles.YAW   * 180/PI);
		g_pRast->MatrixRotateX( walk->state->angles.PITCH * 180/PI);
		g_pRast->MatrixRotateZ(-walk->state->angles.ROLL  * 180/PI);

		caches[walk->state->mdlCache][walk->state->mdlIndex]->Draw(walk->state->skinNum, 
																   walk->state->frameNum,
																   walk->state->nextFrame, 
																   walk->state->frac);

		g_pRast->MatrixPop();

		drawmodelRelease(walk);
	}

	drawmodels = NULL;
}


/*
=======================================
LoadSkins
=======================================
*/
void CModelManager::LoadSkins(void)
{
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXMODELS; e++)
		{
			if (caches[c][e])
				caches[c][e]->LoadSkins();
		}
	}

	ready = true;
}


/*
=======================================
UnLoadSkins
=======================================
*/
void CModelManager::UnLoadSkins(void)
{
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXMODELS; e++)
		{
			if (caches[c][e])
				caches[c][e]->UnLoadSkins();
		}
	}

	ready = false;
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