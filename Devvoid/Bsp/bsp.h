#ifndef BSP_H
#define BSP_H

#include "Com_vector.h"
#include "map_file.h"

#define MAX_FACE_VERTS	32


typedef struct bsp_brush_side_s
{
	int			num_verts;
	vector_t	verts[MAX_FACE_VERTS];

	int		texinfo;
	int		plane;
	bool	visible;

	int		flags;

	struct bsp_brush_side_s *next;
} bsp_brush_side_t;


typedef struct bsp_brush_s
{
	bsp_brush_side_t	*sides;
	struct	bsp_brush_s	*next;
	
	int		contents;
	int		mins[3];
	int		maxs[3];
} bsp_brush_t;


typedef struct bsp_node_s
{
	int			plane;

	bsp_brush_t			*volume;	// brush which is made of all parent planes
	struct bsp_node_s	*parent;


	// node specific
	struct bsp_node_s	*children[2];

	// leaf specific
	bsp_brush_t	*brushes;
	struct		portal_s	*portals;
	int			contents;
	bool		outside;
	int			fleaf;		// leaf number in file - needed for portal file

} bsp_node_t;


//=======================================================

extern	int					num_planes;
extern	plane_t				planes[MAX_MAP_PLANES];

//=======================================================



void bsp_partition(bsp_node_t *n);
bsp_brush_t* new_bsp_brush(void);
void free_bsp_brush(bsp_brush_t *b);
bsp_brush_side_t* new_bsp_brush_side(void);
void free_bsp_brush_side(bsp_brush_side_t *s);
bsp_brush_t* copy_brush(bsp_brush_t *in);
void copy_brush2(bsp_brush_t *in,  bsp_brush_t *out);
int clip_side(bsp_brush_side_t *s, int plane);
void clip_brush(bsp_brush_t *b, int plane);
void calc_brush_bounds(bsp_brush_t *b);
int bsp_test_brush(bsp_brush_t *b, int plane, bool *epsilon);
void reset_bsp_brush(void);
void bsp_brush_split(bsp_brush_t *b, bsp_brush_t **front, bsp_brush_t **back, int plane);
void make_base_side(bsp_brush_side_t *side);
bsp_brush_t* bsp_build_volume(void);

#endif

