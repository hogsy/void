#ifdef _VOID_EXE
	//Exe
	#include "..\Exe\Sys_hdr.h"
#else
	//Tools
	#include "..\Devvoid\Std_lib.h"	
#endif

#include "Com_world.h"

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
	edges =0;
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
	textures=0;
	keys=0;
	leafvis=0;
	
	lightdata=0;
	lightdefs=0;
	light_size =0;
	nlightdefs = 0;

	nverts = niverts = nplanes =  nentities =  nnodes = nleafs = 0;
	nbrushes = nsides = nedges = ntexdefs = ntextures =0;
	nkeys = leafvis_size = 0;
}

/*
======================================
Destructor
======================================
*/
CWorld::~CWorld()
{
	if(edges)
		operator delete(edges);
	if (textures)
		operator delete(textures);
	if (texdefs)
		operator delete(texdefs);
	if (brushes)
		operator delete(brushes);
	if (iverts)
		operator delete(iverts);
	if (nodes)
		operator delete(nodes);
	if (leafs)
		operator delete(leafs);
	if (planes)
		operator delete(planes);
	if (sides)
		operator delete(sides);
	if (verts)
		operator delete(verts);
	if (entities)
		operator delete(entities);
	if (keys)
		operator delete(keys);
	if (leafvis)
		operator delete(leafvis);
	if(lightdata)
		operator delete(lightdata);
	if(lightdefs)
		operator delete(lightdefs);
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

void CWorld::AddLump(FILE *f, bspf_header_t &header, int l, void *data, int size) 
{
	header.lumps[l].length = size;
	header.lumps[l].offset = ftell(f);
	fwrite(data, 1, size, f);
}

bool CWorld::WriteToFile(const char * szPath) const
{
	char filePath[COM_MAXPATH];

	sprintf(filePath,"%s/%s",szPath,m_szFileName);

	FILE *fout = fopen(filePath, "wb");
	if (!fout)
		return false;

	bspf_header_t header;
	fwrite(&header, 1, sizeof(bspf_header_t), fout);

	memset(&header, 0, sizeof(bspf_header_t));
	header.id = BSP_FILE_ID;
	header.version = BSP_VERSION;

	AddLump(fout, header, LUMP_NODES,		nodes,			nnodes		*sizeof(bspf_node_t));
	AddLump(fout, header, LUMP_LEAFS,		leafs,			nleafs		*sizeof(bspf_leaf_t));
	AddLump(fout, header, LUMP_PLANES,		planes,			nplanes		*sizeof(plane_t));
	AddLump(fout, header, LUMP_SIDES,		sides,			nsides		*sizeof(bspf_side_t));
	AddLump(fout, header, LUMP_VERTICES,	verts,			nverts		*sizeof(vector_t));
	AddLump(fout, header, LUMP_VERT_INDICES,iverts,			niverts		*sizeof(int));
	AddLump(fout, header, LUMP_BRUSHES,		brushes,		nbrushes	*sizeof(bspf_brush_t));
	AddLump(fout, header, LUMP_TEXDEF,		texdefs,		ntexdefs	*sizeof(bspf_texdef_t));
	AddLump(fout, header, LUMP_TEXNAMES,	textures,		ntextures	*sizeof(texname_t));
	AddLump(fout, header, LUMP_EDGES,		edges,			nedges		*sizeof(bspf_edge_t));
	AddLump(fout, header, LUMP_ENTITIES,	entities,		nentities	*sizeof(bspf_entity_t));
	AddLump(fout, header, LUMP_KEYS,		keys,			nkeys		*sizeof(key_t));
	AddLump(fout, header, LUMP_LIGHTDEF,	lightdefs,		nlightdefs	*sizeof(bspf_texdef_t));
	AddLump(fout, header, LUMP_LEAF_VIS,	leafvis,		leafvis_size*nleafs);
	AddLump(fout, header, LUMP_LIGHTMAP,	lightdata,		light_size);

	// rewrite the header with offset info
	fseek(fout, 0, SEEK_SET);
	fwrite(&header, 1, sizeof(bspf_header_t), fout);
	fclose(fout);
	return true;
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
		vec.Set(0, 0, 0);
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
		d = DotProduct(planes[nodes[n].plane].norm, v) - planes[nodes[n].plane].d;
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

	if(DotProduct(start, planes[nodes[node].plane].norm) - planes[nodes[node].plane].d >= 0)
	{
		n = 0;
		plane = &planes[nodes[node].plane];
	}
	else
	{
		n = 1;
		plane = &planes[nodes[node].plane^1];
	}

	dstart = DotProduct(start, plane->norm) - plane->d;
	dend   = DotProduct(end  , plane->norm) - plane->d;

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
	traceInfo.endpos = end;

	// find the length/direction of a full trace
	vector_t dir = end - start;

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
		bbox[i] += start;
		bend = bbox[i] + dir;

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
	traceInfo.endpos.VectorMA(start, traceInfo.fraction, dir);
}

void CWorld::Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end)
{
	traceInfo.fraction = 1;
	traceInfo.plane = 0;
	traceInfo.endpos = end;

	// find the length/direction of a full trace
	vector_t dir = end - start;

	traceInfo.fraction = 0;
	traceInfo.plane = Ray(0, start, end, &traceInfo.fraction, 0);
	traceInfo.endpos.VectorMA(start, traceInfo.fraction, dir);
}

//======================================================================================
//======================================================================================

//int CWorld::LoadLump(CFileStream &file, int l, void **data)
int   CWorld::LoadLump(I_FileReader  * pFile, int l, void **data)
{
	*data = (void*) new byte[m_worldHeader.lumps[l].length+1];
	memset(*data, 0, m_worldHeader.lumps[l].length+1);

	pFile->Seek(m_worldHeader.lumps[l].offset, SEEK_SET);
	pFile->Read(*data, m_worldHeader.lumps[l].length, 1);

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
	
#ifdef _VOID_EXE
	I_FileReader * pFile = System::CreateFileReader(FILE_BUFFERED);
#else
	I_FileReader * pFile = CreateFileReader(FILE_BUFFERED);
#endif

	if(!pFile->Open(szFileName))
	{
		ComPrintf("CWorld::CreateWorld: Could not open %s\n",szFileName);
		pFile->Destroy();
		return 0;
	}

	pFile->Read(&m_worldHeader,sizeof(bspf_header_t),1);
	if (m_worldHeader.id != BSP_FILE_ID)
	{
		ComPrintf("CWorld::CreateWorld: %s not a Void World file!", szFileName);
		pFile->Destroy();
		return 0;
	}

	if (m_worldHeader.version != BSP_VERSION)
	{
		ComPrintf("CWorld::CreateWorld: World Version %d, need %d for %s\n", 
							m_worldHeader.version, BSP_VERSION, szFileName);
		pFile->Destroy();
		return 0;
	}

	m_pWorld = new CWorld;

	// read in all the lumps
	m_pWorld->nnodes		= LoadLump(pFile,LUMP_NODES, (void**)&m_pWorld->nodes)	/ sizeof(bspf_node_t);
	m_pWorld->nleafs		= LoadLump(pFile,LUMP_LEAFS, (void**)&m_pWorld->leafs)	/ sizeof(bspf_leaf_t);
	m_pWorld->nplanes		= LoadLump(pFile,LUMP_PLANES,(void**)&m_pWorld->planes) / sizeof(plane_t);
	m_pWorld->nsides		= LoadLump(pFile,LUMP_SIDES,(void**)&m_pWorld->sides)	/ sizeof(bspf_side_t);
	m_pWorld->nverts		= LoadLump(pFile,LUMP_VERTICES,(void**)&m_pWorld->verts) / sizeof(vector_t);
	m_pWorld->niverts		= LoadLump(pFile,LUMP_VERT_INDICES,(void**)&m_pWorld->iverts) / sizeof(int);
	m_pWorld->nbrushes		= LoadLump(pFile,LUMP_BRUSHES,	(void**)&m_pWorld->brushes) / sizeof(bspf_brush_t);
	m_pWorld->ntexdefs		= LoadLump(pFile,LUMP_TEXDEF,	(void**)&m_pWorld->texdefs) / sizeof(bspf_texdef_t);
	m_pWorld->ntextures		= LoadLump(pFile,LUMP_TEXNAMES,(void**)&m_pWorld->textures)/ sizeof(texname_t);
	m_pWorld->nedges		= LoadLump(pFile,LUMP_EDGES,	(void**)&m_pWorld->edges)	 / sizeof(bspf_edge_t);
	m_pWorld->nentities		= LoadLump(pFile,LUMP_ENTITIES,(void**)&m_pWorld->entities)/ sizeof(bspf_entity_t);
	m_pWorld->nkeys			= LoadLump(pFile,LUMP_KEYS,	(void**)&m_pWorld->keys)	 / sizeof(key_t);
	m_pWorld->leafvis_size	= LoadLump(pFile,LUMP_LEAF_VIS,(void**)&m_pWorld->leafvis) / m_pWorld->nleafs;
	m_pWorld->light_size	= LoadLump(pFile,LUMP_LIGHTMAP,(void**)&m_pWorld->lightdata);
	m_pWorld->nlightdefs	= LoadLump(pFile,LUMP_LIGHTDEF,(void**)&m_pWorld->lightdefs)/sizeof(bspf_texdef_t);

	//Cache pointer to the last loaded world
	strcpy(m_szFileName,szFileName);
	m_refCount++;

	pFile->Destroy();
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
		delete m_pWorld;
		m_pWorld = 0;
		pWorld = 0;
	}
}

void CWorld::DestroyVisData(void)
{
	if (leafvis)
	{
		operator delete(leafvis);
		leafvis = NULL;
		leafvis_size = 0;
	}
}

void CWorld::DestroyLightData(void)
{
	if (lightdata)
	{
		operator delete(lightdata);
		lightdata = NULL;
		light_size = 0;
	}

	if (lightdefs)
	{
		operator delete(lightdefs);
		lightdefs = NULL;
		nlightdefs = 0;
	}

}

void CWorld::SetLightData(unsigned char *data, int len, bspf_texdef_t *defs, int numdefs)
{
	light_size = len;
	lightdata = data;
	lightdefs = defs;
	nlightdefs = numdefs;
}
