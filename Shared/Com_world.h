#ifndef COM_VOID_WORLD_H
#define COM_VOID_WORLD_H

#include "Com_defs.h"
#include "Com_vector.h"
#include "Bsp_file.h"

class CFileStream;

/*
============================================================================
keeps info about trace operations within the world
============================================================================
*/
struct TraceInfo
{
	TraceInfo() { fraction = 0.0f; plane = 0; }
	~TraceInfo() { plane = 0; }

	vector_t	endpos;		// where the trace ended
	float		fraction;	// fraction of trace completed
	plane_t	  * plane;
};

/*
============================================================================
The World class
============================================================================
*/
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

	int  PointContents(const vector_t &v);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end, 
									 const vector_t &mins, const vector_t &maxs);
	
	//the world should be loaded and destroyed using these
	static CWorld * CreateWorld(const char * szFileName);
	static void DestroyWorld(CWorld * pworld);

	//World Data

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

	plane_t * Ray(int node, const vector_t &start, const vector_t &end, 
				  float *endfrac, plane_t *lastplane);

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