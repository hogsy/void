#ifndef BSP_FILE_H
#define BSP_FILE_H

#define BSP_FILE_ID	('v'<<24 | 'b'<<16 | 's'<<8 | 'p')
#define BSP_VERSION	10


#define LUMP_NODES			0
#define LUMP_LEAFS			1
#define LUMP_PLANES			2
#define LUMP_SIDES			3
#define LUMP_VERTICES		4
#define LUMP_VERT_INDICES	5
#define LUMP_BRUSHES		6
#define LUMP_TEXDEF			7
#define LUMP_TEXNAMES		8
#define LUMP_EDGES			9
#define	LUMP_ENTITIES		10
#define LUMP_KEYS			11
#define LUMP_LEAF_VIS		12
#define LUMP_LIGHTMAP		13
#define LUMP_LIGHTDEF		14
#define LUMP_HEADER			15



typedef char texname_t[32];



#define MAX_KEY_NAME	32
#define MAX_KEY_VALUE	1024
typedef struct
{
	char	name[MAX_KEY_NAME];
	char	value[MAX_KEY_VALUE];
} key_t;


typedef struct
{
	float		vecs[2][4];		// [s/t][xyz offset]
	int			texture;
} bspf_texdef_t;


#define SURF_INVISIBLE	0x00000001
#define SURF_SKY		0x00000004

typedef struct
{
	int		first_vert;
	int		num_verts;
	int		texdef;
	int		lightdef;
	int		plane;
	int		flags;
	float	area;
} bspf_side_t;


typedef struct
{
	int verts[2];
	int sides[2];
} bspf_edge_t;


typedef struct
{
	int		first_side;
	int		num_sides;

	int		first_edge;
	int		num_edges;

	int		mins[3];
	int		maxs[3];
} bspf_brush_t;


typedef struct
{
	int		children[2];
	int		plane;
	int		mins[3];
	int		maxs[3];
} bspf_node_t;


typedef struct
{
	int		first_brush;
	int		num_brushes;

	int		mins[3];
	int		maxs[3];

	int		contents;
	int		vis;	// offset in leaf vis info
} bspf_leaf_t;

typedef struct
{
	int		first_node;
	int		num_nodes;

	int		first_key;
	int		num_keys;
} bspf_entity_t;


typedef struct
{
	int offset;
	int length;
} bspf_lump_t;


typedef struct
{
	int				id;
	int				version;
	bspf_lump_t		lumps[LUMP_HEADER];
} bspf_header_t;


#endif


