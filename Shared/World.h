#ifndef VOID_WORLD_H
#define VOID_WORLD_H

#include "Com_defs.h"
#include "Com_vector.h"
#include "Bsp_file.h"

class CFileStream;

class CWorld
{
public:

	CWorld();
	~CWorld();

	void PrintInfo() const;
	void WriteToFile(char * szFilename);

	//Util Key Access funcs
	int	  GetKeyInt(int ent, const char * keyName) const;
	void  GetKeyVector(int ent, const char * keyName, vector_t &vec) const;
	float GetKeyFloat(int ent, const char * keyName) const;
	const char* GetKeyString(int ent, const char * keyName) const;

	//the world should be loaded and destroyed using these
	static CWorld * CreateWorld(const char * szFileName);
	static void DestroyWorld(CWorld * pworld);

	vector_t		*verts;
	int				*iverts;
	plane_t			*planes;
	bspf_entity_t	*entities;
	bspf_node_t		*nodes;
	bspf_leaf_t		*leafs;
	bspf_brush_t	*brushes;
	bspf_side_t		*sides;
	bspf_edge_t		*edges;
	bspf_texdef_t	*texdefs;
	bspf_texdef_t	*lightdefs;
	texname_t		*textures;
	key_t			*keys;
	unsigned char	*leafvis;
	unsigned char	*lightdata;

	int	nverts;
	int niverts;
	int nplanes;
	int nentities;
	int nnodes;
	int	nleafs;
	int nbrushes;
	int nsides;
	int nedges;
	int nlightdefs;
	int ntexdefs;
	int ntextures;
	int nkeys;
	int leafvis_size;	// size of vis info for one leaf
	int light_size;		// size of all light data

private:

	static int LoadLump(FILE * fp, int l, void **data);
	static int LoadLump(CFileStream &file, int l, void **data);
	
	//Cached world data to prevent 
	//the world from being loaded twice.
	static bspf_header_t m_worldHeader;
	static CWorld * m_pWorld;
	static char     m_szFileName[COM_MAXPATH];
	static int  	m_refCount;
};

#endif











/*
struct world_t
{
	vector_t		*verts;
	int				*iverts;
	plane_t			*planes;
	bspf_entity_t	*entities;
	bspf_node_t		*nodes;
	bspf_leaf_t		*leafs;
	bspf_brush_t	*brushes;
	bspf_side_t		*sides;
	bspf_edge_t		*edges;
	bspf_texdef_t	*texdefs;
	bspf_texdef_t	*lightdefs;
	texname_t		*textures;
	key_t			*keys;
	unsigned char	*leafvis;
	unsigned char	*lightdata;

	int	nverts;
	int niverts;
	int nplanes;
	int nentities;
	int nnodes;
	int	nleafs;
	int nbrushes;
	int nsides;
	int nedges;
	int nlightdefs;
	int ntexdefs;
	int ntextures;
	int nkeys;
	int leafvis_size;	// size of vis info for one leaf
	int light_size;		// size of all light data
};

void world_destroy(world_t *world);
world_t* world_create(char *filename);
void world_write(world_t *world, char *filename);
void world_print(world_t *world);


char* key_get_value(world_t *w, int ent, char *name);
int key_get_int(world_t *w, int ent, char *name);
float key_get_float(world_t *w, int ent, char *name);
void key_get_vector(world_t *w, int ent, char *name, vector_t &v);

*/





/*
#define WORLD_VERSION			7
#define WORLD_ID				('z'<<24 | 'u'<<16 | 'l'<<8 | 'u')

#define MAX_ENTITY_NAME_LENGTH	32
#define MAX_TEXTURE_NAME_LENGTH	32

typedef struct
{
	float s, t;
} tvert_t;


// world vertices - includes an index
typedef struct
{
	float x, y, z;

#ifdef _ZULU	// only progs that have to write a world need the index
	unsigned int index;	// only used in world writing
#endif

#ifdef _VLIGHT
	unsigned int index;	// only used in world writing
#endif

} w_vertex_t;




#define PORTAL_INVALIDATOR	0xffffffff
#define LIGHTMAP_NONE		0xffffffff

// face flags
#define FACE_SKY		0x00000001
#define FACE_MIRROR		0x00000002

// face
typedef struct
{
	unsigned int		flags;
	unsigned int		color;

	unsigned int		num_vertices;	// number of vertices of the face
	unsigned int		portalindex;	// index of where the portal points
	unsigned int		tex_index;		// texture index
	unsigned int		light_index;	// lightmap index

	plane_t				plane;	// plane the face resides in

	struct sector_t		*portal;		// where it is a portal to, null if it's solid

	w_vertex_t			**vertices;	// geometry info
	tvert_t				*tverts;	// texture coords
	tvert_t				*lverts;	// lightmap coords

} face_t;


// entity
#define ENTITY_LIGHT_INDEX 0xffffffff
typedef struct
{
	int		 ent_index;		// which entity it is in the list of entities
	vector_t origin;		// FIXME - add a skin index?
	vector_t angles;
} wentity_t;


// sector
typedef struct sector_t
{
	unsigned int num_faces;	// number of faces in the sector
	face_t		 *faces;

	unsigned int num_entities;
	wentity_t	*entities;			// all of the entities in the level
} sector_t;



// header for world files
typedef struct
{
	int			 version;	// version of the world info
	unsigned int light_id;	// random number that must be the same in the file with the lightmap data
	unsigned int num_textures;
	unsigned int num_entities;
	unsigned int num_vertices;
	unsigned int num_sectors;
} world_header_t;


// here she is, the world
typedef struct
{
	world_header_t	header;

	char		*textures;			// list of texture names
	char		*entities;			// list of entity names - which models are needed
	w_vertex_t	*vertices;			// pointer to the first vertice
	sector_t	*sectors;			// pointer to the first sector
} world_t;

*/


