#include "Ren_cache.h"
#include "Tex_main.h"
#include "Mdl_cache.h"
#include "Light_main.h"



poly_t *cache_polys;
extern poly_t *tpoly;
int used_polys;
extern light_info_t light;
extern bool			lights_available;

// !!! only map polys are cached !!!
#define POLY_CACHE_POLYS	1024
#define POLY_CACHE_ALLOCS	32


cpoly_t *free_polys = NULL;
cpoly_t* cache_allocs[POLY_CACHE_ALLOCS];
int		 num_cache_allocs = 0;


/*
=================
allocate some more polys
=================
*/
cpoly_t* poly_alloc(void)
{
	if (num_cache_allocs == POLY_CACHE_ALLOCS)
		FError("too many cache alloc's!  ** tell Ripper **");

	// free_polys must be NULL
	free_polys = new cpoly_t[POLY_CACHE_POLYS];
	if (!free_polys)	FError("mem for poly cache");

	// set the linkage
	for (int i=0; i<POLY_CACHE_POLYS-1; i++)
		free_polys[i].next = &free_polys[i+1];
	free_polys[i].next = NULL;

	cache_allocs[num_cache_allocs] = free_polys;
	num_cache_allocs++;
	cpoly_t *ret = free_polys;
	free_polys = free_polys->next;

	return ret;
}


/*
==================
get a poly from the pool
==================
*/
cpoly_t* get_poly(void)
{
	if (!free_polys)
		return poly_alloc();

	cpoly_t *ret = free_polys;
	free_polys = free_polys->next;
	return ret;
}



/*
==================
return a list of polys to the pool
==================
*/
void return_poly(cpoly_t *p)
{
	if (!p)
		return;

	cpoly_t *tmp = p;
	while (p->next)
		p = p->next;
	p->next = free_polys;
	free_polys = tmp;
}



/******************************************************************************
draw a poly
******************************************************************************/
void r_draw_world_poly(poly_t *p, dimension_t dim)
{
	float s, t;
	glBegin(GL_TRIANGLE_FAN);

	for (int v = 0; v < p->num_vertices; v++)
	{
		s = p->vertices[v].x * world->texdefs[p->texdef].vecs[0][0] + 
			p->vertices[v].y * world->texdefs[p->texdef].vecs[0][1] + 
			p->vertices[v].z * world->texdefs[p->texdef].vecs[0][2] + 
			world->texdefs[p->texdef].vecs[0][3];

		t = p->vertices[v].x * world->texdefs[p->texdef].vecs[1][0] + 
			p->vertices[v].y * world->texdefs[p->texdef].vecs[1][1] + 
			p->vertices[v].z * world->texdefs[p->texdef].vecs[1][2] + 
			world->texdefs[p->texdef].vecs[1][3];

		s /= dim[0];
		t /= dim[1];

		glTexCoord2f(s, t);
		glVertex3f(  p->vertices[v].x,
					 p->vertices[v].z, 
					-p->vertices[v].y);

	}
	glEnd();
}

void r_draw_light_poly(poly_t *p)
{
	if (p->lightdef == -1)
		return;

	float s, t;
	glBindTexture(GL_TEXTURE_2D, tex->light_names[p->lightdef]);

	glBegin(GL_TRIANGLE_FAN);
	for (int v = 0; v < p->num_vertices; v++)
	{
		s = p->vertices[v].x * world->lightdefs[p->lightdef].vecs[0][0] + 
			p->vertices[v].y * world->lightdefs[p->lightdef].vecs[0][1] + 
			p->vertices[v].z * world->lightdefs[p->lightdef].vecs[0][2] + 
			world->lightdefs[p->lightdef].vecs[0][3];

		t = p->vertices[v].x * world->lightdefs[p->lightdef].vecs[1][0] + 
			p->vertices[v].y * world->lightdefs[p->lightdef].vecs[1][1] + 
			p->vertices[v].z * world->lightdefs[p->lightdef].vecs[1][2] + 
			world->lightdefs[p->lightdef].vecs[1][3];

		glTexCoord2f(s, t);
		glVertex3f(  p->vertices[v].x,
					 p->vertices[v].z, 
					-p->vertices[v].y);

	}
	glEnd();
}


void r_draw_multi_poly(poly_t *p, dimension_t dim)
{
	// skip polys without a lightmap
	if (p->lightdef == -1)
		return;

	float s, t, ls, lt;

	glBindTexture(GL_TEXTURE_2D, tex->light_names[p->lightdef]);

	glBegin(GL_TRIANGLE_FAN);
	for (int v = 0; v < p->num_vertices; v++)
	{
		// world tex coords
		s = p->vertices[v].x * world->texdefs[p->texdef].vecs[0][0] + 
			p->vertices[v].y * world->texdefs[p->texdef].vecs[0][1] + 
			p->vertices[v].z * world->texdefs[p->texdef].vecs[0][2] + 
			world->texdefs[p->texdef].vecs[0][3];

		t = p->vertices[v].x * world->texdefs[p->texdef].vecs[1][0] + 
			p->vertices[v].y * world->texdefs[p->texdef].vecs[1][1] + 
			p->vertices[v].z * world->texdefs[p->texdef].vecs[1][2] + 
			world->texdefs[p->texdef].vecs[1][3];

		s /= dim[0];
		t /= dim[1];

		// lightmap tex coords
		ls = p->vertices[v].x * world->lightdefs[p->lightdef].vecs[0][0] + 
			 p->vertices[v].y * world->lightdefs[p->lightdef].vecs[0][1] + 
			 p->vertices[v].z * world->lightdefs[p->lightdef].vecs[0][2] + 
			 world->lightdefs[p->lightdef].vecs[0][3];

		lt = p->vertices[v].x * world->lightdefs[p->lightdef].vecs[1][0] + 
			 p->vertices[v].y * world->lightdefs[p->lightdef].vecs[1][1] + 
			 p->vertices[v].z * world->lightdefs[p->lightdef].vecs[1][2] + 
			 world->lightdefs[p->lightdef].vecs[1][3];


		glMultiTexCoord2fARB(GL_TEXTURE0_ARB,  s,  t);
		glMultiTexCoord2fARB(GL_TEXTURE1_ARB, ls, lt);

		glVertex3f(  p->vertices[v].x,
					 p->vertices[v].z, 
					-p->vertices[v].y);

	}
	glEnd();
}


/******************************************************************************
clear out all cached polys
******************************************************************************/
void cache_purge_single(void)
{
	//
	// single texture / multi pass rendering
	//
	for (int pass=0; pass<2; pass++)
	{
		// set blending mode for this pass
		switch (pass)
		{
		case 0:	// world textures
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			glDisable(GL_BLEND);
			break;

		case 1:	// lightmaps
			glDisable(GL_DEPTH_TEST);
			if ((world->nlightdefs && world->light_size) && !(rInfo->rflags&RFLAG_FULLBRIGHT))
			{
				glEnable(GL_BLEND);
				glBlendFunc(GL_ZERO, GL_SRC_COLOR);
			}
			else
				continue;	// lighting not available or in fullbright
			break;
		}


		for(int t=0; t<tex->num_textures; t++)
		{
			// only bind if we have a poly that uses it
			if (!tex->polycaches[t])
				continue;

			// only bind world texture once for all polys that use it
			if (pass == 0)
				glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);


			for (cpoly_t *p=tex->polycaches[t]; p; p=p->next)
			{
				if (pass == 0)
					r_draw_world_poly(&p->poly, tex->dims[t]);
				else if (pass == 1)
					r_draw_light_poly(&p->poly);
			}
		}
	}

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


void cache_purge_multi(void)
{
	//
	// arb multi texturing
	//


	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_ALWAYS);

	// world textures are unit 0, lightmaps are unit 1
	// set up blending
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glDisable(GL_BLEND);

	glActiveTextureARB(GL_TEXTURE1_ARB);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_BLEND);


	for(int t=0; t<tex->num_textures; t++)
	{
		if (!tex->polycaches[t])
			continue;

		// only bind world texture once for all polys that use it
		glActiveTextureARB(GL_TEXTURE0_ARB);
		glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);

		glActiveTextureARB(GL_TEXTURE1_ARB);

		for (cpoly_t *p=tex->polycaches[t]; p; p=p->next)
			r_draw_multi_poly(&p->poly, tex->dims[t]);
	}

	// turn unit 1 back off
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

}


extern CVar *  g_pMultiTexture;
void cache_purge(void)
{

	if (g_pMultiTexture->value && (rInfo->rflags&RFLAG_MULTITEXTURE) && 
		!(rInfo->rflags&RFLAG_FULLBRIGHT) && world->nlightdefs && world->light_size)
		cache_purge_multi();
	else
		cache_purge_single();


	// free all the polys that are cached
	for(int t=0; t<tex->num_textures; t++)
	{
		return_poly(tex->polycaches[t]);
		tex->polycaches[t] = NULL;
	}

	model_cache_purge();
}


/******************************************************************************
add a poly to be drawn
******************************************************************************/
void cache_add_poly(cpoly_t *poly)
{
	poly->next = tex->polycaches[world->texdefs[poly->poly.texdef].texture];
	tex->polycaches[world->texdefs[poly->poly.texdef].texture] = poly;
}


/******************************************************************************
free mem from the whole cache
******************************************************************************/
void cache_destroy(void)
{
	for (int i=0; i<num_cache_allocs; i++)
		delete [] cache_allocs[i];

	num_cache_allocs = 0;
	free_polys = NULL;
}




