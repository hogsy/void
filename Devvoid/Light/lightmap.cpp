#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "light.h"
#include "Com_world.h"


extern	CWorld		*world;
extern	lightmap_t	*lightmaps[65536];	// max brush sides
extern	int			num_lightmaps;


/*
==========
lightmap_square - make the 3d rectangle the lightmap covers
==========
*/
void lightmap_square(lightmap_t *l, bspf_side_t *s)
{
	// find the longest side
	int v, nv;
	float	len, blen = -1;
	int		best = -1;
	vector_t side;

	for (v=0; v<s->num_verts; v++)
	{
		nv = (v+1) % s->num_verts;

		VectorSub(world->verts[world->iverts[nv+s->first_vert]], world->verts[world->iverts[v+s->first_vert]], side);
		len = VectorLength(&side);
		if (len > blen)
		{
			blen = len;
			best = v;
		}
	}

	if (best == -1)
		Error("no best side in lightmap_square()");


	// right vector
	VectorSub(world->verts[world->iverts[(best+1)%s->num_verts+s->first_vert]], world->verts[world->iverts[best+s->first_vert]], l->right);

	// down vector
	_CrossProduct(&l->right, &world->planes[s->plane].norm, &l->down);

	VectorNormalize(&l->right);
	VectorNormalize(&l->down);

	l->lheight = l->lwidth = 0;
	VectorCopy(world->verts[world->iverts[best+s->first_vert]], l->origin);

	float dt, ds;
	dt = dot(l->origin, l->down);
	ds = dot(l->origin, l->right);

	// add all the points of the side to the square - make it grow as needed
	for (v=0; v<s->num_verts; v++)
	{
	// top plane
		float dist = dot(world->verts[world->iverts[v+s->first_vert]], l->down) - dt;

		// move the plane back
		if (dist < 0)
		{
			VectorMA(&l->origin, dist, &l->down, &l->origin);
			dt += dist;
		}
		// increase height?
		else
		{
			if (dist > l->lheight)
				l->lheight = dist;
		}

	// side plane
		dist = dot(world->verts[world->iverts[v+s->first_vert]], l->right) - ds;

		// move the plane back
		if (dist < 0)
		{
			VectorMA(&l->origin, dist, &l->right, &l->origin);
			ds += dist;
		}
		// increase width?
		else
		{
			if (dist > l->lwidth)
				l->lwidth = dist;
		}
	}
}


/*
============
lightmap_build - create data for all lightmaps
============
*/
void lightmap_build_leaf(int leaf)
{
	int endb = world->leafs[leaf].first_brush + world->leafs[leaf].num_brushes;
	for (int b=world->leafs[leaf].first_brush; b<endb; b++)
	{
		int ends = world->brushes[b].first_side + world->brushes[b].num_sides;
		for (int s=world->brushes[b].first_side; s<ends; s++)
		{
			lightmap_t *l;
			l = (lightmap_t*)malloc(sizeof(lightmap_t));
			if (!l)
				Error("not enough mem for lightmaps");
			

			// find the rectangle the lightmap covers in 3d space
			lightmap_square(l, &world->sides[s]);

			// if it's too small, skip - should never happen - small sides should be removed in bsp
			if ((l->lwidth < 0.1f) || (l->lheight < 0.1f))
			{
				ComPrintf("side too small for lightmap\n");
				continue;
			}

			// find the dimensions of the lightmap we need
			if		(l->lwidth > (16*16))	l->width = 32;
			else if (l->lwidth > ( 8*16))	l->width = 16;
			else if (l->lwidth > ( 4*16))	l->width = 8;
			else if (l->lwidth > ( 2*16))	l->width = 4;
			else if (l->lwidth > ( 1*16))	l->width = 2;
			else							l->width = 1;

			if		(l->lheight > (16*16))	l->height = 32;
			else if (l->lheight > ( 8*16))	l->height = 16;
			else if (l->lheight > ( 4*16))	l->height = 8;
			else if (l->lheight > ( 2*16))	l->height = 4;
			else if (l->lheight > ( 1*16))	l->height = 2;
			else							l->height = 1;


			// set the lightmap to the ambient color
			for (int p=0; p<32*32*3; p++)
				l->data[p] = g_ambient[p%3];

			// track which one we use
			world->sides[s].lightdef = num_lightmaps;

			l->leaf = leaf;
			l->side = s;

			lightmaps[num_lightmaps] = l;
			num_lightmaps++;
		}
	}
}

void lightmap_build(int node)
{
	for (int i=0; i<2; i++)
	{
		if (world->nodes[node].children[i] > 0)
			lightmap_build(world->nodes[node].children[i]);
		else
			lightmap_build_leaf(-world->nodes[node].children[i]);
	}
}
