#ifndef VOID_CLIP_H
#define VOID_CLIP_H

#define EPSILON .001f

#include "Com_vector.h"
#include "Com_3dstructs.h"

typedef struct frustum_t
{
	unsigned int num_planes;
	plane_t	*planes;
} frustum_t;

void clip_build_frust_from_poly(poly_t *poly, vector_t *p, frustum_t *frust);
void clip_poly_to_frust(poly_t *poly, frustum_t *frust);


void clip_poly_to_plane (poly_t *oldpoly, plane_t *plane);
void clip_build_plane3(vector_t *a, vector_t *b, vector_t *c, plane_t *p);
void clip_intersect(vector_t *p1, vector_t *p2, plane_t *plane, vector_t *intersect);

#endif
