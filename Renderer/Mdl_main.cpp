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

	m_pFile = CreateFileReader(FILE_BUFFERED);

	//Initialize to 0
	for (int c=0; c<CACHE_NUMCACHES; c++)
	{
		for (int e=0; e<GAME_MAXMODELS; e++)
			caches[c][e] = NULL;
	}
}

/*
=======================================
Destructor 
=======================================
*/
CModelManager::~CModelManager()
{
	UnloadModelAll();

	// free any drawmodel structs we have allocated
	for (int c=0; c<num_drawmodel_allocs; c++)
		delete [] drawmodel_allocs[c];

	m_pFile->Destroy();
}

/*
=======================================
LoadModel 
=======================================
*/
int CModelManager::LoadModel(const char *model, CacheType mdlCache, int mdlIndex)
{
	if(mdlIndex == -1)
	{
		for(int i=0; i<GAME_MAXMODELS; i++)
		{
			if (!caches[mdlCache][i])
			{
				if(mdlIndex == -1)
					mdlIndex = i;
			}
			else if(caches[mdlCache][i]->IsFile(model))
			{
				caches[mdlCache][i]->AddRef();
				return i;
			}
		}

		if(mdlIndex == -1)
		{
			ComPrintf("CModelManager::LoadModel::No space to load %s\n", model);
			return -1;
		}
	}

	// make sure our spot is free
	if (caches[mdlCache][mdlIndex])
	{
		ComPrintf("CModelManager::LoadModel:: Error loading %s. Slot occupied by %s\n", model,
			caches[mdlCache][mdlIndex]->GetFileName());
		return -1;
	}

	// else create a new one
	if (_stricmp("md2", &model[strlen(model)-3])==0)
		caches[mdlCache][mdlIndex] =  new CModelMd2();
	else
		caches[mdlCache][mdlIndex] =  new CModelSp2();


	if(!m_pFile->Open(model))
		caches[mdlCache][mdlIndex]->LoadFail();
	else
		caches[mdlCache][mdlIndex]->LoadModel(m_pFile,model);
	
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
		ComPrintf("CModelManager::UnloadModel:[%d][%d]: model not loaded\n", mdlCache, mdlIndex);
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
Unload models regardless of locks
=======================================
*/
void CModelManager::UnloadModelCache(CacheType mdlCache)
{
	for (int i=0; i<GAME_MAXMODELS; i++)
	{
		if (caches[mdlCache][i])
		{
			delete caches[mdlCache][i];
			caches[mdlCache][i] = 0;
		}
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
//	vector_t trans;

	g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
	g_pRast->BlendFunc(VRAST_SRC_BLEND_NONE, VRAST_DST_BLEND_NONE);
	g_pRast->PolyColor3f(1, 1, 1);

	for (walk=drawmodels; walk; walk=next)
	{
		next = walk->next;
		g_pRast->MatrixPush();

		// add this model transform to the stack
		g_pRast->MatrixTranslate(walk->state->origin.x, walk->state->origin.y, walk->state->origin.z);

		g_pRast->MatrixRotateZ(walk->state->angles.YAW   * 180/PI);
		g_pRast->MatrixRotateY(-walk->state->angles.PITCH  * 180/PI);
		g_pRast->MatrixRotateX(walk->state->angles.ROLL * 180/PI);

//		walk->state->animInfo.
		int nextframe = walk->state->animInfo.currentFrame + 1;
		if (nextframe > walk->state->animInfo.frameEnd)
			nextframe = walk->state->animInfo.frameBegin;
		float frac = GetCurTime();
		frac -= floor(frac);

		caches[walk->state->mdlCache][walk->state->mdlIndex]->Draw(walk->state->skinNum, 
																   walk->state->animInfo.currentFrame,
																   nextframe,
																   frac);
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
//	if (!drawmodel_allocs[num_drawmodel_allocs])
//		FError("not enough mem for drawmodels! - %d allocated", DRAWMODELS_PER_ALLOC*num_drawmodel_allocs);

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