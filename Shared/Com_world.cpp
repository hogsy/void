#include "Com_world.h"
#include "I_file.h"

//Static data
CWorld * CWorld::m_pWorld=0;
bspf_header_t CWorld::m_worldHeader;
char CWorld::m_szFileName[COM_MAXPATH];
int	 CWorld::m_refCount=0;

const float ON_EPSILON = 0.03125f;

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

/*
======================================
Return integer which is flagged with the 
type of contents the given point is in
======================================
*/
int CWorld::PointContents(const vector_t &v)
{
	int n=0;
	float d = 0.0f;
	
	while(1)
	{
		// test to this nodes plane
		d = Void3d::DotProduct(planes[nodes[n].plane].norm, v) - planes[nodes[n].plane].d;
		if (d>=0)
			n = nodes[n].children[0];
		else
			n = nodes[n].children[1];
		// if we found a leaf, it's what we want
		if (n<=0)
			return leafs[-n].contents;
	}
	return 0;
}

/*
======================================
Collide a ray with everything in the world
======================================
*/
plane_t * CWorld::Ray(int node, const vector_t &start, const vector_t &end, 
					  float *endfrac, plane_t *lastplane)
{
	plane_t *hitplane;
	float dstart, dend;
	plane_t *plane;
	int n, nnode;

	if(Void3d::DotProduct(start, planes[nodes[node].plane].norm) - planes[nodes[node].plane].d >= 0)
	{
		n = 0;
		plane = &planes[nodes[node].plane];
	}
	else
	{
		n = 1;
		plane = &planes[nodes[node].plane^1];
	}

	dstart = Void3d::DotProduct(start, plane->norm) - plane->d;
	dend   = Void3d::DotProduct(end  , plane->norm) - plane->d;

	float frac = (dstart)/(dstart-dend);

	// we're going to hit a plane that separates 2 child nodes
	if (0<frac && frac<1)
	{
		frac = (dstart- ON_EPSILON)/(dstart-dend);
		if (frac < 0)	frac = 0;

		// collide through near node
		nnode = nodes[node].children[n];
		// node
		if (nnode>0)
		{
			hitplane = Ray(nnode, start, end, endfrac, lastplane);
			// if we're not at the intersection, we hit something - return
			if (hitplane)
				return hitplane;
		}
		// leaf
		else
		{
			// stop at beginning if we're in a solid node
			if (leafs[-nnode].contents & CONTENTS_SOLID)
				return lastplane;
			// else we can move all the way through it - FIXME add other contents tests??
		}

		// we made it through near side - collide with far side
		nnode = nodes[node].children[1-n];

		// node
		if (nnode>0)
		{
			*endfrac = frac;
			hitplane = Ray(nnode, start, end, endfrac, plane);
			if (hitplane)
				return hitplane;
		}

		// leaf
		else
		{
			if (leafs[-nnode].contents & CONTENTS_SOLID)
			{
				*endfrac = frac;
				return plane;
			}
		}

		// if we're here, we didn't hit anything
		*endfrac = 1;
		return 0;
	}

	// else we're entirely in the near node
	nnode = nodes[node].children[n];

	// node
	if (nnode>0)
		return Ray(nnode, start, end, endfrac, lastplane);

	// leaf
	if (leafs[-nnode].contents & CONTENTS_SOLID)
		return lastplane;
	*endfrac = 1;
	return 0;
}


/*
======================================
collide a bounding box with everything in the world
======================================
*/
void CWorld::Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end, 
								 const vector_t &mins, const vector_t &maxs)
{
	traceInfo.fraction = 1;
	traceInfo.plane = 0;
	VectorCopy(end, traceInfo.endpos);

	// find the length/direction of a full trace
	vector_t dir;
	VectorSub(end, start, dir);

	float	 frac;
	plane_t* hitplane;
	vector_t bend;	// where this box corner should end

	vector_t bbox[8];
	bbox[0].x = mins.x;	bbox[0].y = mins.y;	bbox[0].z = mins.z;
	bbox[1].x = mins.x;	bbox[1].y = mins.y;	bbox[1].z = maxs.z;
	bbox[2].x = mins.x;	bbox[2].y = maxs.y;	bbox[2].z = mins.z;
	bbox[3].x = mins.x;	bbox[3].y = maxs.y;	bbox[3].z = maxs.z;
	bbox[4].x = maxs.x;	bbox[4].y = mins.y;	bbox[4].z = mins.z;
	bbox[5].x = maxs.x;	bbox[5].y = mins.y;	bbox[5].z = maxs.z;
	bbox[6].x = maxs.x;	bbox[6].y = maxs.y;	bbox[6].z = mins.z;
	bbox[7].x = maxs.x;	bbox[7].y = maxs.y;	bbox[7].z = maxs.z;

	// test each corner of the box
	// FIXME - leave out one of the corners based on direction??
	for (int i=0; i<8; i++)
	{
		VectorAdd(bbox[i], start, bbox[i]);
		VectorAdd(bbox[i], dir, bend);

		frac = 0;
		hitplane = Ray(0, bbox[i], bend, &frac, 0);

		// find the length this corner went
		if ((traceInfo.fraction > frac) && hitplane)
		{
			traceInfo.fraction = frac;
			traceInfo.plane = hitplane;

			if (traceInfo.fraction==0)
				break;
		}
	}
	VectorMA(&start, traceInfo.fraction, &dir, &traceInfo.endpos);
}

void CWorld::Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end)
{
	traceInfo.fraction = 1;
	traceInfo.plane = 0;
	VectorCopy(end, traceInfo.endpos);

	// find the length/direction of a full trace
	vector_t dir;
	VectorSub(end, start, dir);

	traceInfo.fraction = 0;
	traceInfo.plane = Ray(0, start, end, &traceInfo.fraction, 0);
	VectorMA(&start, traceInfo.fraction, &dir, &traceInfo.endpos);
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
	if(pWorld == m_pWorld)
	{
		m_refCount--;
		if(m_refCount > 0)
			return;
		m_pWorld = 0;
	}
	delete pWorld;
}



//Put this in another appropriately named 
//static func, if the VLight app needs to load the world
#if 0

	FILE * bsp_file = fopen(szFileName, "rb");
	if (!bsp_file)
	{
		ComPrintf("couldn't open %s\n", szFileName);
		return NULL;
	}

	fread(&m_worldHeader, 1, sizeof(bspf_header_t), bsp_file);
	if (m_worldHeader.id != BSP_FILE_ID)
	{
		ComPrintf("%s not a void bsp file!", szFileName);
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




































