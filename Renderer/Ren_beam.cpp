
#include "Standard.h"
#include "Shader.h"
#include "ShaderManager.h"
#include "ren_cache.h"

extern	vector_t	forward, right, up;	// view directions
extern	plane_t frust[5];				// frustum

//extern	eyepoint_t	eye;				// where we're gonna draw from
extern const CCamera * camera;

extern	CVar *	g_pBeamTolerance;

vector_t lines[1024][2];
int		 num_lines;


/*
===========
defs only needed for beam tree
===========
*/

#define BEAM_SOLID_FRONT	1
#define BEAM_SOLID_BACK		2

#ifdef _DEBUG
#define MAX_SIL_ALLOCS		(8*32)
#define SILS_PER_ALLOC		(32/32)
#define MAX_BEAM_ALLOCS		(16*8)
#define BEAMS_PER_ALLOC		(128/8)
#else
#define MAX_SIL_ALLOCS		8
#define SILS_PER_ALLOC		32
#define MAX_BEAM_ALLOCS		16
#define BEAMS_PER_ALLOC		128
#endif



struct beam_node_t
{
	plane_t	p;
	int		stat;
	beam_node_t * children[2];
};


struct sil_t
{
	vector_t		edges[32][2];
	int				nedges;
	vector_t		center;
	float			area;
	bool			sky;
	cpoly_t			*polys;
	sil_t			*next;
};

beam_node_t *beam_head=NULL;

//==========================================

/*
=========
get_sil - allocate a sil
=========
*/

int		sil_allocs = 0;
sil_t	*sil_a[MAX_SIL_ALLOCS];
sil_t	*free_sils=NULL;

void sil_alloc(void)
{
	if (sil_allocs == MAX_SIL_ALLOCS)
		FError("too many sil allocs! Tell Ripper\n");

	sil_a[sil_allocs] = new sil_t[SILS_PER_ALLOC];
	if (!sil_a[sil_allocs])
		FError("not enough mem for sils! - %d allocated", SILS_PER_ALLOC*sil_allocs);

	free_sils = sil_a[sil_allocs];
	sil_allocs++;

	for (int a=0; a<SILS_PER_ALLOC-1; a++)
		free_sils[a].next = &free_sils[a+1];
	free_sils[a].next = NULL;
}


sil_t* get_sil(void)
{
	if (!free_sils)
		sil_alloc();

	sil_t *ret = free_sils;
	free_sils = free_sils->next;
	return ret;
}


/*
=========
free_sil - return the sil to the list
=========
*/
void free_sil(sil_t *s)
{
	// free all polys
	g_pShaders->ReturnPoly(s->polys);

	s->next = free_sils;
	free_sils = s;
}


/*
=========
get_beam - allocate a beam tree node
=========
*/

int			beam_allocs = 0;
beam_node_t	*beam_a[MAX_BEAM_ALLOCS];
beam_node_t	*free_beams=NULL;

void beam_alloc(void)
{
	if (beam_allocs == MAX_BEAM_ALLOCS)
		FError("too many sil allocs! Tell Ripper\n");

	beam_a[beam_allocs] = new beam_node_t[BEAMS_PER_ALLOC];
	if (!beam_a[beam_allocs])
		FError("not enough mem for beam nodes! - %d allocated", BEAMS_PER_ALLOC*beam_allocs);

	free_beams = beam_a[beam_allocs];
	beam_allocs++;

	for (int a=0; a<BEAMS_PER_ALLOC-1; a++)
		free_beams[a].children[0] = &free_beams[a+1];
	free_beams[a].children[0] = NULL;
}


beam_node_t* get_beam(void)
{
	if (!free_beams)
		beam_alloc();

	beam_node_t *ret = free_beams;
	free_beams = free_beams->children[0];
	return ret;
}


/*
=========
free_beam - return the beam node to the list
=========
*/
void free_beam(beam_node_t *n)
{
	n->children[0] = free_beams;
	free_beams = n;
}

// return a whole tree
void free_beam_t(beam_node_t *n)
{
	if (!n)
		return;

	free_beam_t(n->children[0]);
	free_beam_t(n->children[1]);
	free_beam(n);
}


/*
=========
beam_shutdown - free all mem associated with the beam tree
=========
*/
void beam_shutdown(void)
{
	int i;
	for (i=0; i<sil_allocs; i++)
		delete [] sil_a[i];

	for (i=0; i<beam_allocs; i++)
		delete [] beam_a[i];

	
	sil_allocs = 0;
	beam_allocs= 0;

	beam_head = NULL;
	free_beams= NULL;
	free_sils = NULL;
}


//====================================================================


/*
==========
sil_get_sky_polys
==========
*/
void sil_get_sky_polys(sil_t *s)
{
	vector_t clipverts[33];
	float dists[33];
	int sides[33];
	int num_clipverts;

	// return any polys we may have gotten
	g_pShaders->ReturnPoly(s->polys);
	s->polys = NULL;

	// add each of the sky polys and clip to the sil
	int endside = world->brushes[0].first_side + world->brushes[0].num_sides;
	for (int side=world->brushes[0].first_side; side<endside; side++)
	{
		cpoly_t *poly = g_pShaders->GetPoly();
		poly->num_vertices = world->sides[side].num_verts;
		poly->texdef		= world->sides[side].texdef;
		poly->lightdef		= world->sides[side].lightdef;

		for (int v=0; v<poly->num_vertices; v++)
		{
//			VectorAdd(eye.origin, world->verts[world->iverts[world->sides[side].first_vert+v]], poly->vertices[v]);
			VectorAdd(camera->origin, world->verts[world->iverts[world->sides[side].first_vert+v]], poly->vertices[v]);
		}

		// clip to each sil edge
		for (int edge=0; edge<s->nedges; edge++)
		{
			plane_t plane;
			vector_t a, b;
			VectorSub(s->edges[edge][0], camera->origin, a);
			VectorSub(s->edges[edge][1], camera->origin, b);

			_CrossProduct(&a, &b, &plane.norm);
			VectorNormalize(&plane.norm);
//			plane.d = dot(plane.norm, eye.origin);
			plane.d = dot(plane.norm, camera->origin);

			if ((dot(plane.norm, s->center) - plane.d) <0)
			{
				plane.d = -plane.d;
				VectorScale(&plane.norm, -1, &plane.norm);
			}

			bool allfront = true;
			bool allback = true;

			for (int i=0; i<poly->num_vertices; i++)
			{
				dists[i] = dot(plane.norm, poly->vertices[i]) - plane.d;
				
				if (dists[i] > 0.01f)
				{
					sides[i] = 1;
					allback = false;
				}
				else if (dists[i] < -0.01f)
				{
					sides[i] = -1;
					allfront = false;
				}
				else
					sides[i] = 0;
			}

			if (allfront)
				continue;
			if (allback)
			{
				poly->next = NULL;
				g_pShaders->ReturnPoly(poly);
				break;
			}

			num_clipverts = 0;

			dists[i] = dists[0];
			sides[i] = sides[0];

			for (i=0; i<poly->num_vertices; i++)
			{
				if (sides[i] == 0)
				{
					VectorCopy(poly->vertices[i], clipverts[num_clipverts]);
					num_clipverts++;
					continue;
				}

				if (sides[i] == 1)
				{
					VectorCopy(poly->vertices[i], clipverts[num_clipverts]);
					num_clipverts++;
				}

				if ((sides[i+1] == 0) || (sides[i] == sides[i+1]))
					continue;

				vector_t *nextvert = &poly->vertices[(i+1)%poly->num_vertices];
				double frac = dists[i] / (dists[i]-dists[i+1]);
				clipverts[num_clipverts].x = (float)(poly->vertices[i].x + frac*(nextvert->x - poly->vertices[i].x));
				clipverts[num_clipverts].y = (float)(poly->vertices[i].y + frac*(nextvert->y - poly->vertices[i].y));
				clipverts[num_clipverts].z = (float)(poly->vertices[i].z + frac*(nextvert->z - poly->vertices[i].z));
				num_clipverts++;
			}

			// copy everything back
			if (num_clipverts > 32)
				num_clipverts = 32;
			memcpy(poly->vertices, clipverts, sizeof(vector_t) * num_clipverts);
			poly->num_vertices = num_clipverts;
		}

		if (edge == s->nedges)
		{
			poly->next = s->polys;
			s->polys = poly;
		}
	}
}


/*
==========
sil_build - create a sil from the brush.
==========
*/
sil_t* sil_build(bspf_brush_t *b)
{
	sil_t *sil = get_sil();
	sil->nedges = 0;
	sil->area = 0;
	sil->sky = false;
	sil->polys = NULL;
	sil->next = NULL;
	VectorSet(&sil->center, 0, 0, 0);

	cpoly_t *poly;

	// determine which direction each face is facing
	bool facing[64];
	for (int s=0; s<b->num_sides; s++)
	{
		plane_t *p = &world->planes[world->sides[s+b->first_side].plane];
//		float d = dot(p->norm, eye.origin) - p->d;
		float d = dot(p->norm, camera->origin) - p->d;

		// view origin on frontside of poly
		if (d > 0.1f)
		{
			facing[s] = true;

			// add area
			d = -dot(p->norm, forward);
			if (d>0)
				sil->area += world->sides[s+b->first_side].area * d;
			else
				sil->area -= world->sides[s+b->first_side].area * d;

//			// use sky polys instead if it's a sky brush
			if (world->sides[s+b->first_side].flags & SURF_SKY)
				sil->sky = true;

			else
			{
				// add facing polys to the list
				poly = g_pShaders->GetPoly();
				poly->num_vertices = world->sides[s+b->first_side].num_verts;
				poly->texdef		= world->sides[s+b->first_side].texdef;
				poly->lightdef		= world->sides[s+b->first_side].lightdef;

				for (int v=0; v<poly->num_vertices; v++)
				{
					VectorCopy(world->verts[world->iverts[world->sides[s+b->first_side].first_vert+v]], poly->vertices[v]);
				}
				poly->next = sil->polys;
				sil->polys = poly;
			}
		}

		else
			facing[s] = false;
	}


	// if our sil area is small enough, just add it to the zbuffer list 
	// would take longer to pass through the beam tree
	if (!sil->sky && (sil->area < g_pBeamTolerance->fval))
	{
		cpoly_t *next;
		for (; sil->polys; sil->polys = next)
		{
			next = sil->polys->next;
			sil->polys->forcez = true;
			g_pShaders->CacheAdd(sil->polys);
		}

		free_sil(sil);
		return NULL;
	}



	// add all edges that we need
	for (int e=0; e<b->num_edges; e++)
	{
		bspf_edge_t *edge = &world->edges[e+b->first_edge];

		// if we have an edge with one side or we have opposite facing sides
		bool add = false;

		if (edge->sides[1] == -1)
		{
			if (facing[edge->sides[0]-b->first_side])
				add = true;
		}
		else if (facing[edge->sides[0]-b->first_side] == !facing[edge->sides[1]-b->first_side])
			add = true;
		if (add)
		{
			VectorCopy(world->verts[edge->verts[0]], sil->edges[sil->nedges][0]);
			VectorCopy(world->verts[edge->verts[1]], sil->edges[sil->nedges][1]);
			sil->nedges++;

			VectorAdd(sil->center, world->verts[edge->verts[0]], sil->center);
			VectorAdd(sil->center, world->verts[edge->verts[1]], sil->center);
		}
	}

	if (sil->nedges == 32)
		sil->nedges = 32;

	if (sil->nedges)
	{
		VectorScale(&sil->center, 1.0f/(2*sil->nedges), &sil->center);

		// if any one of the polys was a sky surf make this sil a sky sil
		if (sil->sky)
			sil_get_sky_polys(sil);

		return sil;
	}


	free_sil(sil);
	return NULL;
}




/*
=======
sil_split
=======
*/
void sil_split_polys(cpoly_t *base, plane_t *p, cpoly_t **front, cpoly_t **back)
{
	if (!base)
		return;

	// do a recursive split down the chain
	sil_split_polys(base->next, p, front, back);
	base->next = NULL;

	// split this one

	bool allfront = true;
	bool allback  = true;

	float	dists[33];
	int		sides[33];

	int v;
	for (v=0; v<base->num_vertices; v++)
	{
		dists[v] = dot(base->vertices[v], p->norm) - p->d;

		if (dists[v] > 0.01f)
		{
			allback = false;
			sides[v] = 1;
		}
		else if (dists[v] < -0.01f)
		{
			allfront = false;
			sides[v] = -1;
		}
		else
			sides[v] = 0;
	}

	// quick out
	if (allfront)
	{
		base->next = *front;
		*front = base;
		return;
	}
	if (allback)
	{
		base->next = *back;
		*back = base;
		return;
	}

	// else we actually have to split it
	dists[v] = dists[0];
	sides[v] = sides[0];

	cpoly_t *tmp = g_pShaders->GetPoly();
	tmp->next = *front;
	*front = tmp;

	tmp = g_pShaders->GetPoly();
	tmp->next = *back;
	*back = tmp;

	(*front)->num_vertices = (*back)->num_vertices = 0;
	(*front)->lightdef		= (*back)->lightdef	 = base->lightdef;
	(*front)->texdef		= (*back)->texdef		 = base->texdef;

	for (v=0; v<base->num_vertices; v++)
	{
		if (sides[v] == 0)
		{
			VectorCopy(base->vertices[v], (*front)->vertices[(*front)->num_vertices]);
			VectorCopy(base->vertices[v], (*back )->vertices[(*back )->num_vertices]);
			(*front)->num_vertices++;
			(*back )->num_vertices++;
			continue;
		}

		if (sides[v] == 1)
		{
			VectorCopy(base->vertices[v], (*front)->vertices[(*front)->num_vertices]);
			(*front)->num_vertices++;
		}

		else if (sides[v] == -1)
		{
			VectorCopy(base->vertices[v], (*back )->vertices[(*back )->num_vertices]);
			(*back )->num_vertices++;
		}

		if ((sides[v+1] == 0) || (sides[v] == sides[v+1]))
			continue;

		// generate a split point
		vector_t inter;
		int nv = (v+1) % base->num_vertices;
		float frac = dists[v] / (dists[v]-dists[v+1]);

		inter.x = base->vertices[v].x + frac*(base->vertices[nv].x - base->vertices[v].x);
		inter.y = base->vertices[v].y + frac*(base->vertices[nv].y - base->vertices[v].y);
		inter.z = base->vertices[v].z + frac*(base->vertices[nv].z - base->vertices[v].z);

		VectorCopy(inter, (*front)->vertices[(*front)->num_vertices]);
		VectorCopy(inter, (*back )->vertices[(*back )->num_vertices]);
		(*front)->num_vertices++;
		(*back )->num_vertices++;
	}

	// these should never happen
	if ((*front)->num_vertices < 3)
	{
		tmp = *front;
		*front = tmp->next;
		tmp->next = NULL;
		g_pShaders->ReturnPoly(tmp);
	}

	if ((*back)->num_vertices < 3)
	{
		tmp = *back;
		*back = tmp->next;
		tmp->next = NULL;
		g_pShaders->ReturnPoly(tmp);
	}

	base->next = NULL;
	g_pShaders->ReturnPoly(base);
}



void sil_split(sil_t *base, plane_t *p, sil_t **front, sil_t **back)
{
	float	dists[32][2];
	int		sides[32][2];

	int e;

	// find all dists/sides
	for (e=0; e<base->nedges; e++)
	{
		for (int i=0; i<2; i++)
		{
			dists[e][i] = dot(p->norm, base->edges[e][i]) - p->d;

			if (dists[e][i] < -0.01f)
				sides[e][i] = -1;
			else if (dists[e][i] > 0.01f)
				sides[e][i] = 1;
			else
				sides[e][i] = 0;
		}
	}
/*
	// split polys first cause they can allow a fast out
	cpoly_t *fpoly = NULL;
	cpoly_t *bpoly = NULL;
	sil_split_polys(base->polys, p, &fpoly, &bpoly);

	// no polys mean there is no sil on that side
	if (!fpoly)
	{
		*front = NULL;
		*back  = base;
		base->polys = bpoly;
		return;
	}

	if (!bpoly)
	{
		*back  = NULL;
		*front = base;
		base->polys = fpoly;
		return;
	}

	// allocate new sils - we're gonna have to split em
	base->polys = NULL;
*/	*front = get_sil();
	*back  = get_sil();
	(*front)->nedges = (*back)->nedges = 0;
	(*front)->polys  = NULL; //fpoly;
	(*back )->polys  = NULL; //bpoly;
	(*front)->sky = base->sky;
	(*back)->sky = base->sky;
	VectorCopy(base->center, (*front)->center);
	VectorCopy(base->center, (*back )->center);

	for (e=0; e<base->nedges; e++)
	{
		// coplanar - neither side gets it
		if ((sides[e][0] == 0) && (sides[e][1] == 0))
		{
			continue;
		}

		// we're crossing the plane
		else if ((sides[e][0] + sides[e][1]) == 0)
		{
			vector_t inter;
			float frac = dists[e][0] / (dists[e][0] - dists[e][1]);

			inter.x = base->edges[e][0].x + frac * (base->edges[e][1].x - base->edges[e][0].x);
			inter.y = base->edges[e][0].y + frac * (base->edges[e][1].y - base->edges[e][0].y);
			inter.z = base->edges[e][0].z + frac * (base->edges[e][1].z - base->edges[e][0].z);

			// doesn't matter which direction the edges go
			VectorCopy(inter, (*front)->edges[(*front)->nedges][0]);
			VectorCopy(inter, (*back )->edges[(*back )->nedges][0]);

			if (sides[e][0] == 1)
			{
				VectorCopy(base->edges[e][0], (*front)->edges[(*front)->nedges][1]);
				VectorCopy(base->edges[e][1], (*back )->edges[(*back )->nedges][1]);
			}

			else
			{
				VectorCopy(base->edges[e][1], (*front)->edges[(*front)->nedges][1]);
				VectorCopy(base->edges[e][0], (*back )->edges[(*back )->nedges][1]);
			}

			(*front)->nedges++;
			(*back)->nedges++;
		}

		// all on backside
		else if ((sides[e][0] == -1) || (sides[e][1] == -1))
		{
			VectorCopy(base->edges[e][0], (*back)->edges[(*back)->nedges][0]);
			VectorCopy(base->edges[e][1], (*back)->edges[(*back)->nedges][1]);
			(*back)->nedges++;
		}

		// all on frontside
		else
		{
			VectorCopy(base->edges[e][0], (*front)->edges[(*front)->nedges][0]);
			VectorCopy(base->edges[e][1], (*front)->edges[(*front)->nedges][1]);
			(*front)->nedges++;
		}
	}

	// split polys first cause they can allow a fast out
	sil_split_polys(base->polys, p, &(*front)->polys, &(*back)->polys);
	base->polys = NULL;

	// no polys mean there is no sil on that side
	if (!(*front)->polys)
	{
		free_sil(*front);
		*front = NULL;
	}

	if (!(*back)->polys)
	{
		free_sil(*back);
		*back = NULL;
	}
	free_sil(base);
}



//====================================================================================


/*
========
beam_reset - set the tree to the frustum
========
*/
void beam_reset(void)
{
	beam_node_t *walk;

	free_beam_t(beam_head);

	beam_head = get_beam();
	walk = beam_head;
	walk->children[1] = NULL;
	VectorCopy(frust[0].norm, walk->p.norm);
	walk->p.d = frust[0].d;
	walk->stat = BEAM_SOLID_BACK;

	walk = walk->children[0];
	walk->children[0] = get_beam();
	walk->children[1] = NULL;
	VectorCopy(frust[1].norm, walk->p.norm);
	walk->p.d = frust[1].d;
	walk->stat = BEAM_SOLID_BACK;

	walk = walk->children[0];
	walk->children[0] = get_beam();
	walk->children[1] = NULL;
	VectorCopy(frust[2].norm, walk->p.norm);
	walk->p.d = frust[2].d;
	walk->stat = BEAM_SOLID_BACK;

	walk = walk->children[0];
	walk->children[0] = NULL;
	walk->children[1] = NULL;
	VectorCopy(frust[3].norm, walk->p.norm);
	walk->p.d = frust[3].d;
	walk->stat = BEAM_SOLID_BACK;

	num_lines = 0;
}


/*
=========
beam_leaf - make a leaf
=========
*/
void beam_leaf(beam_node_t *parent, int side, sil_t *sil)
{
	if (!sil->nedges)
	{
		// solid node
		if (side == 0)
			parent->stat |= BEAM_SOLID_FRONT;
		else
			parent->stat |= BEAM_SOLID_BACK;

		// add all polys to the cache
		cpoly_t *next;
		for ( ; sil->polys; sil->polys = next)
		{
			next = sil->polys->next;
			sil->polys->forcez = false;
			g_pShaders->CacheAdd(sil->polys);
		}

		free_sil(sil);
		return;
	}

	beam_node_t *child = get_beam();
	parent->children[side] = child;

	child->stat = 0;
	child->children[0] = child->children[1] = NULL;

	// build the plane
	sil->nedges--;
	vector_t a, b;
	VectorSub(sil->edges[sil->nedges][0], sil->edges[sil->nedges][1], a);
	VectorSub(sil->edges[sil->nedges][0], camera->origin, b);


	
	// copy this sil's sides to the line list
	if (num_lines < 1024)
	{
		VectorCopy(sil->edges[sil->nedges][0], lines[num_lines][0]);
		VectorCopy(sil->edges[sil->nedges][1], lines[num_lines][1]);
		num_lines++;
	}



	_CrossProduct(&a, &b, &child->p.norm);
	VectorNormalize(&child->p.norm);
//	child->p.d = dot(eye.origin, child->p.norm);
	child->p.d = dot(camera->origin, child->p.norm);

	float d = dot(sil->center, child->p.norm) - child->p.d;
	beam_leaf(child, (d<0), sil);
}


/*
==========
beam_trim - remove nodes that are solid on both sides
==========
*/
bool beam_trim(beam_node_t *n)
{
	if (n->children[0])
	{
		if (beam_trim(n->children[0]))
		{
			// frontside is solid - trim it
			free_beam(n->children[0]);
			n->children[0] = NULL;
			n->stat |= BEAM_SOLID_FRONT;
		}
	}

	if (n->children[1])
	{
		if (beam_trim(n->children[1]))
		{
			// backside is solid - trim it
			free_beam(n->children[1]);
			n->children[1] = NULL;
			n->stat |= BEAM_SOLID_BACK;
		}
	}

	return (n->stat == (BEAM_SOLID_FRONT|BEAM_SOLID_BACK));
}


/*
=========
beam_insert
=========
*/
void beam_insert_r(beam_node_t *node, sil_t *sil)
{
	sil_t *front, *back;
	front = back = NULL;

	sil_split(sil, &node->p, &front, &back);

	if (front)
	{
		if (node->stat & BEAM_SOLID_FRONT)
			free_sil(front);
		else
		{
			if (node->children[0])
				beam_insert_r(node->children[0], front);
			else
				beam_leaf(node, 0, front);
		}
	}

	if (back)
	{
		if (node->stat & BEAM_SOLID_BACK)
			free_sil(back);
		else
		{
			if (node->children[1])
				beam_insert_r(node->children[1], back);
			else
				beam_leaf(node, 1, back);
		}
	}
}


void beam_insert(bspf_brush_t *br, int contents)
{
	// non-solid are never passed through beam tree
	if (!(contents & CONTENTS_SOLID))
	{
		for (int s=0; s<br->num_sides; s++)
		{
			cpoly_t *poly = g_pShaders->GetPoly();
			poly->num_vertices = world->sides[s+br->first_side].num_verts;
			poly->texdef		= world->sides[s+br->first_side].texdef;
			poly->lightdef		= world->sides[s+br->first_side].lightdef;
			poly->forcez = true;

			for (int v=0; v<poly->num_vertices; v++)
			{
				VectorCopy(world->verts[world->iverts[world->sides[s+br->first_side].first_vert+v]], poly->vertices[v]);
			}

			g_pShaders->CacheAdd(poly);
		}

		return;
	}


	sil_t *bsil = sil_build(br);
	if (!bsil)
		return;

	beam_insert_r(beam_head, bsil);
	beam_trim(beam_head);
}






