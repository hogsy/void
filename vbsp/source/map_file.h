
#ifndef MAP_FILE_H
#define MAP_FILE_H

#include "3dmath.h"
#include "bsp_file.h"

#define MAX_MAP_ENTITIES		2048
#define MAX_MAP_BRUSHES			(8192*2)
#define MAX_MAP_BRUSH_SIDES		(65536*2) // need sides for portals too
#define MAX_MAP_TEXINFOS		16384
#define MAX_MAP_KEYS			8192

#define MAX_MAP_PLANES			65536
#define MAX_MAP_NODES			65536



typedef struct				// same fields as in qbsp3
{
	char	name[32];
	int		shift[2];
	int		rotation;
	float	scale[2];
	int		flags;
	int		value;
} map_texinfo_t;


typedef struct
{
	int 		plane;
	int			texinfo;
	int			flags;
} map_brush_side_t;


typedef struct
{
	int		first_side;
	int		num_sides;
	int		contents;
} map_brush_t;



typedef struct
{
	int		first_key;
	int		num_keys;

	int		first_brush;
	int		num_brushes;

} map_entity_t;



bool load_map(char *path);
int get_worldspawn(void);



//=======================================================

extern	int					num_map_entities;
extern	map_entity_t		map_entities[MAX_MAP_ENTITIES];

extern	int					num_map_brushes;
extern	map_brush_t			map_brushes[MAX_MAP_BRUSHES];

extern	int					num_map_brush_sides;
extern	map_brush_side_t	map_brush_sides[MAX_MAP_BRUSH_SIDES];

extern	int					num_map_texinfos;
extern	map_texinfo_t		map_texinfos[MAX_MAP_TEXINFOS];

extern	int					num_keys;
extern	key_t				keys[MAX_MAP_KEYS];

//=======================================================




#endif
