
#include "Mdl_main.h"
#include "Mdl_cache.h"

#define MAX_CACHED_MODELS	256
model_cache_t *model_cache;
model_cache_t *tmodel;
int used_models;



/************************************************
purge the model cache
************************************************/
void model_cache_purge(void)
{

	glDisable(GL_BLEND);
	glDepthFunc(GL_LEQUAL);

	for (int m=0; m<used_models; m++)
	{
		// fixme - set skin here!!
		glPushMatrix();
		glTranslatef(model_cache[m].origin.x,
					 model_cache[m].origin.z,
					-model_cache[m].origin.y);
		glRotatef( model_cache[m].angles.ROLL  * 180/PI, 0, 0, 1);
		glRotatef(-model_cache[m].angles.PITCH * 180/PI, 1, 0, 0);
		glRotatef( model_cache[m].angles.YAW   * 180/PI, 0, 1, 0);

		model_draw(model_cache[m].model_num, model_cache[m].frame);

		glPopMatrix();
	}

	used_models = 0;
	tmodel = &model_cache[0];

	glDepthFunc(GL_ALWAYS);
	glEnable(GL_BLEND);
}


/************************************************
add a model to the cache
************************************************/
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
************************************************/
void model_cache_init(void)
{
	model_cache = (model_cache_t*) MALLOC(sizeof(model_cache_t) * MAX_CACHED_MODELS);
	if (!model_cache) FError("mem for model cache!");

	used_models = 0;
	tmodel = &model_cache[0];

}


/************************************************
destroy the model cache
************************************************/
void model_cache_destroy(void)
{
	if (model_cache)
		free (model_cache);
	model_cache = NULL;
}

