
#include <memory.h>
#include <string.h>

#include "Com_vector.h"
#include "std_lib.h"
#include "bsp_file.h"
#include "bsp.h"
#include "entity.h"
#include "com_trace.h"


// required for filesystem
#include "Com_defs.h"
#include "I_file.h"
#include "I_filesystem.h"


// shaders
#include "../Renderer/Rast_main.h"
#include "../Renderer/ShaderManager.h"



#define MAX_MAP_VERTS		65536
#define	MAX_MAP_LEAFS		65536
#define MAX_MAP_TEXTURES	256
#define MAX_MAP_EDGES		256000


//=======================================================

// use the other plane array

int				num_verts;
vector_t		verts[MAX_MAP_VERTS];

int				num_vert_indices;
int				vert_indices[MAX_MAP_BRUSH_SIDES * MAX_FACE_VERTS/2];

int				num_sides;
bspf_side_t		fsides[MAX_MAP_BRUSH_SIDES];

int				num_entities;
bspf_entity_t	fentities[MAX_MAP_ENTITIES];

int				num_brushes;
bspf_brush_t	fbrushes[MAX_MAP_BRUSHES];

int				num_nodes;
bspf_node_t		fnodes[MAX_MAP_NODES];

int				num_leafs;
bspf_leaf_t		fleafs[MAX_MAP_LEAFS];

int				num_textures;
texname_t		ftextures[MAX_MAP_TEXTURES];

int				num_texdefs;
bspf_texdef_t	ftexdefs[MAX_MAP_TEXINFOS];

int				num_edges;
bspf_edge_t		fedges[MAX_MAP_EDGES];

extern bsp_brush_t sky_brush;

//=======================================================

/*
==============
get_texture
==============
*/
int get_texture(char *name)
{
	for (int i=0; i<num_textures; i++)
	{
		if (!strcmp(ftextures[i], name))
			return i;
	}

	// have to add a new one
	if (num_textures == MAX_MAP_TEXTURES)
		Error("too many textures");

	num_textures++;

	strcpy(ftextures[i], name);
	return i;
}

/*
============
tex_map_to_bsp	- carmack
============
*/
void tex_map_to_bsp(map_texinfo_t *tinfo, bspf_texdef_t *tdef, int p)
{
	if (!tinfo->name[0])
		Error("texture has no name");

	memset (tdef, 0, sizeof(bspf_texdef_t));
	tdef->texture = get_texture(tinfo->name);

	//
	// find the right & up vectors
	//

	// total hack to avoid using the vector class
	typedef struct
	{
		float x, y, z;
	} vec_t;

	vec_t	baseaxis[18] =
	{
	{ 0, 0, 1}, {1,0,0}, {0,-1, 0},			// floor
	{ 0, 0,-1}, {1,0,0}, {0,-1, 0},		// ceiling
	{ 1, 0, 0}, {0,1,0}, {0, 0,-1},			// west wall
	{-1, 0, 0}, {0,1,0}, {0, 0,-1},		// east wall
	{ 0, 1, 0}, {1,0,0}, {0, 0,-1},			// south wall
	{ 0,-1, 0}, {1,0,0}, {0, 0,-1}			// north wall
	};


	int		bestaxis;
	float	d, bestdot;
	
	bestdot = 0;
	bestaxis = 0;
	
	for (int i=0 ; i<6 ; i++)
	{
		d = dot (planes[p].norm, baseaxis[i*3]);
		if (d > bestdot)
		{
			bestdot = d;
			bestaxis = i;
		}
	}

	vector_t base[2];
	VectorCopy (baseaxis[bestaxis*3+1], base[0]);
	VectorCopy (baseaxis[bestaxis*3+2], base[1]);


	// we dont do any shifting - maybe in the future
/*
	float shift[2];
	shift[0] = DotProduct (origin, vecs[0]);
	shift[1] = DotProduct (origin, vecs[1]);

	if (!bt->scale[0])
		bt->scale[0] = 1;
	if (!bt->scale[1])
		bt->scale[1] = 1;
*/

// rotate axis
	float sinv, cosv, ang;
	if (tinfo->rotation == 0)
		{ sinv = 0 ; cosv = 1; }
	else if (tinfo->rotation == 90)
		{ sinv = 1 ; cosv = 0; }
	else if (tinfo->rotation == 180)
		{ sinv = 0 ; cosv = -1; }
	else if (tinfo->rotation == 270)
		{ sinv = -1 ; cosv = 0; }
	else
	{
		ang = (float)(tinfo->rotation / 180 * PI);
		sinv = (float)sin(ang);
		cosv = (float)cos(ang);
	}

	int sv, tv;
	if (base[0].x)
		sv = 0;
	else if (base[0].y)
		sv = 1;
	else
		sv = 2;

	if (base[1].x)
		tv = 0;
	else if (base[1].y)
		tv = 1;
	else
		tv = 2;

	float ns, nt;
	for (i=0 ; i<2 ; i++)
	{
		ns = cosv * ((float*)&base[i])[sv] - sinv * ((float*)&base[i])[tv];
		nt = sinv * ((float*)&base[i])[sv] +  cosv * ((float*)&base[i])[tv];
		((float*)&base[i])[sv] = ns;
		((float*)&base[i])[tv] = nt;
	}

	for (i=0 ; i<2 ; i++)
	{
		for (int j=0 ; j<3 ; j++)
			tdef->vecs[i][j] = ((float*)&base[i])[j] / tinfo->scale[i];
	}

	tdef->vecs[0][3] = (float)tinfo->shift[0];
	tdef->vecs[1][3] = (float)tinfo->shift[1];

	int w, h;
	
	g_pShaders->GetDims(tinfo->name, w, h);
	tdef->vecs[0][0] /= w;
	tdef->vecs[0][1] /= w;
	tdef->vecs[0][2] /= w;
	tdef->vecs[0][3] /= w;
	tdef->vecs[1][0] /= h;
	tdef->vecs[1][1] /= h;
	tdef->vecs[1][2] /= h;
	tdef->vecs[1][3] /= h;

}



/*
============
get_texdef
============
*/
int get_texdef(map_texinfo_t *tinfo, int plane)
{
	if (num_texdefs == MAX_MAP_TEXINFOS)
		Error("too many texdefs");

	tex_map_to_bsp(tinfo, &ftexdefs[num_texdefs], plane);
	num_texdefs++;

	return num_texdefs-1;
}



//=======================================================


/*
===========
get_vert
===========
*/
#define VERT_SNAP 0.3
#define VERT_INT  0.01
int get_vert(vector_t &v)
{
	// if it's close to an integer, snap it to that
	if (fabs(v.x - (int)floor(v.x+0.5)) < VERT_INT)		v.x = (float)((int)floor(v.x+0.5));
	if (fabs(v.y - (int)floor(v.y+0.5)) < VERT_INT)		v.y = (float)((int)floor(v.y+0.5));
	if (fabs(v.z - (int)floor(v.z+0.5)) < VERT_INT)		v.z = (float)((int)floor(v.z+0.5));


	// try to find a close vert
	for (int i=0; i<num_verts; i++)
	{
		if ((fabs(v.x - verts[i].x) < VERT_SNAP) &&
			(fabs(v.y - verts[i].y) < VERT_SNAP) &&
			(fabs(v.z - verts[i].z) < VERT_SNAP))

			return i;
	}

	// else we have to add a new one
	if (num_verts == MAX_MAP_VERTS)
		Error("too many verts");
	VectorCopy(v, verts[num_verts]);
	num_verts++;
	return i;
}


/*
===========
side_area
===========
*/
float side_area(bsp_brush_side_t *si)
{
	vector_t a, b, cross;
	float area = 0;

	for (int p=1; p<si->num_verts-1; p++)
	{
		VectorSub(si->verts[p  ], si->verts[0], a);
		VectorSub(si->verts[p+1], si->verts[0], b);

		_CrossProduct(&a, &b, &cross);

		area += VectorLength(&cross) / 2;
	}
	return area;
}


/*
===========
fill_sides
===========
*/
int fill_sides(bsp_brush_side_t *si)
{
	int num = 0;

	for (bsp_brush_side_t *s = si; s; s=s->next, num++)
	{
		if (num_sides == MAX_MAP_BRUSH_SIDES)
			Error("too many sides!");

		int thisside = num_sides;
		num_sides++;

		fsides[thisside].lightdef = -1;
		fsides[thisside].plane = s->plane;
		fsides[thisside].first_vert = num_vert_indices;
		fsides[thisside].flags = s->flags;
		fsides[thisside].num_verts  = s->num_verts;
		fsides[thisside].texdef = get_texdef(&map_texinfos[s->texinfo], s->plane);
		fsides[thisside].area = side_area(si);

		for (int v=0; v<s->num_verts; v++)
		{
			if (num_vert_indices == MAX_MAP_BRUSH_SIDES * MAX_FACE_VERTS/2)
				Error("too many vert indices");

			int ver = get_vert(s->verts[v]);
			vert_indices[num_vert_indices] = ver;
			num_vert_indices++;

		}
	}

	return num;
}



/*
===========
fill_brushes
===========
*/
int fill_brushes(bsp_brush_t *br)
{
	int num = 0;
	for (bsp_brush_t *b = br; b; b=b->next, num++)
	{
		if (num_brushes == MAX_MAP_BRUSHES)
			Error("too many brushes!");

		int thisbrush = num_brushes;
		num_brushes++;

		fbrushes[thisbrush].first_side = num_sides;
		fbrushes[thisbrush].num_sides = fill_sides(br->sides);

		fbrushes[thisbrush].mins[0] = br->mins[0];
		fbrushes[thisbrush].mins[1] = br->mins[1];
		fbrushes[thisbrush].mins[2] = br->mins[2];
		fbrushes[thisbrush].maxs[0] = br->maxs[0];
		fbrushes[thisbrush].maxs[1] = br->maxs[1];
		fbrushes[thisbrush].maxs[2] = br->maxs[2];
	}

	return num;
}


/*
===========
fill_leaf
===========
*/
int fill_leaf(bsp_node_t *n)
{
	// outside node always 0
	if (n->outside)
	{
		n->fleaf = 0;
		return 0;
	}

	if (num_leafs == MAX_MAP_NODES)
		Error("too many leafs!");

	int thisleaf = num_leafs;
	num_leafs++;

	fleafs[thisleaf].first_brush = num_brushes;
	fleafs[thisleaf].num_brushes = fill_brushes(n->brushes);
	fleafs[thisleaf].mins[0] = n->volume->mins[0];
	fleafs[thisleaf].mins[1] = n->volume->mins[1];
	fleafs[thisleaf].mins[2] = n->volume->mins[2];
	fleafs[thisleaf].maxs[0] = n->volume->maxs[0];
	fleafs[thisleaf].maxs[1] = n->volume->maxs[1];
	fleafs[thisleaf].maxs[2] = n->volume->maxs[2];
	fleafs[thisleaf].contents= n->contents;
	fleafs[thisleaf].vis = 0;

	n->fleaf = thisleaf;

	return thisleaf;
}


/*
===========
fill_nodes
===========
*/
int fill_nodes(bsp_node_t *n)
{
	if (n->outside)
		return 0;

	// see if it's a leaf
	if (n->plane == -1)
		return (-1*fill_leaf(n));


	// fill the node
	if (num_nodes == MAX_MAP_NODES)
		Error("too many nodes!");

	if (!n->children[0] || !n->children[1])
		Error("node without children!!!");


	int thisnode = num_nodes;
	num_nodes++;

	fnodes[thisnode].plane = n->plane;
	fnodes[thisnode].children[0] = fill_nodes(n->children[0]);
	fnodes[thisnode].children[1] = fill_nodes(n->children[1]);

	fnodes[thisnode].mins[0] = n->volume->mins[0];
	fnodes[thisnode].mins[1] = n->volume->mins[1];
	fnodes[thisnode].mins[2] = n->volume->mins[2];
	fnodes[thisnode].maxs[0] = n->volume->maxs[0];
	fnodes[thisnode].maxs[1] = n->volume->maxs[1];
	fnodes[thisnode].maxs[2] = n->volume->maxs[2];

	return thisnode;
}


/*
==============
create all edges
==============
*/
void fill_edges(void)
{
	bspf_edge_t edge;

	for (int b=0; b<num_brushes; b++)
	{
		fbrushes[b].first_edge = num_edges;
		fbrushes[b].num_edges = 0;
		
		int ends = fbrushes[b].first_side + fbrushes[b].num_sides;
		for (int s1=fbrushes[b].first_side; s1<ends; s1++)
		{
			int endv = fsides[s1].first_vert + fsides[s1].num_verts;
			for (int v=fsides[s1].first_vert; v<endv; v++)
			{
				int nv = ((v-fsides[s1].first_vert+1) % fsides[s1].num_verts) + fsides[s1].first_vert;

				edge.sides[0] = s1;
				edge.sides[1] = -1;
				edge.verts[0] = vert_indices[v];
				edge.verts[1] = vert_indices[nv];

				// find the side we're sharing with
				for (int s2=fbrushes[b].first_side; s2<ends; s2++)
				{
					int endv2 = fsides[s2].first_vert + fsides[s2].num_verts;
					for (int v2=fsides[s2].first_vert; v2<endv2; v2++)
					{
						if (s1 == s2)
							continue;

						int nv2 = ((v2-fsides[s2].first_vert+1) % fsides[s2].num_verts) + fsides[s2].first_vert;

						// is this edge the same?
						if ((vert_indices[v2] != edge.verts[0]) && (vert_indices[v2] != edge.verts[1]))
							continue;

						if ((vert_indices[nv2] != edge.verts[0]) && (vert_indices[nv2] != edge.verts[1]))
							continue;

						// else we have each vert being one of the ones in the edge
						edge.sides[1] = s2;
						break;
					}
					
					// we found a match
					if (v2 != endv2)
						break;
				}


				// compare v-nv edge to all our other edges if we have a 2 sided edge
				if (edge.sides[1] != -1)
				{
					int ende = fbrushes[b].first_edge + fbrushes[b].num_edges;
					for (int e=fbrushes[b].first_edge; e<ende; e++)
					{
						if ((fedges[e].sides[0] != edge.sides[0]) && (fedges[e].sides[1] != edge.sides[0]))
							continue;

						if ((fedges[e].sides[0] != edge.sides[1]) && (fedges[e].sides[1] != edge.sides[1]))
							continue;

						if ((fedges[e].verts[0] != edge.verts[0]) && (fedges[e].verts[1] != edge.verts[0]))
							continue;

						if ((fedges[e].verts[0] != edge.verts[1]) && (fedges[e].verts[1] != edge.verts[1]))
							continue;

						// we're here -> we have a match
						edge.sides[0] = -1;
						break;
					}
				}

				// finally add the edge
				if (edge.sides[0] != -1)
				{
					if (num_edges == MAX_MAP_EDGES)
						Error("too many edges!!");

					fedges[num_edges].sides[0] = edge.sides[0];
					fedges[num_edges].sides[1] = edge.sides[1];
					fedges[num_edges].verts[0] = edge.verts[0];
					fedges[num_edges].verts[1] = edge.verts[1];
					
					num_edges++;
					fbrushes[b].num_edges++;
				}
			}
		}
	}

/*
	for (int b=0; b<num_brushes; b++)
	{
		fbrushes[b].first_edge = num_edges;
		fbrushes[b].num_edges = 0;

		int ends = fbrushes[b].first_side + fbrushes[b].num_sides;
		for (int s1=fbrushes[b].first_side; s1<ends; s1++)
		{

			int endv1 = fsides[s1].first_vert + fsides[s1].num_verts;
			for (int v=fsides[s1].first_vert; v<endv1; v++)
			{
				int nv = (v-fsides[s1].first_vert+1)%fsides[s1].num_verts + fsides[s1].first_vert;
				int es2 = -1;


				for (int s2=fbrushes[b].first_side; s2<ends; s2++)
				{
					if (s1==s2)	// cant have an edge with itself
						continue;

					// compare it to all s2's verts
					int endv2 = fsides[s2].first_vert + fsides[s2].num_verts;
					for (int cv=fsides[s2].first_vert; cv<endv2; cv++)
					{
						int ncv = (cv-fsides[s2].first_vert+1)%fsides[s2].num_verts + fsides[s2].first_vert;

						if (!(((vert_indices[v]  == vert_indices[cv]) &&
							  (vert_indices[nv] == vert_indices[ncv])) ||
							 ((vert_indices[v]  == vert_indices[ncv]) &&
							  (vert_indices[nv] == vert_indices[cv]))))
							 continue;	// not a shared edge


						// the edge is shared, make sure we dont already have it
						es2 = s2;
						int ende = fbrushes[b].first_edge + fbrushes[b].num_edges;
						for (int e=fbrushes[b].first_edge; e<ende; e++)
						{
							if ((((vert_indices[v]  == fedges[e].verts[0]) &&
								 (vert_indices[nv] == fedges[e].verts[1])) ||
								((vert_indices[v] == fedges[e].verts[1]) &&
								 (vert_indices[nv]  == fedges[e].verts[0]))) &&
							   (((fedges[e].sides[0] == s1) && (fedges[e].sides[1] == s2)) ||
								((fedges[e].sides[1] == s1) && (fedges[e].sides[0] == s2))))
							{
								goto skip_edge;
							}
						}

						// must not have found one that matches
//						if ((e != ende) || (fbrushes[b].num_edges == 0))
						{
							goto add_edge;
						}
					}
				}

add_edge:;
				// finally, we can add the edge
				if (num_edges == MAX_MAP_EDGES)
					Error("too many edges");

				fedges[num_edges].sides[0] = s1;
				fedges[num_edges].sides[1] = es2;
				fedges[num_edges].verts[0] = vert_indices[v];
				fedges[num_edges].verts[1] = vert_indices[nv];
				num_edges++;
				fbrushes[b].num_edges++;
skip_edge:;

			}
		}
	}
*/
}


/*
============
fill_ent
============
*/
void fill_ent(entity_t *ent)
{
	fentities[num_entities].first_key = num_keys;
	fentities[num_entities].first_node = num_nodes;
	fentities[num_entities].num_keys = 0;
	fentities[num_entities].num_nodes = 0;

	// fill keys in
	for (vkey_t *k=ent->key; k; k=k->next)
	{
		strcpy(keys[num_keys].name, k->k.name);
		strcpy(keys[num_keys].value, k->k.value);

		num_keys++;
		fentities[num_entities].num_keys++;
	}

	fill_nodes(ent->root);
	fentities[num_entities].num_nodes = num_nodes - fentities[num_entities].first_node;

	num_entities++;
}



//=======================================================
// carmack style bsp file structure
//=======================================================

bspf_header_t header;
FILE* fout;

/*
============
add_lump
============
*/
void add_lump(int l, void *data, int size)
{
	header.lumps[l].length = size;
	header.lumps[l].offset = ftell(fout);

	fwrite(data, 1, size, fout);
}


/*
============
write_bsp
============
*/
void write_bsp(entity_t *ents, char *file)
{

	v_printf("\nwriting bsp file %s\n", file);

	// load shaders
	g_pShaders = new CShaderManager();


	// build all our arrays
	num_verts			= 0;
	num_vert_indices	= 0;
	num_nodes			= 0;
	num_brushes			= 0;
	num_sides			= 0;
	num_texdefs			= 0;
	num_textures		= 0;
	num_edges			= 0;
	num_entities		= 0;
	num_keys			= 0;

	// leaf 0 is always the outside leaf
	num_leafs			= 1;
	fleafs[0].first_brush = 0;
	fleafs[0].num_brushes = 0;
	fleafs[0].mins[0] = 99999;
	fleafs[0].mins[1] = 99999;
	fleafs[0].mins[2] = 99999;
	fleafs[0].maxs[0] = -99999;
	fleafs[0].maxs[1] = -99999;
	fleafs[0].maxs[2] = -99999;
	fleafs[0].contents= CONTENTS_SOLID;
	fleafs[0].vis	  = 0;

	// brush 0 is always the sky faces
	sky_brush.mins[0] = sky_brush.mins[1] = sky_brush.mins[2] = 0;
	sky_brush.maxs[0] = sky_brush.maxs[1] = sky_brush.maxs[2] = 0;
	fill_brushes(&sky_brush);
	v_printf("%4d sky sides\n", num_sides);


	for (entity_t *e=ents; e; e=e->next)
		fill_ent(e);


// we're not using edges right now, they just take a lot of time to fill
	fill_edges();


	fout = fopen(file, "wb");
	if (!fout)
		Error("couldn't open %s for writing!", file);

	fwrite(&header, 1, sizeof(bspf_header_t), fout);

	memset(&header, 0, sizeof(bspf_header_t));
	header.id = BSP_FILE_ID;
	header.version = BSP_VERSION;

	add_lump(LUMP_NODES,		fnodes,			num_nodes		*sizeof(bspf_node_t));
	add_lump(LUMP_LEAFS,		fleafs,			num_leafs		*sizeof(bspf_leaf_t));
	add_lump(LUMP_PLANES,		planes,			num_planes		*sizeof(plane_t));
	add_lump(LUMP_SIDES,		fsides,			num_sides		*sizeof(bspf_side_t));
	add_lump(LUMP_VERTICES,		verts,			num_verts		*sizeof(vector_t));
	add_lump(LUMP_VERT_INDICES, vert_indices,	num_vert_indices*sizeof(int));
	add_lump(LUMP_BRUSHES,		fbrushes,		num_brushes		*sizeof(bspf_brush_t));
	add_lump(LUMP_TEXDEF,		ftexdefs,		num_texdefs		*sizeof(bspf_texdef_t));
	add_lump(LUMP_TEXNAMES,		ftextures,		num_textures	*sizeof(texname_t));
	add_lump(LUMP_EDGES,		fedges,			num_edges		*sizeof(bspf_edge_t));
	add_lump(LUMP_ENTITIES,		fentities,		num_entities	*sizeof(bspf_entity_t));
	add_lump(LUMP_KEYS,			keys,			num_keys		*sizeof(key_t));

	// never any vis or lighting info
	header.lumps[LUMP_LIGHTDEF].length = 0;
	header.lumps[LUMP_LIGHTDEF].offset = 0;
	header.lumps[LUMP_LEAF_VIS].length = 0;
	header.lumps[LUMP_LEAF_VIS].offset = 0;
	header.lumps[LUMP_LIGHTMAP].length = 0;
	header.lumps[LUMP_LIGHTMAP].offset = 0;


	// rewrite the header with offset info
	fseek(fout, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(bspf_header_t), fout);
	fclose(fout);


	v_printf("%4d entities\n", num_entities);
	v_printf("%4d keys\n", num_keys);
	v_printf("%4d nodes\n", num_nodes);
	v_printf("%4d leafs\n", num_leafs);
	v_printf("%4d planes\n", num_planes);
	v_printf("%4d brushes\n", num_brushes);
	v_printf("%4d brush sides\n", num_sides);
	v_printf("%4d edges\n", num_edges);
	v_printf("%4d vertices\n", num_verts);
	v_printf("%4d vert indices\n", num_vert_indices);
	v_printf("%4d texture names\n", num_textures);
	v_printf("%4d texdefs\n", num_texdefs);

	delete g_pShaders;

}




