#include "World.h"

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

//Include proper project header
#ifdef _VOID_EXE_
	#include "../Exe/Source/Sys_hdr.h"
	#define PRINT ComPrintf

#elif defined _VVIS
	#include "../vvis/source/std_lib.h"
	#define PRINT v_printf

#elif defined _VLIGHT_
	#include "../vlight/source/std_lib.h"
	#define PRINT v_printf
#else
	#include "../Exe/Source/Sys_hdr.h"
	#define PRINT ComPrintf
#endif


/**************************************************************
print all info about the world
**************************************************************/
void world_print(world_t *world)
{

}


/**************************************************************
free up all the memory used by the world
**************************************************************/
void world_destroy(world_t *world)
{
	if (!world)
		return;

	if(world->edges)
		delete [] world->edges;

	if (world->textures)
		delete [] world->textures;

	if (world->texdefs)
		delete [] world->texdefs;

	if (world->brushes)
		delete [] world->brushes;

	if (world->iverts)
		delete [] world->iverts;

	if (world->nodes)
		delete [] world->nodes;

	if (world->leafs)
		delete [] world->leafs;

	if (world->planes)
		delete [] world->planes;

	if (world->sides)
		delete [] world->sides;

	if (world->verts)
		delete [] world->verts;

	if (world->entities)
		delete [] world->entities;

	if (world->keys)
		delete [] world->keys;

	if (world->leafvis)
		delete [] world->leafvis;

	if(world->lightdata)
		delete [] world->lightdata;

	if(world->lightdefs)
		delete [] world->lightdefs;

	delete world;
	world = NULL;
}





/**************************************************************
Read a world file from disk
**************************************************************/
/*
===========
load_lump
===========
*/
static bspf_header_t header;

#ifdef _VOID_EXE_

static CFileStream  * m_pFile=0;

int load_lump(int l, void **data)
{
	*data = MALLOC(header.lumps[l].length+1);
	memset(*data, 0, header.lumps[l].length+1);

	m_pFile->Seek(header.lumps[l].offset, SEEK_SET);
	m_pFile->Read(*data, header.lumps[l].length, 1);

	return header.lumps[l].length;
}

#else

static FILE *bsp_file;

int load_lump(int l, void **data)
{
	*data = (void*) new unsigned char[header.lumps[l].length+1];
	memset(*data, 0, header.lumps[l].length+1);

	fseek(bsp_file, header.lumps[l].offset, SEEK_SET);
	fread(*data, 1, header.lumps[l].length, bsp_file);
	return header.lumps[l].length;
}


#endif


world_t* world_read(char *filename)
{
#ifdef _VOID_EXE_
	
	m_pFile = new CFileStream();

	if(!m_pFile->Open(filename))
	{
		PRINT("World_Read:Could not open %s\n",filename);
		delete m_pFile;
		m_pFile = 0;
		return 0;
	}

	m_pFile->Read(&header,sizeof(bspf_header_t),1);
	if (header.id != BSP_FILE_ID)
	{
		PRINT("World_Read: %s not a void bsp file!", filename);
		m_pFile->Close();
		delete m_pFile;
		m_pFile = 0;
		return 0;
	}

	if (header.version != BSP_VERSION)
	{
		PRINT("World_Read: bsp version %d, need %d\n", header.version, BSP_VERSION);
		m_pFile->Close();
		delete m_pFile;
		m_pFile = 0;
		return 0;
	}

	world_t *w = (world_t*)MALLOC(sizeof(world_t));

	// read in all the lumps
	w->nnodes		= load_lump(LUMP_NODES,	        (void**)&w->nodes)	 / sizeof(bspf_node_t);
	w->nleafs		= load_lump(LUMP_LEAFS,			(void**)&w->leafs)	 / sizeof(bspf_leaf_t);
	w->nplanes		= load_lump(LUMP_PLANES,		(void**)&w->planes)  / sizeof(plane_t);
	w->nsides		= load_lump(LUMP_SIDES,			(void**)&w->sides)	 / sizeof(bspf_side_t);
	w->nverts		= load_lump(LUMP_VERTICES,		(void**)&w->verts)	 / sizeof(vector_t);
	w->niverts		= load_lump(LUMP_VERT_INDICES,	(void**)&w->iverts)	 / sizeof(int);
	w->nbrushes		= load_lump(LUMP_BRUSHES,		(void**)&w->brushes) / sizeof(bspf_brush_t);
	w->ntexdefs		= load_lump(LUMP_TEXDEF,		(void**)&w->texdefs) / sizeof(bspf_texdef_t);
	w->ntextures	= load_lump(LUMP_TEXNAMES,		(void**)&w->textures)/ sizeof(texname_t);
	w->nedges		= load_lump(LUMP_EDGES,			(void**)&w->edges)	 / sizeof(bspf_edge_t);
	w->nentities	= load_lump(LUMP_ENTITIES,		(void**)&w->entities)/ sizeof(bspf_entity_t);
	w->nkeys		= load_lump(LUMP_KEYS,			(void**)&w->keys)	 / sizeof(key_t);
	w->leafvis_size	= load_lump(LUMP_LEAF_VIS,		(void**)&w->leafvis) / w->nleafs;
	w->light_size	= load_lump(LUMP_LIGHTMAP,		(void**)&w->lightdata);
	w->nlightdefs	= load_lump(LUMP_LIGHTDEF,		(void**)&w->lightdefs)/sizeof(bspf_texdef_t);

	m_pFile->Close();
	delete m_pFile;
	m_pFile = 0;
	return w;

#else

	bsp_file = fopen(filename, "rb");
	if (!bsp_file)
	{
		PRINT("couldn't open %s\n", filename);
		return NULL;
	}

	fread(&header, 1, sizeof(bspf_header_t), bsp_file);
	if (header.id != BSP_FILE_ID)
	{
		PRINT("%s not a void bsp file!", filename);
		fclose(bsp_file);
		return NULL;
	}

	if (header.version != BSP_VERSION)
	{
		PRINT("bsp version %d, need %d\n", header.version, BSP_VERSION);
		fclose(bsp_file);
		return NULL;
	}

	world_t *w = new world_t;

	// read in all the lumps
	w->nnodes	= load_lump(LUMP_NODES,			(void**)&w->nodes)	 / sizeof(bspf_node_t);
	w->nleafs	= load_lump(LUMP_LEAFS,			(void**)&w->leafs)	 / sizeof(bspf_leaf_t);
	w->nplanes	= load_lump(LUMP_PLANES,		(void**)&w->planes)  / sizeof(plane_t);
	w->nsides	= load_lump(LUMP_SIDES,			(void**)&w->sides)	 / sizeof(bspf_side_t);
	w->nverts	= load_lump(LUMP_VERTICES,		(void**)&w->verts)	 / sizeof(vector_t);
	w->niverts	= load_lump(LUMP_VERT_INDICES,	(void**)&w->iverts)	 / sizeof(int);
	w->nbrushes	= load_lump(LUMP_BRUSHES,		(void**)&w->brushes) / sizeof(bspf_brush_t);
	w->ntexdefs = load_lump(LUMP_TEXDEF,		(void**)&w->texdefs) / sizeof(bspf_texdef_t);
	w->ntextures= load_lump(LUMP_TEXNAMES,		(void**)&w->textures)/ sizeof(texname_t);
	w->nedges	= load_lump(LUMP_EDGES,			(void**)&w->edges)	 / sizeof(bspf_edge_t);
	w->nentities= load_lump(LUMP_ENTITIES,		(void**)&w->entities)/ sizeof(bspf_entity_t);
	w->nkeys	= load_lump(LUMP_KEYS,			(void**)&w->keys)	 / sizeof(key_t);
	w->leafvis_size	= load_lump(LUMP_LEAF_VIS,	(void**)&w->leafvis) / w->nleafs;
	w->light_size	= load_lump(LUMP_LIGHTMAP,	(void**)&w->lightdata);
	w->nlightdefs	= load_lump(LUMP_LIGHTDEF,	(void**)&w->lightdefs)/sizeof(bspf_texdef_t);

	fclose(bsp_file);
	return w;
#endif
}


/**************************************************************
Get a pointer to a world
**************************************************************/
world_t* world_create(char *filename)
{
	PRINT("Loading world file %s: ", filename);

	world_t	*w = NULL;
	w = world_read(filename);

	PRINT("OK\n");
	return w;
}




//====================================================================
// key access funcs
//====================================================================
char* key_get_value(world_t *w, int ent, char *name)
{
	int end = w->entities[ent].first_key + w->entities[ent].num_keys;
	for (int k=w->entities[ent].first_key; k<end; k++)
	{
		if (strcmp(w->keys[k].name, name) == 0)
			return w->keys[k].value;
	}
	return "";
}

int key_get_int(world_t *w, int ent, char *name)
{
	return atoi(key_get_value(w, ent, name));
}

float key_get_float(world_t *w, int ent, char *name)
{
	return (float)atof(key_get_value(w, ent, name));
}

void key_get_vector(world_t *w, int ent, char *name, vector_t &v)
{
	char *s = key_get_value(w, ent, name);
	if (!s)
		VectorSet(&v, 0, 0, 0);
	else
		sscanf(s, "%f %f %f", &v.x, &v.y, &v.z);
}

