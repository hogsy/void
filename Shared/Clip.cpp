


/////////////////////   CLIPPING CODE   ///////////////////////

#include "clip.h"
#include <memory.h>	//for memcpy

float *sides;
poly_t clippoly;

void clip_poly_to_plane (poly_t *oldpoly, plane_t *plane)
{
// make sure we wont go over our bounds
	if (oldpoly->num_vertices >= 64)
		return;


	int v = 0;
	vector_t a, b;	// points to clip between
//	tvert_t ta, tb, la, lb;
	int nextvert;
	float	scale;


// calc all the distances right off
	for (v = 0; v < oldpoly->num_vertices; v++)
		sides[v] = dot(oldpoly->vertices[v], plane->norm) - plane->d;

	clippoly.num_vertices = 0;

	for (v = 0; v < oldpoly->num_vertices; v++)
	{
        nextvert = (v + 1) % oldpoly->num_vertices;

	// store temp 3d vertices
		VectorCopy(oldpoly->vertices[v], a);
		VectorCopy(oldpoly->vertices[nextvert], b);
/*
	// store temp tex coords
		ta.s = oldpoly->vertices[v].s;
		ta.t = oldpoly->vertices[v].t;
		tb.s = oldpoly->vertices[nextvert].s;
		tb.t = oldpoly->vertices[nextvert].t;

	// store temp light coords
		la.s = oldpoly->vertices[v].ls;
		la.t = oldpoly->vertices[v].lt;
		lb.s = oldpoly->vertices[nextvert].ls;
		lb.t = oldpoly->vertices[nextvert].lt;
*/
	// ok, we have 2 points to clip between now - a and b
		if (sides[v] < -EPSILON && sides[nextvert] < -EPSILON)	// both points are outside - bail early
			continue;


		// first point is inside
		if (sides[v] >= -EPSILON)
		{
			VectorCopy(a, clippoly.vertices[clippoly.num_vertices]);
/*
			clippoly.vertices[clippoly.num_vertices].s = ta.s;
			clippoly.vertices[clippoly.num_vertices].t = ta.t;
			clippoly.vertices[clippoly.num_vertices].ls = la.s;
			clippoly.vertices[clippoly.num_vertices].lt = la.t;
*/
			clippoly.num_vertices++;


		// make sure we don't go over our bounds - will at most make 1 more vert than before
			if (clippoly.num_vertices > oldpoly->num_vertices)
				break;
		}


		// first point is inside, second is outside or vice versa find the intersection
		if ((sides[v] > EPSILON && sides[nextvert] < -EPSILON) ||
			(sides[v] < -EPSILON && sides[nextvert] > EPSILON))
		{
			// now we're gonna have to find an intersection
			scale = (sides[v]) / (sides[v] - sides[nextvert]);

			clippoly.vertices[clippoly.num_vertices].x = a.x + scale*(b.x-a.x);
			clippoly.vertices[clippoly.num_vertices].y = a.y + scale*(b.y-a.y);
			clippoly.vertices[clippoly.num_vertices].z = a.z + scale*(b.z-a.z);
/*
			clippoly.vertices[clippoly.num_vertices].s = ta.s + scale*(tb.s-ta.s);
			clippoly.vertices[clippoly.num_vertices].t = ta.t + scale*(tb.t-ta.t);
			clippoly.vertices[clippoly.num_vertices].ls = la.s + scale*(lb.s-la.s);
			clippoly.vertices[clippoly.num_vertices].lt = la.t + scale*(lb.t-la.t);
*/
			clippoly.num_vertices++;


		// make sure we don't go over our bounds - will at most make 1 more vert than before
			if (clippoly.num_vertices > oldpoly->num_vertices)
				break;
		}
	}

	oldpoly->num_vertices = clippoly.num_vertices;

// not a full poly
	if (oldpoly->num_vertices < 3)
	{
		oldpoly->num_vertices = 0;
		return;
	}

// copy all of the verts out of the tmp struct
	for (v = 0; v < oldpoly->num_vertices; v++)
		memcpy(&oldpoly->vertices[v], &clippoly.vertices[v], sizeof(rvertex_t));
}


/******************************************************************************
clip a poly to a frustrum
******************************************************************************/
void clip_poly_to_frust(poly_t *poly, frustum_t *frust)
{
	unsigned int p;

// clip to poly
	for (p = 0; p < frust->num_planes; p++)
	{
		if (poly->num_vertices >2)
			clip_poly_to_plane(poly, &frust->planes[p]);
	}
}



/******************************************************************************
build a plane using 2 points and the origin
******************************************************************************/
void clip_build_plane(vector_t *a, vector_t *b, plane_t *p)
{

	_CrossProduct (a, b, &p->norm);
	VectorNormalize (&p->norm);

	// calculate distance to origin	(should be 0 for all planes through the origin)
	p->d = 0;
}


/******************************************************************************
build a plane using 3 arbitrary points
******************************************************************************/
void clip_build_plane3(vector_t *a, vector_t *b, vector_t *c, plane_t *p)
{
	vector_t u, v;
	VectorSub((*c), (*a), u);
	VectorSub((*b), (*a), v);

	_CrossProduct(&u, &v, &p->norm);
	VectorNormalize (&p->norm);
	p->d = dot(p->norm, (*a));
}


/******************************************************************************
create a frustrum from the poly - don't make a near z plane
bue****************************************************************************/
void clip_build_frust_from_poly(poly_t *poly, vector_t *p, frustum_t *frust)
{
	unsigned int plane, nextvert;
	vector_t a, b;	// temp vects

	frust->num_planes = poly->num_vertices;

	for (plane = 0; plane < frust->num_planes; plane++)
	{
		nextvert = (plane + 1) % poly->num_vertices;

		VectorCopy(poly->vertices[plane], a);
		VectorCopy(poly->vertices[nextvert], b);

		clip_build_plane3(p, &b, &a, &frust->planes[plane]);
	}
}


/******************************************************************************
find the intersection of a line and a plane
******************************************************************************/
void clip_intersect(vector_t *p1, vector_t *p2, plane_t *plane, vector_t *intersect)
{
	float d1, d2, scale;

	VectorCopy((*p2), (*intersect));

	d1 = dot ((*p1), plane->norm) - plane->d;
	d2 = dot ((*p2), plane->norm) - plane->d;

	if (d1 > EPSILON && d2 > EPSILON)
		return;	// ray is on front side of face

	if (d1 < EPSILON)
		return;	// ray starts on back side of face

// clip the ray to the plane
#ifdef _EXE
	scale = (d1 - EPSILON) / (d1 - d2);
#else
	scale = d1 / (d1 - d2);
#endif

	intersect->x = p1->x + scale * (p2->x - p1->x);
	intersect->y = p1->y + scale * (p2->y - p1->y);
	intersect->z = p1->z + scale * (p2->z - p1->z);
}


