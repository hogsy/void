
#include "Mdl_main.h"
#include "Mdl_cache.h"

#define MAX_CACHED_MODELS	256
////model_cache_t *model_cache;
//model_cache_t *tmodel;
int used_models;



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