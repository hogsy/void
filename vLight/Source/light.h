
#ifndef LIGHT_H
#define LIGHT_H


#include "Com_vector.h"
#include "Com_world.h"


typedef struct
{
	// actual lightmap data
	int		width, height;
	unsigned char	data[32*32*3];

// define the rectange in 3d space
	vector_t	origin;
	vector_t	right, down;
	float		lwidth, lheight;

	int		side;	// side this lightmap goes on
	int		leaf;	// leaf the side is in
} lightmap_t;


typedef struct
{
	vector_t	origin;
	vector_t	color;
	float		intensity;
} light_t;


bool light_run(CWorld *w);
void lightmap_build(int node);
void light_write(void);


#endif

