
#include <memory.h>
#include "std_lib.h"
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

/*
==============
plane_test
==============
*/
#define PTEST_NORM_EPSILON	0.00001
#define PTEST_D_EPSILON		0.01
bool plane_test(plane_t &p1, plane_t &p2)
{
	if ((fabs(p1.d - p2.d) < PTEST_D_EPSILON) &&
		(fabs(p1.norm.x - p2.norm.x) < PTEST_NORM_EPSILON) &&
		(fabs(p1.norm.y - p2.norm.y) < PTEST_NORM_EPSILON) &&
		(fabs(p1.norm.z - p2.norm.z) < PTEST_NORM_EPSILON))
		return true;
	return false;
}


/*
=============
get_plane - like q2, # and #^1 are opposites
=============
*/
int get_plane(plane_t plane)
{
	// do we already have a close one?
	for (int p=0; p<num_planes; p++)
	{
		if (plane_test(plane, planes[p]))
			return p;
	}

	// make a new one
	if (num_planes >= MAX_MAP_PLANES-1)
		Error("too many planes!");
	planes[num_planes] = plane;
	num_planes++;

	// make opposing one
	VectorScale(&plane.norm, -1, &planes[num_planes].norm);
	planes[num_planes].d = -plane.d;
	num_planes++;

	return p;
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
		VectorSet(&other[0], 0, 0, 1);
		VectorSet(&other[1], 0, (planes[side->plane].norm.x < 0) ? 1 : -1.0f, 0);
	}
	else if (best_axis == 1)
	{
		VectorSet(&other[0], 0, 0, 1);
		VectorSet(&other[1], (planes[side->plane].norm.y < 0) ? -1.0f : 1, 0, 0);
	}
	else
	{
		VectorSet(&other[0], 0, 1, 0);
		VectorSet(&other[1], (planes[side->plane].norm.z < 0) ? 1 : -1.0f, 0, 0);
	}

	// make sure our right/up vectors are planar
	for (int i=0; i<2; i++)
	{
		float d = dot(planes[side->plane].norm, other[i]);
		VectorMA(&other[i], -d, &planes[side->plane].norm, &other[i]);
		VectorNormalize(&other[i]);
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
#define PLANE_ON	0
#define PLANE_FRONT	1
#define PLANE_BACK	-1
#define CLIP_EPSILON 0.001
void clip_side_p(bsp_brush_side_t *s, plane_t *p)
{
	int			num_cverts = 0;
	vector_t	clipverts[MAX_FACE_VERTS+1];
	int			sides[MAX_FACE_VERTS+1];
	float		dists[MAX_FACE_VERTS+1];

	bool allin = true;
	bool allout= true;

	for (int i=0; i<s->num_verts; i++)
	{
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
		return;
	if (allout)
	{
		s->num_verts = 0;
		return;
	}

	dists[i] = dists[0];
	sides[i] = sides[0];
	
	for (i=0; i<s->num_verts; i++)
	{
		if (sides[i] == PLANE_ON)
		{
			VectorCopy(s->verts[i], clipverts[num_cverts]);
			num_cverts++;
			continue;
		}

		if (sides[i] == PLANE_BACK)
		{
			VectorCopy(s->verts[i], clipverts[num_cverts]);
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
}

void clip_side(bsp_brush_side_t *s, int plane)
{
	clip_side_p(s, &planes[plane]);
}

/*
=============
clip_brush
=============
*/
void clip_brush(bsp_brush_t *b, plane_t *plane)
{
// add this plane to the list if it clips
	bsp_brush_side_t *s;
	if (bsp_test_brush(b, get_plane(*plane)) == 0)
	{
		s = new_bsp_brush_side();
		s->texinfo = 0;
		s->flags = SURF_INVISIBLE;
		s->plane = get_plane(*plane);

		make_base_side(s);
		for (bsp_brush_side_t *s1=b->sides; s1; s1=s1->next)
			if (s->plane != s1->plane)
				clip_side(s, s1->plane);

		s->next = b->sides;
		b->sides = s;
	}

// clip all sides
	for (s=b->sides; s; s=s->next)
		clip_side_p(s, plane);

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
bsp_build_volume
============
*/
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

		VectorSet(&pl.norm, 0, 0, 0);

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
	ent->root->brushes = NULL;
	ent->root->parent = NULL;
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

			ent->root->brushes->sides->plane = get_plane(map_brush_sides[s].plane);
			ent->root->brushes->sides->texinfo = map_brush_sides[s].texinfo;
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
int bsp_test_brush(bsp_brush_t *b, int plane)
{
	bool front = false;
	bool back = false;

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
			if (d > 0.001)
			{
				if (back)
					return 0;
				front = true;
			}
			else if (d < -0.001)
			{
				if (front)
					return 0;
				back = true;
			}
		}
	}

	if (front && back)
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
	int test = bsp_test_brush(b, plane);
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

	clip_brush(*back,  &planes[plane]);
	clip_brush(*front, &planes[plane^1]);
}


/*
============
bsp_select_plane
============
*/
int bsp_select_plane(bsp_node_t *n)
{

	// find splits, fronts, backs
	int splits, front, back;

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
			if (bsp_test_brush(n->volume, s->plane) != 0)
				continue;

			splits = front = back = 0;

			for (b2=n->brushes; b2; b2=b2->next)
			{
				r = bsp_test_brush(b2, s->plane);
				if (r==1)
					front++;
				else if (r==-1)
					back++;
				else
					splits++;
			}


		// how good of a splitter is it?
			v = splits*5 + abs(front-back);

//			if (!splits && (!front || !back))	// all brushes are on the same side - doesn't do any good
//				v = 9999999;

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
	n->children[0]->brushes = n->children[1]->brushes = NULL;
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
