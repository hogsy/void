#ifndef MDL_MAIN_H
#define MDL_MAIN_H

#include "I_clientRenderer.h"

#ifdef _DEBUG
#define MAX_DRAWMODEL_ALLOCS 64
#define	DRAWMODELS_PER_ALLOC 4
#else
#define MAX_DRAWMODEL_ALLOCS 16
#define	DRAWMODELS_PER_ALLOC 16
#endif

class CModelCacheEntry;


class CModelManager
{
public:
	CModelManager();
	virtual ~CModelManager();

	/* Interface functions */

	// load a model into memory
	int LoadModel(const char *model, CacheType cache, int index=-1);

	// add the model to the render cache
	void DrawModel(const ClEntity &state);
	void Purge(void);

	// unload models from memory
	void UnloadModel(CacheType cache, int index);
	void UnloadModelCache(CacheType cache);
	void UnloadModelAll(void);

	void GetInfo(ClEntity &state);

	// funcs for vid restarts
	void LoadSkins(void);
	void UnLoadSkins(void);


private:

	// struct to hold a list of models to be drawn
	struct drawmodel_t
	{
		drawmodel_t() { state = 0; next = 0; }
		~drawmodel_t() { state = 0; next = 0; }

		const ClEntity *state;
		drawmodel_t *next;
	};

	int		num_drawmodel_allocs;
	drawmodel_t	*drawmodel_allocs[MAX_DRAWMODEL_ALLOCS];
	drawmodel_t	*free_drawmodels;
	drawmodel_t *drawmodels;	// list of models to be drawn

	void drawmodelAlloc(void);
	drawmodel_t* drawmodelGet(void);
	void drawmodelRelease(drawmodel_t *d);

	bool ready;

	CModelCacheEntry *caches[CACHE_NUMCACHES][GAME_MAXMODELS];
};


#endif


