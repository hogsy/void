
#ifndef LIGHTMAP_H
#define LIGHTMAP_H

#include "world.h"
#include "3dmath.h"

/*
=================================
definition of a lightmap file
=================================
*/

#define LIGHT_VERSION	1
#define LIGHT_ID		('v'<<24 | 'l'<<16 | 'i'<<8 | 't')


typedef struct
{
	short	width, height;
	unsigned char	*data;
	vector_t origin;	// top left of the lightmap
	vector_t vright, vdown;	// which way the lightmap is oriented
	plane_t  pleft, ptop;
	double sfactor, tfactor;	// actual dimensions of each texel
} lightmap_t;


typedef struct
{
	vector_t		origin;
	unsigned char	color[3];
	unsigned int	sector;
} light_t;


#define MAX_SECTOR_LIGHTS 16
typedef struct
{
	unsigned int num_lights;
	unsigned int *lights;
} light_sectors_t;


typedef struct
{
	int			 version;	// version of the light info
	unsigned int world_id;	// random number that must be the same as the world one
	unsigned int num_lightmaps;
	unsigned int num_lights;
} light_header_t;


typedef struct
{
	light_header_t	header;
	light_t			*lights;	// light info
	light_sectors_t	*lsectors;	// list of lights that effect each sector
	lightmap_t		*lightmaps;
} light_info_t;




void light_file_write(light_info_t *l, world_t *w, char *filename);

#endif



