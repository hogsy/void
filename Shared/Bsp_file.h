
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


// same contents as qbsp
#define	CONTENTS_SOLID			0x00000001		// an eye is never valid in a solid
#define	CONTENTS_WINDOW			0x00000002		// translucent, but not watery
#define	CONTENTS_AUX			0x00000004
#define CONTENTS_SKY			0x00000004		// my sky contents is same sas q2 AUX
#define	CONTENTS_LAVA			0x00000008
#define	CONTENTS_SLIME			0x00000010
#define	CONTENTS_WATER			0x00000020
#define	CONTENTS_MIST			0x00000040
#define	LAST_VISIBLE_CONTENTS	0x00000040


// remaining contents are non-visible, and don't eat brushes
#define	CONTENTS_AREAPORTAL		0x00008000
#define	CONTENTS_PLAYERCLIP		0x00010000
#define	CONTENTS_MONSTERCLIP	0x00020000

// currents can be added to any other contents, and may be mixed
#define	CONTENTS_CURRENT_0		0x00040000
#define	CONTENTS_CURRENT_90		0x00080000
#define	CONTENTS_CURRENT_180	0x00100000
#define	CONTENTS_CURRENT_270	0x00200000
#define	CONTENTS_CURRENT_UP		0x00400000
#define	CONTENTS_CURRENT_DOWN	0x00800000

#define	CONTENTS_ORIGIN			0x01000000	// removed before bsping an entity

#define	CONTENTS_MONSTER		0x02000000	// should never be on a brush, only in game
#define	CONTENTS_DEADMONSTER	0x04000000
#define	CONTENTS_DETAIL			0x08000000	// brushes to be added after vis leafs
#define	CONTENTS_TRANSLUCENT	0x10000000	// auto set if any surface has trans
#define	CONTENTS_LADDER			0x20000000




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


