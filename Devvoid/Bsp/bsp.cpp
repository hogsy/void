#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "../Std_lib.h"
#include "bsp.h"
#include "map_file.h"
#include "entity.h"


//=======================================================

int					num_bsp_brushes=0;
bsp_brush_t			bsp_brushes[MAX_MAP_BRUSHES];

int					num_planes=0;
plane_t				planes[MAX_MAP_PLANES];
bool				tested[MAX_MAP_PLANES];	// have we tested with these planes?

int					num_bsp_nodes=0;
bsp_node_t			bsp_nodes[MAX_MAP_NODES];

int					num_bsp_brush_sides=0;
bsp_brush_side_t	bsp_brush_sides[MAX_MAP_BRUSH_SIDES];

bsp_brush_t			sky_brush = { NULL, NULL, CONTENTS_SKY | CONTENTS_SOLID, {0,0,0}, {0,0,0}};


//=======================================================



/*
=============
allocate a new bsp brush side
=============
*/
bool	free_brush_sides[MAX_MAP_BRUSH_SIDES];
bsp_brush_side_t* new_bsp_brush_side(void)
{
	if (num_bsp_brush_sides == MAX_MAP_BRUSH_SIDES)
		Error("too many bsp brush sides!");

	int s=0;
	while (!free_brush_sides[s])
		s++;

	num_bsp_brush_sides++;
	free_brush_sides[s] = false;
	bsp_brush_sides[s].flags = 0;
	bsp_brush_sides[s].visible = true;
	return &bsp_brush_sides[s];
}

void free_bsp_brush_side(bsp_brush_side_t *s)
{
	int i = s - bsp_brush_sides;
	free_brush_sides[i] = true;
	num_bsp_brush_sides--;
}

/*
=============
allocate a new bsp brush
=============
*/
bool	free_brushes[MAX_MAP_BRUSHES];
bsp_brush_t* new_bsp_brush(void)
{
	if (num_bsp_brushes == MAX_MAP_BRUSHES)
		Error("too many bsp brushes!");

	// find the first free brush
	int b=0;
	while (!free_brushes[b])
		b++;

	num_bsp_brushes++;
	free_brushes[b] = false;
	return &bsp_brushes[b];
}

void free_bsp_brush(bsp_brush_t *b)
{
	// free all sides too
	for (bsp_brush_side_t *s=b->sides; s; )
	{
		bsp_brush_side_t *s1 = s;
		s = s->next;

		free_bsp_brush_side(s1);
	}

	int i = (b - bsp_brushes);
	free_brushes[i] = true;
	num_bsp_brushes--;
}

void reset_bsp_brush(void)
{
	sky_brush.contents = CONTENTS_SKY | CONTENTS_SOLID;
	sky_brush.mins[0] = 0;
	sky_brush.mins[1] = 0;
	sky_brush.mins[2] = 0;
	sky_brush.maxs[0] = 0;
	sky_brush.maxs[1] = 0;
	sky_brush.maxs[2] = 0;
	sky_brush.sides = NULL;
	sky_brush.next = NULL;

	// reset map structs
	for (int i=0; i<MAX_MAP_BRUSHES; i++)
		free_brushes[i] = true;
	for (i=0; i<MAX_MAP_BRUSH_SIDES; i++)
		free_brush_sides[i] = true;
}

/*
=============
allocate a new bsp node
=============
*/
bsp_node_t* new_bsp_node(void)
{
	if (num_bsp_nodes == MAX_MAP_NODES)
		Error("too many bsp brushes!");

	num_bsp_nodes++;
	return &bsp_nodes[num_bsp_nodes-1];
}


//=======================================================


void calc_brush_bounds(bsp_brush_t *b)
{
	b->mins[0] = b->mins[1] = b->mins[2] =  99999;
	b->maxs[0] = b->maxs[1] = b->maxs[2] = -99999;

	for (bsp_brush_side_t *s = b->sides; s; s=s->next)
	{
		for (int v=0; v<s->num_verts; v++)
		{
			if (s->verts[v].x > b->maxs[0])
				b->maxs[0] = (int)ceil(s->verts[v].x);

			if (s->verts[v].y > b->maxs[1])
				b->maxs[1] = (int)ceil(s->verts[v].y);

			if (s->verts[v].z > b->maxs[2])
				b->maxs[2] = (int)ceil(s->verts[v].z);


			if (s->verts[v].x < b->mins[0])
				b->mins[0] = (int)floor(s->verts[v].x);

			if (s->verts[v].y < b->mins[1])
				b->mins[1] = (int)floor(s->verts[v].y);

			if (s->verts[v].z < b->mins[2])
				b->mins[2] = (int)floor(s->verts[v].z);
		}
	}
}


/*
=============
make_base_side
=============
*/
void make_base_side(bsp_brush_side_t *side)
{
	// find the closest axial normal
	int best_axis = 0;
	float best_val = (float)fabs(planes[side->plane].norm.x);
	if ((float)fabs(planes[side->plane].norm.y) > best_val)
	{
		best_axis =  1;
		best_val = (float)fabs(planes[side->plane].norm.y);
	}
	if ((float)fabs(planes[side->plane].norm.z) > best_val)
		best_axis =  2;


	vector_t other[2];
	if (best_axis == 0)
	{
		other[0].Set(0, 0, 1);
		other[1].Set(0, (planes[side->plane].norm.x < 0) ? 1 : -1.0f, 0);
	}
	else if (best_axis == 1)
	{
		other[0].Set(0, 0, 1);
		other[1].Set((planes[side->plane].norm.y < 0) ? -1.0f : 1, 0, 0);
	}
	else
	{
		other[0].Set(0, 1, 0);
		other[1].Set((planes[side->plane].norm.z < 0) ? 1 : -1.0f, 0, 0);
	}

	// make sure our right/up vectors are planar
	for (int i=0; i<2; i++)
	{
		float d = dot(planes[side->plane].norm, other[i]);
		other[i].VectorMA(other[i], -d, planes[side->plane].norm);
		other[i].Normalize();
	}


	// make the sides
	plane_t *p = &planes[side->plane];
	side->num_verts = 4;

	side->verts[0].x =  10000*other[0].x + 10000*other[1].x + p->d*p->norm.x;
	side->verts[0].y =  10000*other[0].y + 10000*other[1].y + p->d*p->norm.y;
	side->verts[0].z =  10000*other[0].z + 10000*other[1].z + p->d*p->norm.z;

	side->verts[1].x =  10000*other[0].x - 10000*other[1].x + p->d*p->norm.x;
	side->verts[1].y =  10000*other[0].y - 10000*other[1].y + p->d*p->norm.y;
	side->verts[1].z =  10000*other[0].z - 10000*other[1].z + p->d*p->norm.z;

	side->verts[2].x = -10000*other[0].x - 10000*other[1].x + p->d*p->norm.x;
	side->verts[2].y = -10000*other[0].y - 10000*other[1].y + p->d*p->norm.y;
	side->verts[2].z = -10000*other[0].z - 10000*other[1].z + p->d*p->norm.z;

	side->verts[3].x = -10000*other[0].x + 10000*other[1].x + p->d*p->norm.x;
	side->verts[3].y = -10000*other[0].y + 10000*other[1].y + p->d*p->norm.y;
	side->verts[3].z = -10000*other[0].z + 10000*other[1].z + p->d*p->norm.z;

}


/*
=============
clip_side
=============
*/
float side_area(bsp_brush_side_t *si);
#define PLANE_ON	0
#define PLANE_FRONT	1
#define PLANE_BACK	-1
#define CLIP_EPSILON 0.001
int clip_side_p(bsp_brush_side_t *s, plane_t *p)
{
	int			num_cverts = 0;
	vector_t	clipverts[MAX_FACE_VERTS+1];
	int			sides[MAX_FACE_VERTS+1];
	float		dists[MAX_FACE_VERTS+1];

	bool allin = true;
	bool allout= true;

	for (int i=0; i<s->num_verts; i++)
	{
		if (p->norm.x == 1)
			dists[i] =  s->verts[i].x - p->d;
		else if (p->norm.x == -1)
			dists[i] = -s->verts[i].x - p->d;
		else if (p->norm.y == 1)
			dists[i] =  s->verts[i].y - p->d;
		else if (p->norm.y == -1)
			dists[i] =  -s->verts[i].y - p->d;
		else if (p->norm.z == 1)
			dists[i] =  s->verts[i].z - p->d;
		else if (p->norm.z == -1)
			dists[i] =  -s->verts[i].z - p->d;

		else
			dists[i] = dot(p->norm, s->verts[i]) - p->d;
		
		if (dists[i] > CLIP_EPSILON)
		{
			sides[i] = PLANE_FRONT;
			allin = false;
		}
		else if (dists[i] < -CLIP_EPSILON)
		{
			sides[i] = PLANE_BACK;
			allout = false;
		}
		else
			sides[i] = PLANE_ON;
	}

	if (allin)
		return PLANE_BACK;
	if (allout)
	{
		s->num_verts = 0;
		return PLANE_FRONT;
	}

	dists[i] = dists[0];
	sides[i] = sides[0];

	for (i=0; i<s->num_verts; i++)
	{
		if (sides[i] == PLANE_ON)
		{
			clipverts[num_cverts] = s->verts[i];
			num_cverts++;
			continue;
		}

		if (sides[i] == PLANE_BACK)
		{
			clipverts[num_cverts] = s->verts[i];
			num_cverts++;
		}

		if ((sides[i+1] == PLANE_ON) || (sides[i] == sides[i+1]))
			continue;

		vector_t *nextvert = &s->verts[(i+1)%s->num_verts];
		double frac = dists[i] / (dists[i]-dists[i+1]);
		clipverts[num_cverts].x = (float)(s->verts[i].x + frac*(nextvert->x - s->verts[i].x));
		clipverts[num_cverts].y = (float)(s->verts[i].y + frac*(nextvert->y - s->verts[i].y));
		clipverts[num_cverts].z = (float)(s->verts[i].z + frac*(nextvert->z - s->verts[i].z));
		num_cverts++;
	}

	// copy everything back
	if (num_cverts > MAX_FACE_VERTS)
		Error("clipped face to too many verts");
	memcpy(s->verts, clipverts, sizeof(vector_t) * num_cverts);
	s->num_verts = num_cverts;

	if (side_area(s) < 1.0f)
		s->num_verts = 0;

	return PLANE_ON;
}

int clip_side(bsp_brush_side_t *s, int plane)
{
	return clip_side_p(s, &planes[plane]);
}

/*
=============
clip_brush
=============
*/
void clip_brush(bsp_brush_t *b, int plane)
{
// add this plane to the list if it clips
	bsp_brush_side_t *s;
	int side = bsp_test_brush(b, plane, NULL);

	// front gets clipped away
	if (side == 1)
	{
		bsp_brush_side_t *next;
		for ( ; b->sides; b->sides = next)
		{
			next = b->sides->next;
			free_bsp_brush_side(b->sides);
		}
		return;
	}

	// completely on backside - no effect
	else if (side == -0.01f)
		return;

	// else we have to add it and do clipping

	s = new_bsp_brush_side();
	s->texinfo = 0;
	s->flags = SURF_INVISIBLE;
	s->plane = plane;

	// clip the new side to all the other sides
	make_base_side(s);
	for (bsp_brush_side_t *s1=b->sides; s1; s1=s1->next)
		if (s->plane != s1->plane)
			clip_side(s, s1->plane);

	s->next = b->sides;
	b->sides = s;


	// clip all other sides to the new one
	for (s=b->sides; s; s=s->next)
		clip_side(s, plane);

// remove any faces that where clipped out
	bsp_brush_side_t *prev = NULL, *remove;
	for (s=b->sides; s; )
	{
		if (s->num_verts == 0)
		{
			if (prev)
				prev->next = s->next;
			else
				b->sides = s->next;

			remove = s;
			s = s->next;
			free_bsp_brush_side(remove);
		}

		else
		{
			prev = s;
			s = s->next;
		}
	}

	calc_brush_bounds(b);
}


/*
============
bsp_add_sky_brush
============
*/
bsp_brush_side_t* bsp_sky_side_clip(bsp_brush_side_t *clipper, bsp_brush_side_t *side)
{
	plane_t clipplanes[33];
	side->next = NULL;


	// create all the planes we need
	for (int v=0; v<clipper->num_verts; v++)
	{
		int nv = (v+1) % clipper->num_verts;

		vector_t a = clipper->verts[v] - clipper->verts[nv];
		CrossProduct(clipper->verts[nv], a, clipplanes[v].norm);
		clipplanes[v].norm.Normalize();
		clipplanes[v].d = 0;
	}

	// the plane of the clip side
	clipplanes[v] = planes[clipper->plane];


	// first make sure side isn't entirely on front side of all planes
	// this is a lot slower but will prevent breaking up a lot of polys
	bsp_brush_side_t *test = new_bsp_brush_side();
	memcpy(test, side, sizeof(bsp_brush_side_t));

	for (v=0; v<=clipper->num_verts; v++)
	{
		clip_side_p(test, &clipplanes[v]);

		if (test->num_verts == 0)
		{
			free_bsp_brush_side(test);
			return side;
		}
	}
	free_bsp_brush_side(test);


	bsp_brush_side_t *newsides = NULL;
	for (v=0; v<=clipper->num_verts; v++)
	{
		bsp_brush_side_t *front = new_bsp_brush_side();
		memcpy(front, side, sizeof(bsp_brush_side_t));

		plane_t p = clipplanes[v];

		clip_side_p(side, &p);

		p.norm.Inverse();
		p.d = -p.d;

		clip_side_p(front, &p);

		if (front->num_verts != 0)
		{
			// it's a keeper
			front->next = newsides;
			newsides = front;
		}
		else
			free_bsp_brush_side(front);


		if (side->num_verts == 0)
//			break;
			Error("clipped sky poly bad");

	}

	free_bsp_brush_side(side);
	return newsides;
}


void bsp_add_sky_side(bsp_brush_side_t *side)
{
	// clip away any part of this side that is blocked and
	// clip away any other sides that are blocked by this

	bsp_brush_side_t *newsky = NULL;
	bsp_brush_side_t *next, *walk;

	// clip all existing sides to the new one
	for (bsp_brush_side_t *s=sky_brush.sides; s; s=next)
	{
		next = s->next;

		bsp_brush_side_t *sideclip = bsp_sky_side_clip(side, s);

		if (sideclip)
		{
			walk = sideclip;
			while (walk->next)
				walk = walk->next;
			walk->next = newsky;
			newsky = sideclip;
		}
	}


	// clip all new ones to existing sides
	bsp_brush_side_t *clipsides = side;
	clipsides->next = NULL;

	for (s=newsky; s; s=s->next)
	{
		bsp_brush_side_t *newclipsides = clipsides;
		clipsides = NULL;
	
		for (bsp_brush_side_t *s2=newclipsides; s2; s2=next)
		{
			next = s2->next;
			bsp_brush_side_t *sideclip = bsp_sky_side_clip(s, s2);

			if (sideclip)
			{
				walk = sideclip;
				while(walk->next)
					walk = walk->next;

				walk->next = clipsides;
				clipsides = sideclip;
			}
		}
	}


	walk = clipsides;
	if (clipsides)
	{
		while(walk->next)
			walk = walk->next;

		walk->next = newsky;
		newsky = clipsides;
	}

	sky_brush.sides = newsky;
}


void bsp_add_sky_brush(bsp_brush_t *b)
{
	bsp_brush_side_t *next;
	bsp_brush_side_t *s = b->sides;
	b->sides = NULL;
	for (; s; s=next)
	{
		next = s->next;

		// only add sides facing the origin
		if (planes[s->plane].d < 0)
		{
			bsp_add_sky_side(s);
		}
		else
			free_bsp_brush_side(s);
	}
	
	free_bsp_brush(b);
	calc_brush_bounds(&sky_brush);
}


/*
============
bsp_build_volume
============
*/
int get_plane(plane_t plane);
bsp_brush_t* bsp_build_volume(void)
{
	bsp_brush_t *vol = new_bsp_brush();
	vol->next = NULL;

	// first add the 6 planes that limit the world
	vol->sides = NULL;
	vol->next = NULL;

	plane_t pl;
	pl.d = 10000;

	bsp_brush_side_t *nside;
	for (int i=0; i<6; i++)
	{
		nside = new_bsp_brush_side();

		pl.norm.Set(0, 0, 0);

		switch (i)
		{
		case 0:
			pl.norm.x = 1;
			break;
		case 1:
			pl.norm.x = -1;
			break;
		case 2:
			pl.norm.y = 1;
			break;
		case 3:
			pl.norm.y = -1;
			break;
		case 4:
			pl.norm.z = 1;
			break;
		case 5:
			pl.norm.z = -1;
		}

		nside->plane = get_plane(pl);
		nside->next = vol->sides;
		vol->sides = nside;
	}

	// create all the verts
	for (bsp_brush_side_t *tside1 = vol->sides; tside1; tside1 = tside1->next)
	{
		make_base_side(tside1);

		// clip each face to every other face
		for (bsp_brush_side_t *tside2 = vol->sides; tside2; tside2 = tside2->next)
		{
			if (tside1 == tside2)
				continue;

			clip_side(tside1, tside2->plane);
		}
	}

	calc_brush_bounds(vol);
	return vol;
}


/*
=============
build_bsp_brushes
=============
*/
void build_bsp_brushes(entity_t *ent)
{
	ent->root = new_bsp_node();
	ent->root->portals = NULL;
	ent->root->brushes = NULL;
	ent->root->parent = NULL;
	ent->root->outside = false;
	ent->root->volume = bsp_build_volume();

// only copy over the most basic info
	int eend = map_entities[ent->map_ent].first_brush + map_entities[ent->map_ent].num_brushes;
	for (int b=map_entities[ent->map_ent].first_brush; b<eend; b++)
	{
		bsp_brush_t *tb = ent->root->brushes;
		ent->root->brushes = new_bsp_brush();
		ent->root->brushes->next = tb;
		ent->root->brushes->sides = NULL;
		ent->root->brushes->contents = map_brushes[b].contents;

		int send = map_brushes[b].first_side + map_brushes[b].num_sides;
		for (int s=map_brushes[b].first_side; s<send; s++)
		{
			bsp_brush_side_t *ts = ent->root->brushes->sides;
			ent->root->brushes->sides = new_bsp_brush_side();
			ent->root->brushes->sides->next = ts;

			ent->root->brushes->sides->plane = map_brush_sides[s].plane;
			ent->root->brushes->sides->texinfo = map_brush_sides[s].texinfo;
			ent->root->brushes->sides->flags = map_brush_sides[s].flags;
			ent->root->brushes->sides->num_verts = 0;
		}
	}

// build vertice lists
	for (bsp_brush_t *tbrush = ent->root->brushes; tbrush; tbrush = tbrush->next)
	{
		for (bsp_brush_side_t *tside1 = tbrush->sides; tside1; tside1 = tside1->next)
		{
			make_base_side(tside1);

			// clip each face to every other face
			for (bsp_brush_side_t *tside2 = tbrush->sides; tside2; tside2 = tside2->next)
			{
				if (tside1 == tside2)
					continue;

				clip_side(tside1, tside2->plane);
			}
		}

		calc_brush_bounds(tbrush);
	}

	// separate out all of the sky brushes
	bsp_brush_t *prev = NULL;
	bsp_brush_t *next;
	for (tbrush = ent->root->brushes; tbrush; tbrush = next)
	{
		next = tbrush->next;

		if (tbrush->contents & CONTENTS_SKY)
		{
			// take it out of the list
			if (prev)
				prev->next = next;
			else
				ent->root->brushes = next;

			bsp_add_sky_brush(tbrush);
		}
		else
			prev = tbrush;

	}
}


//=======================================================

/*
============
copy_brush
============
*/
bsp_brush_t* copy_brush(bsp_brush_t *in)
{
	bsp_brush_side_t *ts = NULL;
	bsp_brush_t *out = new_bsp_brush();

	out->sides = NULL;
	out->next = NULL;
	for (bsp_brush_side_t *s=in->sides; s; s=s->next)
	{
		ts = out->sides;
		out->sides = new_bsp_brush_side();
		memcpy(out->sides, s, sizeof(bsp_brush_side_t));
		out->sides->next = ts;
	}

	out->contents = in->contents;

	memcpy(out->mins, in->mins, sizeof(int)*6);
	return out;
}

void copy_brush2(bsp_brush_t *in,  bsp_brush_t *out)
{
	bsp_brush_side_t *s, *ts = NULL;

	for (s=out->sides; s; )
	{
		ts = s;
		s=s->next;
		free_bsp_brush_side(ts);
	}

	out->sides = NULL;
	for (s=in->sides; s; s=s->next)
	{
		ts = out->sides;
		out->sides = new_bsp_brush_side();
		memcpy(out->sides, s, sizeof(bsp_brush_side_t));
		out->sides->next = ts;
	}

	out->contents = in->contents;

	memcpy(out->mins, in->mins, sizeof(int)*6);
}


/*
============
node_contents
============
*/
int node_contents(bsp_node_t *n)
{
	if (n->plane == -1)
		return n->contents;

	int c=0;
	c  = node_contents(n->children[0]);
	c |= node_contents(n->children[1]);
	return c;
}

/*
============
bsp_test_brush
============
*/
int bsp_test_brush(bsp_brush_t *b, int plane, bool *epsilon)
{
	bool front = false;
	bool back = false;
	float dfront = -99999;
	float dback  =  99999;

	float d;

	for (bsp_brush_side_t *s=b->sides; s; s=s->next)
	{
		for (int v=0; v<s->num_verts; v++)
		{
			// backside
			if (s->plane == plane)
				return -1;

			// frontside
			if (s->plane == (plane^1))
				return 1;


			d = dot(planes[plane].norm, s->verts[v]) - planes[plane].d;

			if (d > dfront)
				dfront = d;
			if (d < dback)
				dback = d;

			if (d > 0.01)
			{
//				if (back)
//					return 0;
				front = true;
			}
			else if (d < -0.01)
			{
//				if (front)
//					return 0;
				back = true;
			}
		}
	}

	if (epsilon)
	{
		if (((dfront > 0) && (dfront < 2)) ||
			((dback  < 0) && (dback  >-2)))
			*epsilon = true;
		else
			*epsilon = false;
	}


	if (front && back)
		return 0;

	if (!front && !back)
		return 0;

	if (back)
		return -1;
	return 1;
}


/*
===========
bsp_brush_split
===========
*/
void bsp_brush_split(bsp_brush_t *b, bsp_brush_t **front, bsp_brush_t **back, int plane)
{
	// see if we actually split the brush
	int test = bsp_test_brush(b, plane, NULL);
	if (test==1)
	{
		*front = b;
		*back = NULL;
		return;
	}
	if (test==-1)
	{
		*front = NULL;
		*back = b;
		return;
	}


//	plane_t p = planes[plane];

	*front = copy_brush(b);
	*back  = copy_brush(b);
	(*front)->next = (*back)->next = NULL;

	clip_brush(*back,  plane);
	clip_brush(*front, plane^1);
}


/*
============
bsp_test_to_parents
============
*/
bool bsp_test_to_parents(bsp_node_t *n, int plane)
{
	if (!n)
		return false;

	if ((n->plane|1) == (plane|1))
		return true;
	return bsp_test_to_parents(n->parent, plane);
}


/*
============
bsp_select_plane
============
*/
int bsp_select_plane(bsp_node_t *n)
{

	// find splits, fronts, backs
	int splits, front, back, epsilon;
	bool bepsilon;

	int v, best_val = 9999999;
	int best_plane  = -1;
	int r;

	bsp_brush_side_t	*s;
	bsp_brush_t			*b1, *b2;

	// set not tested
	for (int p=0; p<MAX_MAP_PLANES; p++)
		tested[p] = false;


	for (b1=n->brushes; b1; b1=b1->next)
	{
		for (s=b1->sides; s; s=s->next)
		{
			// have we already tested this plane from a different side?
			if (tested[s->plane])
				continue;

			// make sure it splits the volume
			if (bsp_test_brush(n->volume, s->plane, NULL) != 0)
				continue;

			if (bsp_test_to_parents(n, best_plane))
				continue;

			splits = front = back = epsilon = 0;

			for (b2=n->brushes; b2; b2=b2->next)
			{
				r = bsp_test_brush(b2, s->plane, &bepsilon);
				if (bepsilon)
					epsilon++;
				if (r==1)
					front++;
				else if (r==-1)
					back++;
				else
					splits++;
			}


		// how good of a splitter is it?
			v = splits*5 + abs(front-back);

			// epsilon brushes = bad
			v += epsilon * 1000;

			// axial plane == good
			if (!(( (planes[s->plane].norm.x == -1) || 
					(planes[s->plane].norm.x ==  1) ||
					(planes[s->plane].norm.y == -1) ||
					(planes[s->plane].norm.y ==  1) ||
					(planes[s->plane].norm.z == -1) ||
					(planes[s->plane].norm.z ==  1))))
					v *= 2;


			if (v < best_val)
			{
				best_val = v;
				best_plane = s->plane;
			}

			tested[s->plane] = true;
		}
	}

	return best_plane;
}


/*
============
bsp_partition
============
*/
void brush_split(bsp_brush_t *b, bsp_brush_t **front, bsp_brush_t **back, int plane);

void bsp_partition(bsp_node_t *n)
{
	n->children[0] = n->children[1] = NULL;
	n->portals = NULL;
	n->outside = false;

	int best_plane = bsp_select_plane(n);

	// see if it's a leaf
	if (best_plane == -1)
	{
		n->plane = -1;
		n->contents = 0;
		if (n->brushes)
			n->contents |= n->brushes->contents;
		return;
	}

	// split all the brushes with the best plane
	n->plane = best_plane;
	n->children[0] = new_bsp_node();
	n->children[1] = new_bsp_node();
	n->children[0]->outside = n->children[1]->outside = false;
	n->children[0]->brushes = n->children[1]->brushes = NULL;
	n->children[0]->portals = n->children[1]->portals = NULL;
	n->children[0]->parent  = n->children[1]->parent  = n;

	// split current volume into it's 2 child volumes
	bsp_brush_split(n->volume, &n->children[0]->volume, &n->children[1]->volume, n->plane);

	// split all brushes into 2 child lists
	bsp_brush_t *b1, *b2;
	for (b1=n->brushes; b1; )
	{
		b2 = b1;
		b1 = b1->next;
		b2->next = NULL;

		bsp_brush_t *front, *back;
		bsp_brush_split(b2, &front, &back, n->plane);

		if (front)
		{
			front->next = n->children[0]->brushes;
			n->children[0]->brushes = front;
		}

		if (back)
		{
			back->next = n->children[1]->brushes;
			n->children[1]->brushes = back;
		}

		// a new brushes were allocated - have to free old one
		if (front && back)
			free_bsp_brush(b2);
	}

	n->brushes = NULL;
	bsp_partition(n->children[0]);
	bsp_partition(n->children[1]);
	n->contents = node_contents(n);
}
