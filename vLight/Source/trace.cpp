
#include "std_lib.h"
#include "trace.h"
#include "world.h"

extern world_t *world;
#define ON_EPSILON 0.03125f


/**************************************************************
collide a ray with everything in the world
**************************************************************/
void ray(int node, vector_t *start, vector_t *end, vector_t *endpos)
{
	float dstart, dend;
	dstart = dot((*start), world->planes[world->nodes[node].plane].norm) - world->planes[world->nodes[node].plane].d;
	dend   = dot((*end)  , world->planes[world->nodes[node].plane].norm) - world->planes[world->nodes[node].plane].d;

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
		nnode = world->nodes[node].children[n];
		// node
		if (nnode>0)
		{
			ray(nnode, start, &inter, endpos);
			// if we're not at the intersection, we hit something - return
			if (!VectorCompare(&inter, endpos))
				return;
		}

		// leaf
		else
		{
			// stop at beginning
			if (world->leafs[-nnode].contents & CONTENTS_SOLID)
			{
				VectorCopy((*start), (*endpos));
				return;
			}

			// else we can move all the way through it - FIXME add contents??
		}


		// we made it through near side - collide with far side
		nnode = world->nodes[node].children[1-n];

		// node
		if (nnode>0)
		{
			ray(nnode, &inter, end, endpos);
			if (!VectorCompare(end, endpos))
				return;
		}

		// leaf
		else
		{
			if (world->leafs[-nnode].contents & CONTENTS_SOLID)
			{
				VectorCopy(inter, (*endpos));
				return;
			}
		}

		// if we're here, we didn't hit anything
		VectorCopy((*end), (*endpos));
		return;
	}

	// else we're entirely in the near node
	nnode = world->nodes[node].children[n];

	// node
	if (nnode>0)
	{
		ray(nnode, start, end, endpos);
		return;
	}

	// leaf
	if (world->leafs[-nnode].contents & CONTENTS_SOLID)
	{
		VectorCopy((*start), (*endpos));
	}
	else
	{
		VectorCopy((*end), (*endpos));
	}
}



/**************************************************************
collide a bounding box with everything in the world
**************************************************************/
trace_t trace(vector_t *start, vector_t *end)
{
	trace_t trace;
	trace.fraction = 1;
	VectorCopy((*end), trace.endpos);

	if (VectorCompare2(start, end, 0.01f))
		return trace;

	// find the length/direction of a full trace
	vector_t dir;
	VectorSub((*end), (*start), dir);
	float want_length = VectorLength(&dir);


	vector_t bend;
	ray(0, start, end, &trace.endpos);
	VectorSub(trace.endpos, (*start), bend);

	trace.fraction = VectorLength(&bend) / want_length;
	if (trace.fraction < 0) trace.fraction = 0;
	if (trace.fraction > 1) trace.fraction = 1;

	return trace;
}
