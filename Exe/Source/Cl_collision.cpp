#include "Cl_main.h"
#include "Cl_collision.h"

extern world_t	 *g_pWorld;


// FIXME - should collide with all objects in the sector first, then objects
// against the player, then against the walls and ???walls against player???
// since it's just a single ray, and no objects, just collide against walls right now

#define ON_EPSILON 0.03125f


void ray_leaf(int leaf, rayhit_t *rhit, vector_t *start, vector_t *end)
{
	float dstart, dend;
	float frac, nfrac;
	frac = 1;

	// dir used for finding intersect points
	vector_t dir;
	VectorSub((*end), (*start), dir);

	// check against all brushes
	int endb = g_pWorld->leafs[leaf].first_brush + g_pWorld->leafs[leaf].num_brushes;
	for (int b=g_pWorld->leafs[leaf].first_brush; b<endb; b++)
	{

		int ends = g_pWorld->brushes[b].first_side + g_pWorld->brushes[b].num_sides;
		for (int s=g_pWorld->brushes[b].first_side; s<ends; s++)
		{
			dstart = dot ((*start), g_pWorld->planes[g_pWorld->sides[s].plane].norm) - g_pWorld->planes[g_pWorld->sides[s].plane].d;
			dend   = dot ((*end)  , g_pWorld->planes[g_pWorld->sides[s].plane].norm) - g_pWorld->planes[g_pWorld->sides[s].plane].d;


			if ((dend < 0) && (dend<dstart) && (dstart>=-2))
			{
				nfrac = dstart / (dstart-dend);

				// no use doing all this if it will be farther if it does hit it
				if (nfrac >= frac)
					continue;


				// find the intersection point
				vector_t inter;
				VectorMA(start, nfrac, &dir, &inter);

				// we hit the plane, make sure we actually hit the face
				int endv = g_pWorld->sides[s].first_vert + g_pWorld->sides[s].num_verts;
				for (int v=g_pWorld->sides[s].first_vert; v<endv; v++)
				{
					int nv = (v-g_pWorld->sides[s].first_vert+1)%g_pWorld->sides[s].num_verts + g_pWorld->sides[s].first_vert;

					vector_t a, b, norm;
					VectorSub(g_pWorld->verts[g_pWorld->iverts[ v]], inter, a);
					VectorSub(g_pWorld->verts[g_pWorld->iverts[nv]], inter, b);

					_CrossProduct(&b, &a, &norm);

					// facing the same way - hit's outside the face
					if (dot(norm, dir) > 0)
						break;
				}

				// didn't terminate abnormally - hit the face
				if (v == endv)
				{
					rhit->side = s;
					if (nfrac > 0.99f) nfrac = 1;
					if (nfrac < 0.1f) nfrac = 0;

					frac = nfrac;
				}
			}
		}
	}


	// copy our where we ended up into the intersection
	VectorMA(start, frac, &dir, &rhit->end);
//	rhit->dist = frac*len;
}

/**************************************************************
collide a ray with everything in the world
**************************************************************/
rayhit_t ray(int node, vector_t *start, vector_t *end)
{
	rayhit_t rhit;
	rhit.side = -1;
	VectorCopy((*start), rhit.end);
//	VectorMA(start, len, dir, &rhit.end);

	if (node == -1)
		return rhit;

	float dstart, dend;

//	VectorMA(start, len, dir, &rhit.end);

/*	// if we're in a leaf
	if (g_pWorld->nodes[node].num_brushes)
	{
		ray_leaf(node, &rhit, start, end);
		return rhit;
	}
*/
	dstart = dot((*start), g_pWorld->planes[g_pWorld->nodes[node].plane].norm) - g_pWorld->planes[g_pWorld->nodes[node].plane].d;
	dend   = dot((*end)  , g_pWorld->planes[g_pWorld->nodes[node].plane].norm) - g_pWorld->planes[g_pWorld->nodes[node].plane].d;

	float frac = dstart/(dstart-dend);
	int n;

	if (dstart >= 0)
		n = 0;
	else
		n = 1;

	int nnode;

	// we're going to hit a plane that separates 2 child nodes
	if (0<frac && frac<1)
	{
		// we hit something in the near child node
		nnode = g_pWorld->nodes[node].children[n];
		if (nnode>0)
			rhit = ray(nnode, start, end);
		else
			ray_leaf(-nnode, &rhit, start, end);

		if (VectorCompare2(&rhit.end, end, 0.5f))
			return rhit;

		// didn't hit anything in near node, collide with far node
		vector_t nstart;
		VectorCopy(rhit.end, nstart);

		nnode = g_pWorld->nodes[node].children[1-n];
		if (nnode>0)
			return ray(nnode, &nstart, end);
		else
		{
			ray_leaf(-nnode, &rhit, &nstart, end);
			return rhit;
		}
	}

	// else trace lies entirely within near node - collide with it
	nnode = g_pWorld->nodes[node].children[n];
	if (nnode>0)
		return ray(nnode, start, end);
	ray_leaf(-nnode, &rhit, start, end);
	return rhit;
}



/**************************************************************
collide a bounding box with everything in the world
**************************************************************/
trace_t trace(vector_t *start, vector_t *end, vector_t *mins, vector_t *maxs)
{
	trace_t trace;
	trace.fraction = 1;
	trace.plane = NULL;
	VectorCopy((*start), trace.endpos);

	if (VectorCompare2(start, end, 0.01f))
		return trace;

	// find the length/direction of a full trace
	vector_t dir;
	VectorSub((*end), (*start), dir);
	float want_length = VectorLength(&dir);

	float	shortest = want_length;
	int		side = -1;
	rayhit_t rhit;
	vector_t bend;	// where this box corner should end

	if (mins && maxs)
	{
		vector_t bbox[8];
		bbox[0].x = mins->x;	bbox[0].y = mins->y;	bbox[0].z = mins->z;
		bbox[1].x = mins->x;	bbox[1].y = mins->y;	bbox[1].z = maxs->z;
		bbox[2].x = mins->x;	bbox[2].y = maxs->y;	bbox[2].z = mins->z;
		bbox[3].x = mins->x;	bbox[3].y = maxs->y;	bbox[3].z = maxs->z;
		bbox[4].x = maxs->x;	bbox[4].y = mins->y;	bbox[4].z = mins->z;
		bbox[5].x = maxs->x;	bbox[5].y = mins->y;	bbox[5].z = maxs->z;
		bbox[6].x = maxs->x;	bbox[6].y = maxs->y;	bbox[6].z = mins->z;
		bbox[7].x = maxs->x;	bbox[7].y = maxs->y;	bbox[7].z = maxs->z;

		// test each corner of the box
		// FIXME - leave out one of the corners based on direction??
		for (int i=0; i<8; i++)
		{
			VectorAdd(bbox[i], (*start), bbox[i]);
			VectorAdd(bbox[i], dir, bend);

			rhit = ray(0, &bbox[i], &bend);

			// find the length this corner went
			VectorSub(rhit.end, bbox[i], bend);
			float len = VectorLength(&bend);

			if ((shortest > len) && (rhit.side != -1))
			{
				shortest = len;
				side = rhit.side;

				if (shortest==0)
					break;
			}
		}
	}

	// just trace a line
	else
	{
		rhit = ray(0, start, end);
		VectorSub(rhit.end, (*start), bend);
		shortest = VectorLength(&bend);
		side = rhit.side;
	}


// fill out our trace struct
	trace.fraction = shortest / want_length;
	if (trace.fraction < 0) trace.fraction = 0;
	if (trace.fraction > 1) trace.fraction = 1;

	if (side != -1)
		trace.plane = &g_pWorld->planes[g_pWorld->sides[side].plane];

	VectorMA(start, trace.fraction, &dir, &trace.endpos);
	return trace;


/*
	int i;
	trace.fraction = 1;
	trace.face = NULL;
	trace.endpos.sector = start->sector;


	location_t bbox[8];	// the box that will be traced

//
// set up the box
//


//
// find the smallest dist we could go
//
	rayhit_t rhit;

	vector_t direction;	// dir of the move
	VectorSub((*end), start->point, direction);

// FIXME - wont have to trace all corners!
	for (i = 0; i < 8; i++)
	{
		bbox[i].sector = start->sector;
		bbox[i].sector = (ray(*start, &bbox[i].point)).sector;

		VectorAdd(bbox[i].point, start->point, bbox[i].point);
		rhit = ray(bbox[i], &direction);
		if (rhit.fraction < trace.fraction)
		{
			trace.fraction	= rhit.fraction;
			trace.face		= rhit.face;

			if (rhit.fraction == 0)
				break;
		}
	}

// trace as far as we (the shortest length we went)
// have to do all traces over cause they will all be different (except the shortest one)
	vector_t newdir;
	VectorScale(&direction, trace.fraction, &newdir);

// do the origin
	rhit = ray(*start, &newdir);
	trace.endpos.sector = rhit.sector;
	VectorAdd(start->point, newdir, trace.endpos.point);
*/
}