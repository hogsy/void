#include "World.h"
#include "I_file.h"

//Static data
CWorld * CWorld::m_pWorld=0;
bspf_header_t CWorld::m_worldHeader;
char CWorld::m_szFileName[COM_MAXPATH];
int	 CWorld::m_refCount=0;

//======================================================================================
//======================================================================================

/*
======================================
Constructor
heh, should i bother with this ?
======================================
*/
CWorld::CWorld()
{
	verts=0;
	iverts=0;
	planes=0;
	entities=0;
	nodes=0;
	leafs=0;
	brushes=0;
	sides=0;
	edges=0;
	texdefs=0;
	lightdefs=0;
	textures=0;
	keys=0;
	leafvis=0;
	lightdata=0;

	nverts = niverts = nplanes =  nentities =  nnodes = nleafs = 0;
	nbrushes = nsides = nedges = nlightdefs =  ntexdefs = ntextures =0;
	nkeys = leafvis_size = light_size =0;
}

/*
======================================
Destructor
======================================
*/
CWorld::~CWorld()
{
	if(edges)
		delete [] edges;
	if (textures)
		delete [] textures;
	if (texdefs)
		delete [] texdefs;
	if (brushes)
		delete [] brushes;
	if (iverts)
		delete [] iverts;
	if (nodes)
		delete [] nodes;
	if (leafs)
		delete [] leafs;
	if (planes)
		delete [] planes;
	if (sides)
		delete [] sides;
	if (verts)
		delete [] verts;
	if (entities)
		delete [] entities;
	if (keys)
		delete [] keys;
	if (leafvis)
		delete [] leafvis;
	if(lightdata)
		delete [] lightdata;
	if(lightdefs)
		delete [] lightdefs;
}


/*
======================================
Print some states
======================================
*/
void CWorld::PrintInfo() const
{
}

/*
======================================
Write to file
======================================
*/
void CWorld::WriteToFile(char * szFilename)
{
}

/*
======================================
Util Key Access funcs
======================================
*/
const char* CWorld::GetKeyString(int ent, const char * keyName) const
{
	int end = entities[ent].first_key + entities[ent].num_keys;
	for (int k=entities[ent].first_key; k<end; k++)
	{
		if (strcmp(keys[k].name, keyName) == 0)
			return keys[k].value;
	}
	return 0;
}

int	CWorld::GetKeyInt(int ent, const char * keyName) const
{
	const char * val = GetKeyString(ent,keyName);
	if(val)
		return ::atoi(val);
	return 0;
}

float CWorld::GetKeyFloat(int ent, const char * keyName) const
{
	const char * val = GetKeyString(ent,keyName);
	if(val)
		return ((float)::atof(val));
	return 0;
}

void  CWorld::GetKeyVector(int ent, const char * keyName, vector_t &vec) const
{
	const char * val = GetKeyString(ent,keyName);
	if (!val)
		VectorSet(&vec, 0, 0, 0);
	else
		sscanf(val, "%f %f %f", &vec.x, &vec.y, &vec.z);
}


//======================================================================================
//======================================================================================

int CWorld::LoadLump(FILE * fp, int l, void **data)
{
	*data = (void*) new byte[m_worldHeader.lumps[l].length+1];
	memset(*data, 0, m_worldHeader.lumps[l].length+1);

	fseek(fp, m_worldHeader.lumps[l].offset, SEEK_SET);
	fread(*data, 1, m_worldHeader.lumps[l].length, fp);
	return m_worldHeader.lumps[l].length;
}

int CWorld::LoadLump(CFileStream &file, int l, void **data)
{
	*data = (void*) new byte[m_worldHeader.lumps[l].length+1];
	memset(*data, 0, m_worldHeader.lumps[l].length+1);

	file.Seek(m_worldHeader.lumps[l].offset, SEEK_SET);
	file.Read(*data, m_worldHeader.lumps[l].length, 1);
	return m_worldHeader.lumps[l].length;
}

/*
======================================
World Creation func
======================================
*/
CWorld * CWorld::CreateWorld(const char * szFileName)
{
#ifdef _VOID_EXE_

	// or return cached pointer to client if the local server has alreadly loaded it
	if(!strcmp(m_szFileName,szFileName) && m_pWorld)
	{
		m_refCount++;
		return m_pWorld;
	}

	if(m_pWorld)
	{
		ComPrintf("CWorld::CreateWorld: Free current world (%s) first\n", m_szFileName);
		return 0;
	}
	
	CFileStream fileReader;

	if(!fileReader.Open(szFileName))
	{
		ComPrintf("CWorld::CreateWorld: Could not open %s\n",szFileName);
		fileReader.Close();
		return 0;
	}

	fileReader.Read(&m_worldHeader,sizeof(bspf_header_t),1);
	if (m_worldHeader.id != BSP_FILE_ID)
	{
		ComPrintf("CWorld::CreateWorld: %s not a Void World file!", szFileName);
		fileReader.Close();
		return 0;
	}

	if (m_worldHeader.version != BSP_VERSION)
	{
		ComPrintf("CWorld::CreateWorld: World Version %d, need %d for %s\n", 
							m_worldHeader.version, BSP_VERSION, szFileName);
		fileReader.Close();
		return 0;
	}

	m_pWorld = new CWorld;

	// read in all the lumps
	m_pWorld->nnodes		= LoadLump(fileReader,LUMP_NODES, (void**)&m_pWorld->nodes)	/ sizeof(bspf_node_t);
	m_pWorld->nleafs		= LoadLump(fileReader,LUMP_LEAFS, (void**)&m_pWorld->leafs)	/ sizeof(bspf_leaf_t);
	m_pWorld->nplanes		= LoadLump(fileReader,LUMP_PLANES,(void**)&m_pWorld->planes) / sizeof(plane_t);
	m_pWorld->nsides		= LoadLump(fileReader,LUMP_SIDES,(void**)&m_pWorld->sides)	/ sizeof(bspf_side_t);
	m_pWorld->nverts		= LoadLump(fileReader,LUMP_VERTICES,(void**)&m_pWorld->verts) / sizeof(vector_t);
	m_pWorld->niverts		= LoadLump(fileReader,LUMP_VERT_INDICES,(void**)&m_pWorld->iverts) / sizeof(int);
	m_pWorld->nbrushes		= LoadLump(fileReader,LUMP_BRUSHES,	(void**)&m_pWorld->brushes) / sizeof(bspf_brush_t);
	m_pWorld->ntexdefs		= LoadLump(fileReader,LUMP_TEXDEF,	(void**)&m_pWorld->texdefs) / sizeof(bspf_texdef_t);
	m_pWorld->ntextures		= LoadLump(fileReader,LUMP_TEXNAMES,(void**)&m_pWorld->textures)/ sizeof(texname_t);
	m_pWorld->nedges		= LoadLump(fileReader,LUMP_EDGES,	(void**)&m_pWorld->edges)	 / sizeof(bspf_edge_t);
	m_pWorld->nentities		= LoadLump(fileReader,LUMP_ENTITIES,(void**)&m_pWorld->entities)/ sizeof(bspf_entity_t);
	m_pWorld->nkeys			= LoadLump(fileReader,LUMP_KEYS,	(void**)&m_pWorld->keys)	 / sizeof(key_t);
	m_pWorld->leafvis_size	= LoadLump(fileReader,LUMP_LEAF_VIS,(void**)&m_pWorld->leafvis) / m_pWorld->nleafs;
	m_pWorld->light_size	= LoadLump(fileReader,LUMP_LIGHTMAP,(void**)&m_pWorld->lightdata);
	m_pWorld->nlightdefs	= LoadLump(fileReader,LUMP_LIGHTDEF,(void**)&m_pWorld->lightdefs)/sizeof(bspf_texdef_t);

	fileReader.Close();

	//Cache pointer to the last loaded world
	strcpy(m_szFileName,szFileName);
	m_refCount++;
	return m_pWorld;

#else

	FILE * bsp_file = fopen(szFileName, "rb");
	if (!bsp_file)
	{
		ComPrintf("couldn't open %s\n", filename);
		return NULL;
	}

	fread(&m_worldHeader, 1, sizeof(bspf_header_t), bsp_file);
	if (m_worldHeader.id != BSP_FILE_ID)
	{
		ComPrintf("%s not a void bsp file!", filename);
		fclose(bsp_file);
		return NULL;
	}

	if (m_worldHeader.version != BSP_VERSION)
	{
		ComPrintf("bsp version %d, need %d\n", m_worldHeader.version, BSP_VERSION);
		fclose(bsp_file);
		return NULL;
	}

	CWorld *w = new CWorld;

	// read in all the lumps
	w->nnodes	= LoadLump(bsp_file,LUMP_NODES,			(void**)&w->nodes)	 / sizeof(bspf_node_t);
	w->nleafs	= LoadLump(bsp_file,LUMP_LEAFS,			(void**)&w->leafs)	 / sizeof(bspf_leaf_t);
	w->nplanes	= LoadLump(bsp_file,LUMP_PLANES,		(void**)&w->planes)  / sizeof(plane_t);
	w->nsides	= LoadLump(bsp_file,LUMP_SIDES,			(void**)&w->sides)	 / sizeof(bspf_side_t);
	w->nverts	= LoadLump(bsp_file,LUMP_VERTICES,		(void**)&w->verts)	 / sizeof(vector_t);
	w->niverts	= LoadLump(bsp_file,LUMP_VERT_INDICES,	(void**)&w->iverts)	 / sizeof(int);
	w->nbrushes	= LoadLump(bsp_file,LUMP_BRUSHES,		(void**)&w->brushes) / sizeof(bspf_brush_t);
	w->ntexdefs = LoadLump(bsp_file,LUMP_TEXDEF,		(void**)&w->texdefs) / sizeof(bspf_texdef_t);
	w->ntextures= LoadLump(bsp_file,LUMP_TEXNAMES,		(void**)&w->textures)/ sizeof(texname_t);
	w->nedges	= LoadLump(bsp_file,LUMP_EDGES,			(void**)&w->edges)	 / sizeof(bspf_edge_t);
	w->nentities= LoadLump(bsp_file,LUMP_ENTITIES,		(void**)&w->entities)/ sizeof(bspf_entity_t);
	w->nkeys	= LoadLump(bsp_file,LUMP_KEYS,			(void**)&w->keys)	 / sizeof(key_t);
	w->leafvis_size	= LoadLump(bsp_file,LUMP_LEAF_VIS,	(void**)&w->leafvis) / w->nleafs;
	w->light_size	= LoadLump(bsp_file,LUMP_LIGHTMAP,	(void**)&w->lightdata);
	w->nlightdefs	= LoadLump(bsp_file,LUMP_LIGHTDEF,	(void**)&w->lightdefs)/sizeof(bspf_texdef_t);

	fclose(bsp_file);
	return w;

#endif
}


/*
======================================
Exe should destory the World using this
======================================
*/
void CWorld::DestroyWorld(CWorld * pWorld)
{
	if(!pWorld)
		return;

#ifdef _VOID_EXE
	if(pWorld == m_pWorld)
	{
		m_refCount--;
		if(m_refCount > 0)
			return;
		m_pWorld = 0;
	}
#endif

	delete pWorld;
}
















































#if 0

#include <stdio.h>
#include <memory.h>
#include <stdlib.h>
#include <string.h>

//Include proper project header
#ifdef _VOID_EXE_

	#include "../Exe/Sys_hdr.h"
	#include "I_file.h"
	#define PRINT ComPrintf

	static CFileStream  * m_pFile=0;

	//Keep track of last world loaded for the exe
	static world_t		* m_pWorld = 0;
	static int			  m_refCount = 0;
	static char			  m_worldName[64];

#elif defined _VVIS
	#include "../vvis/source/std_lib.h"
	#define PRINT v_printf

#elif defined _VLIGHT_
	#include "../vlight/source/std_lib.h"
	#define PRINT v_printf

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

//Ref counts in exe
#ifdef _VOID_EXE_
	if(world == m_pWorld)
	{
		m_refCount--;
		if(m_refCount > 0)
			return;
	}
#endif

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

#ifdef _VOID_EXE_
	m_pWorld = 0;
#endif
}



/**************************************************************
Read a world file from disk
**************************************************************/

static bspf_header_t header;

/*
===========
load_lump
===========
*/
#ifdef _VOID_EXE_
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

/*
======================================
Read the world
======================================
*/
world_t* world_read(char *filename)
{
#ifdef _VOID_EXE_

	// or return cached pointer to client if the local server has alreadly loaded it
	if(!strcmp(filename,m_worldName) && m_pWorld)
	{
		m_refCount++;
		return m_pWorld;
	}
	
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

	world_t *w = new world_t;

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

	//Cache pointer to the last loaded world
	strcpy(m_worldName,filename);
	
	m_pWorld = w;
	m_refCount++;

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


#endif
