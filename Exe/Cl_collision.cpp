



//#include "Cl_main.h"
#include "Cl_collision.h"

extern world_t	 *g_pWorld;


// FIXME - should collide with all objects in the sector first, then objects
// against the player, then against the walls and ???walls against player???
// since it's just a single ray, and no objects, just collide against walls right now



#define ON_EPSILON 0.03125f


/**************************************************************
collide a ray with everything in the world
**************************************************************/
plane_t* ray(int node, const vector_t &start, const vector_t &end, float *endfrac, plane_t *lastplane)
{
	plane_t *hitplane;
	float dstart, dend;
	plane_t *plane;
	int n, nnode;

	if (dot(start, g_pWorld->planes[g_pWorld->nodes[node].plane].norm) - g_pWorld->planes[g_pWorld->nodes[node].plane].d >= 0)
	{
		n = 0;
		plane = &g_pWorld->planes[g_pWorld->nodes[node].plane];
	}
	else
	{
		n = 1;
		plane = &g_pWorld->planes[g_pWorld->nodes[node].plane^1];
	}

	dstart = dot(start, plane->norm) - plane->d;
	dend   = dot(end  , plane->norm) - plane->d;

	float frac = (dstart)/(dstart-dend);

	// we're going to hit a plane that separates 2 child nodes
	if (0<frac && frac<1)
	{
		frac = (dstart-ON_EPSILON)/(dstart-dend);
		if (frac < 0)	frac = 0;

	// collide through near node
		nnode = g_pWorld->nodes[node].children[n];
		// node
		if (nnode>0)
		{
			hitplane = ray(nnode, start, end, endfrac, lastplane);
			// if we're not at the intersection, we hit something - return
			if (hitplane)
				return hitplane;
		}

		// leaf
		else
		{
			// stop at beginning if we're in a solid node
			if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
				return lastplane;

			// else we can move all the way through it - FIXME add other contents tests??
		}


	// we made it through near side - collide with far side
		nnode = g_pWorld->nodes[node].children[1-n];

		// node
		if (nnode>0)
		{
			*endfrac = frac;
			hitplane = ray(nnode, start, end, endfrac, plane);
			if (hitplane)
				return hitplane;
		}

		// leaf
		else
		{
			if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
			{
				*endfrac = frac;
				return plane;
			}
		}

		// if we're here, we didn't hit anything
		*endfrac = 1;
		return 0;
	}

	// else we're entirely in the near node
	nnode = g_pWorld->nodes[node].children[n];

	// node
	if (nnode>0)
		return ray(nnode, start, end, endfrac, lastplane);

	// leaf
	if (g_pWorld->leafs[-nnode].contents & CONTENTS_SOLID)
		return lastplane;


	*endfrac = 1;
	return 0;
}


/**************************************************************
collide a bounding box with everything in the world
**************************************************************/
trace_t trace(vector_t &start, vector_t &end, vector_t *mins, vector_t *maxs)
{
	trace_t trace;
	trace.fraction = 1;
	trace.plane = 0;
	VectorCopy(end, trace.endpos);

	// find the length/direction of a full trace
	vector_t dir;
	VectorSub(end, start, dir);

	float frac;
	plane_t*	hitplane;
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
			VectorAdd(bbox[i], start, bbox[i]);
			VectorAdd(bbox[i], dir, bend);

			frac = 0;
			hitplane = ray(0, bbox[i], bend, &frac, 0);

			// find the length this corner went
			if ((trace.fraction > frac) && hitplane)
			{
				trace.fraction = frac;
				trace.plane = hitplane;

				if (trace.fraction==0)
					break;
			}
		}
	}

	// just trace a line
	else
	{
		trace.fraction = 0;
		trace.plane = ray(0, start, end, &trace.fraction, 0);
	}

	VectorMA(&start, trace.fraction, &dir, &trace.endpos);
	return trace;
}



