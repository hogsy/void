
#include "Mdl_main.h"
#include "Mdl_cache.h"


drawmodel_t *drawmodels=NULL;	// list of models to be drawn

/*
=========
get_drawmodel - allocate a drawmodel
=========
*/

#ifdef _DEBUG
#define MAX_DRAWMODEL_ALLOCS 64
#define	DRAWMODELS_PER_ALLOC 4
#else
#define MAX_DRAWMODEL_ALLOCS 16
#define	DRAWMODELS_PER_ALLOC 16
#endif


int		num_drawmodel_allocs = 0;
drawmodel_t	*drawmodel_allocs[MAX_DRAWMODEL_ALLOCS];
drawmodel_t	*free_drawmodels=NULL;

void drawmodel_alloc(void)
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


drawmodel_t* get_drawmodel(void)
{
	if (!free_drawmodels)
		drawmodel_alloc();

	drawmodel_t *ret = free_drawmodels;
	free_drawmodels = free_drawmodels->next;
	return ret;
}


/*
=========
free_drawmodel - return the drawmodel to the list
=========
*/
void free_drawmodel(drawmodel_t *d)
{
	d->next = free_drawmodels;
	free_drawmodels = d;
}





/************************************************
purge the model cache
************************************************
void model_cache_purge(void)
{
	g_pRast->BlendFunc(VRAST_SRC_BLEND_NONE, VRAST_DEST_BLEND_NONE);
	g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);

	for (int m=0; m<used_models; m++)
	{
		// fixme - set skin here!!
		g_pRast->MatrixPush();

		g_pRast->MatrixTranslate(model_cache[m].origin);


		g_pRast->MatrixRotateZ( model_cache[m].angles.ROLL  * 180/PI);
		g_pRast->MatrixRotateX(-model_cache[m].angles.PITCH * 180/PI);
		g_pRast->MatrixRotateY( model_cache[m].angles.YAW   * 180/PI);

		model_draw(model_cache[m].model_num, model_cache[m].frame);

		g_pRast->MatrixPop();
	}

	used_models = 0;
	tmodel = &model_cache[0];
}


/************************************************
add a model to the cache
************************************************
void model_cache_add(void)
{
	used_models++;
	tmodel = &model_cache[used_models];

// if this was the last one, purge
	if (used_models == MAX_CACHED_MODELS)
		model_cache_purge();
}


/************************************************
init the model cache
************************************************
void model_cache_init(void)
{
	model_cache = new model_cache_t[MAX_CACHED_MODELS];
	if (!model_cache) FError("mem for model cache!");

	used_models = 0;
	tmodel = &model_cache[0];

}


/************************************************
destroy the model cache
************************************************
void model_cache_destroy(void)
{
	if (model_cache)
		delete [] model_cache;
	model_cache = NULL;
}

*/