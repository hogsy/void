#ifndef TEXTUREDEFINITIONS_H
#define TEXTUREDEFINITIONS_H

#include "Ren_cache.h"
#include "Standard.h"

typedef int dimension_t[2];

struct tex_t
{
	tex_t() 
	{ 
		num_textures = 0;
		num_lightmaps = 0;
		base_names = 0;
		tex_names = 0;
		light_names = 0;
		dims = 0;
		polycaches = 0;
	}

	unsigned int num_textures;
	unsigned int num_lightmaps;
	GLuint		*base_names;
	GLuint		*tex_names;
	GLuint		*light_names;
	dimension_t	*dims;
	cpoly_t		**polycaches;
};

extern tex_t *tex;

#endif
