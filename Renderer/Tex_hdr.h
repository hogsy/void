#ifndef TEXTUREDEFINITIONS_H
#define TEXTUREDEFINITIONS_H

#include "Ren_cache.h"

typedef int dimension_t[2];

struct tex_t
{
	tex_t() 
	{ 
		bin_base = -1;
		bin_world = -1;
		bin_light = -1;

		dims = 0;
		for (int i=0; i<CACHE_PASS_NUM; i++)
			polycaches[i] = 0;
	}

	int bin_base;
	int bin_world;
	int bin_light;

	dimension_t	*dims;
	cpoly_t		**polycaches[CACHE_PASS_NUM];
};

extern tex_t *tex;

#endif
