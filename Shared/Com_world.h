#ifndef COM_VOID_WORLD_H
#define COM_VOID_WORLD_H

#include "Com_defs.h"
#include "Com_vector.h"
#include "Com_trace.h"
#include "Bsp_file.h"

class CFileStream;

/*
============================================================================
The World class
============================================================================
*/
class CWorld : public I_World
{
public:

	CWorld();
	~CWorld();

	//Interface Implementation
	int  PointContents(const vector_t &v);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end);
	void Trace(TraceInfo &traceInfo, const vector_t &start, const vector_t &end, 
									 const vector_t &mins, const vector_t &maxs);

	void PrintInfo() const;
	bool WriteToFile(const char * szPath) const;

	//Util Key Access funcs
	int	  GetKeyInt(int ent, const char * keyName) const;
	void  GetKeyVector(int ent, const char * keyName, vector_t &vec) const;
	float GetKeyFloat(int ent, const char * keyName) const;
	const char* GetKeyString(int ent, const char * keyName) const;

	void DestroyLightData(void);
	void SetLightData(unsigned char *data, int len, bspf_texdef_t *defs, int numdefs);

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

	static int  LoadLump(CFileStream &file, int l, void **data);
	static void AddLump(FILE *f, bspf_header_t &header, int l, void *data, int size);


	//Cached world data to prevent 
	//the world from being loaded twice.
	static bspf_header_t m_worldHeader;
	static CWorld * m_pWorld;
	static char     m_szFileName[COM_MAXPATH];
	static int  	m_refCount;
};

#endif