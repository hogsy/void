
#ifndef VIS_H
#define VIS_H

#include "Com_vector.h"

typedef struct vportal_s
{
	int			nodes[2];
	plane_t*	planes[2];
	vector_t	verts[2][32];
	int			num_verts;

	struct vportal_s *next[2];
} vportal_t;



void vis_run(const char * szPath, const char * szFileName);

#endif

