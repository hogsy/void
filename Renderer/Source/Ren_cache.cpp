#include "Ren_cache.h"
#include "Tex_hdr.h"
#include "Mdl_cache.h"



poly_t *cache_polys;
extern poly_t *tpoly;
int used_polys;
extern bool			lights_available;

// !!! only map polys are cached !!!
#ifdef _DEBUG
#define POLY_CACHE_POLYS	(1024/512)
#define POLY_CACHE_ALLOCS	(32*512)
#else
#define POLY_CACHE_POLYS	1024
#define POLY_CACHE_ALLOCS	32
#endif

cpoly_t *free_polys = NULL;
cpoly_t* cache_allocs[POLY_CACHE_ALLOCS];
int		 num_cache_allocs = 0;

// FIXME - put in a different struct
vector_t *fullblend;


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
	if (!free_polys)
		FError("mem for poly cache - %d allocated", POLY_CACHE_POLYS*num_cache_allocs);

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
	g_pRast->PolyStart(VRAST_TRIANGLE_FAN);

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

		g_pRast->PolyTexCoord(s, t);
		g_pRast->PolyVertexf(p->vertices[v]);
	}

	g_pRast->PolyEnd();
}

void r_draw_light_poly(poly_t *p)
{

	if (p->lightdef == -1)
		return;

	float s, t;
	g_pRast->TextureSet(tex->bin_light, p->lightdef);
	g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
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

		g_pRast->PolyTexCoord(s, t);
		g_pRast->PolyVertexf(p->vertices[v]);
	}
	g_pRast->PolyEnd();
}


void r_draw_multi_poly(poly_t *p, dimension_t dim)
{
/*
	// skip polys without a lightmap
	if (p->lightdef == -1)
		return;

	float s, t, ls, lt;

	g_pRast->TextureSet(tex->bin_light, p->lightdef);
	g_pRast->PolyStart();

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
*/
}


/******************************************************************************
clear out all cached polys
******************************************************************************/
void cache_purge_single()
{
	bool lightmaps = ((world->nlightdefs && world->light_size) && !(g_rInfo.rflags&RFLAG_FULLBRIGHT));

	g_pRast->PolyColor4f(fullblend->x, fullblend->y, fullblend->z, 1);

	//
	// single texture / multi pass rendering
	//
	for (int pass=0; pass<2*CACHE_PASS_NUM; pass++)
	{
		// set blending mode for this pass
		switch (pass)
		{
		// zfill
		case CACHE_PASS_ZFILL*2:		// texture
			g_pRast->DepthFunc(VRAST_DEPTH_FILL);
			g_pRast->BlendFunc(VRAST_SRC_BLEND_NONE, VRAST_DEST_BLEND_NONE);
			break;


		case CACHE_PASS_ZFILL*2+1:		// lightmap
			if (lightmaps)
			{
				g_pRast->DepthFunc(VRAST_DEPTH_NONE);
				g_pRast->BlendFunc(VRAST_SRC_BLEND_ZERO, VRAST_DEST_BLEND_SRC_COLOR);
			}
			else
				continue;	// lighting not available or in fullbright
			break;


		// zbuffer
		case CACHE_PASS_ZBUFFER*2:		// texture
			g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
			g_pRast->BlendFunc(VRAST_SRC_BLEND_NONE, VRAST_DEST_BLEND_NONE);
			break;

		case CACHE_PASS_ZBUFFER*2+1:	// lightmap
			if (lightmaps)
			{
				g_pRast->BlendFunc(VRAST_SRC_BLEND_ZERO, VRAST_DEST_BLEND_SRC_COLOR);
				g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
			}
			else
				continue;	// lighting not available or in fullbright
			break;


		// alphablend
		case CACHE_PASS_ALPHABLEND*2:		// texture
			g_pRast->BlendFunc(VRAST_SRC_BLEND_SRC_ALPHA, VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA);
			g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
			g_pRast->DepthWrite(false);

			if (lightmaps)
				g_pRast->PolyColor4f(fullblend->x, fullblend->y, fullblend->z, 0.2f);
			else
				g_pRast->PolyColor4f(fullblend->x, fullblend->y, fullblend->z, 0.4f);
			break;

		case CACHE_PASS_ALPHABLEND*2+1:		// lightmap
			if (lightmaps)
			{
				g_pRast->BlendFunc(VRAST_SRC_BLEND_ZERO, VRAST_DEST_BLEND_SRC_COLOR);
				g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
			}
			else
				continue;	// lighting not available or in fullbright
			break;

		default:
			continue;
		}


		for(uint t=0; t<g_pRast->TextureCount(tex->bin_world); t++)
		{
			// only bind if we have a poly that uses it
			if (!tex->polycaches[pass/2][t])
				continue;

			// only bind world texture once for all polys that use it
			if (pass%2 == 0)
				g_pRast->TextureSet(tex->bin_world, t);

			for (cpoly_t *p=tex->polycaches[pass/2][t]; p; p=p->next)
			{
				if (pass%2 == 0)
					r_draw_world_poly(&p->poly, tex->dims[t]);
				else
					r_draw_light_poly(&p->poly);
			}
		}
	}

	g_pRast->DepthWrite(true);
}


void cache_purge_multi()
{
	//
	// arb multi texturing
	//
	
/*
	// world textures are unit 0, lightmaps are unit 1
	// set up blending

	g_pRast->PolyColor4(*fullblend, 1);

	for (int pass=0; pass<CACHE_PASS_NUM; pass++)
	{
		// set blending mode for this pass
		switch (pass)
		{
		// zfill
		case CACHE_PASS_ZFILL:
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glEnable(GL_TEXTURE_2D);
			glDisable(GL_BLEND);

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);

			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_ALWAYS);
			break;

		// zbuffer
		case CACHE_PASS_ZBUFFER:
			glEnable(GL_DEPTH_TEST);
			glDepthFunc(GL_LEQUAL);
			break;

		// alphablend
		case CACHE_PASS_ALPHABLEND:

			glActiveTextureARB(GL_TEXTURE0_ARB);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);

			glActiveTextureARB(GL_TEXTURE1_ARB);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glEnable(GL_BLEND);


			glColor4f(fullblend->x, fullblend->y, fullblend->z, 0.2f);

			break;

		default:
			continue;
		}

		for(uint t=0; t<tex->num_textures; t++)
		{
			if (!tex->polycaches[pass][t])
				continue;

			// only bind world texture once for all polys that use it
			glActiveTextureARB(GL_TEXTURE0_ARB);
			glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);

			glActiveTextureARB(GL_TEXTURE1_ARB);

			for (cpoly_t *p=tex->polycaches[pass][t]; p; p=p->next)
				r_draw_multi_poly(&p->poly, tex->dims[t]);
		}
	}


	// turn unit 1 back off
	glActiveTextureARB(GL_TEXTURE1_ARB);
	glDisable(GL_TEXTURE_2D);
	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);

*/
}


//extern CVar  g_pMultiTexture;
void cache_purge(void)
{
	bool multitex = ((g_rInfo.rflags&RFLAG_MULTITEXTURE) && 
					!(g_rInfo.rflags&RFLAG_FULLBRIGHT) 
					&& world->nlightdefs && world->light_size);

//	if (multitex)
//		cache_purge_multi();
//	else
		cache_purge_single();


	// free all the polys that are cached
	for (int p=0; p<CACHE_PASS_NUM; p++)
	{
		for(uint t=0; t<g_pRast->TextureCount(tex->bin_world); t++)
		{
			return_poly(tex->polycaches[p][t]);
			tex->polycaches[p][t] = NULL;
		}
	}

	// fixme - put this in the cache_purge_* funcs
	model_cache_purge();
}


/******************************************************************************
add a poly to be drawn
******************************************************************************/
void cache_add_poly(cpoly_t *poly, int pass)
{
	poly->next = tex->polycaches[pass][world->texdefs[poly->poly.texdef].texture];
	tex->polycaches[pass][world->texdefs[poly->poly.texdef].texture] = poly;
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




