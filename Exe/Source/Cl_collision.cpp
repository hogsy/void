#include "Cl_main.h"
#include "Cl_collision.h"

extern world_t	 *g_pWorld;


// FIXME - should collide with all objects in the sector first, then objects
// against the player, then against the walls and ???walls against player???
// since it's just a single ray, and no objects, just collide against walls right now


#define ON_EPSILON 0.03125f


/**************************************************************
collide a ray with everything in the world
**************************************************************/
plane_t* ray(int node, vector_t *start, vector_t *end, vector_t *endpos)
{
	plane_t *hitplane;
	float dstart, dend;
	plane_t *plane;

	if (dot((*start), g_pWorld->planes[g_pWorld->nodes[node].plane].norm) - g_pWorld->planes[g_pWorld->nodes[node].plane].d >= 0)
		plane = &g_pWorld->planes[g_pWorld->nodes[node].plane];
	else
		plane = &g_pWorld->planes[g_pWorld->nodes[node].plane^1];

	dstart = dot((*start), plane->norm) - plane->d;
	dend   = dot((*end)  , plane->norm) - plane->d;

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
		vector_t inter;
		inter.x = start->x + frac*(end->x - start->x);
		inter.y = start->y + frac*(end->y - start->y);
		inter.z = start->z + frac*(end->z - start->z);

	// collide through near node
		nnode = g_pWorld->nodes[node].children[n];
		// node
		if (nnode>0)
		{
			hitplane = ray(nnode, start, &inter, endpos);
			// if we're not at the intersection, we hit something - return
			if (hitplane)
				return hitplane;
		}

		// leaf
		else
		{
			// stop at beginning
			if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
			{
				VectorCopy((*start), (*endpos));
					return plane;

			}

			// else we can move all the way through it - FIXME add contents??
		}


		// we made it through near side - collide with far side
		nnode = g_pWorld->nodes[node].children[1-n];

		// node
		if (nnode>0)
		{
			hitplane = ray(nnode, &inter, end, endpos);
			if (hitplane)
				return hitplane;
		}

		// leaf
		else
		{
			if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
			{
				VectorCopy(inter, (*endpos));
				if (n == 0)
					return plane;
			}
		}

		// if we're here, we didn't hit anything
		VectorCopy((*end), (*endpos));
		return NULL;
	}

	// else we're entirely in the near node
	nnode = g_pWorld->nodes[node].children[n];

	// node
	if (nnode>0)
	{
		hitplane = ray(nnode, start, end, endpos);
		return hitplane;
	}

	// leaf
//	if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
//	{
//		VectorCopy((*start), (*endpos));
//	}
//	else
	{
		VectorCopy((*end), (*endpos));
	}

	return NULL;
}












/*



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
**************************************************************
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
*/





/**************************************************************
collide a bounding box with everything in the world
**************************************************************/
trace_t trace(vector_t *start, vector_t *end, vector_t *mins, vector_t *maxs)
{
	trace_t trace;
	trace.fraction = 1;
	trace.plane = NULL;
	VectorCopy((*end), trace.endpos);

	if (VectorCompare2(start, end, 0.01f))
		return trace;

	// find the length/direction of a full trace
	vector_t dir, endpos;
	VectorSub((*end), (*start), dir);
	float want_length = VectorLength(&dir);

	float	shortest = want_length;
	plane_t*	hitplane;
//	rayhit_t rhit;
	vector_t bend;	// where this box corner should end

//	if (mins && maxs)
	if (0)
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

			hitplane = ray(0, &bbox[i], &bend, &endpos);

			// find the length this corner went
			VectorSub(endpos, bbox[i], bend);
			float len = VectorLength(&bend);

			if ((shortest > len) && hitplane)
			{
				shortest = len;
				trace.plane = hitplane;

				if (shortest==0)
					break;
			}
		}
	}

	// just trace a line
	else
	{
		trace.plane = ray(0, start, end, &endpos);
		VectorSub(endpos, (*start), bend);
		shortest = VectorLength(&bend);
	}


// fill out our trace struct
	trace.fraction = shortest / want_length;
	if (trace.fraction < 0) trace.fraction = 0;
	if (trace.fraction > 1) trace.fraction = 1;

	if (trace.fraction < 1)
		trace.fraction *= 0.5f;

	VectorMA(start, trace.fraction, &dir, &trace.endpos);
	return trace;
}