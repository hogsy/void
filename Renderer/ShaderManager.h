#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H


#define MAX_SHADERS		1024
#define MAX_SHADER_BINS	1024
#define CACHE_PASS_NUM			3

typedef int hShader;
class CShader;


// !!! only map polys are cached !!!
#ifdef _DEBUG
#define POLY_CACHE_POLYS	(1024/512)
#define POLY_CACHE_ALLOCS	(32*512)
#else
#define POLY_CACHE_POLYS	1024
#define POLY_CACHE_ALLOCS	32
#endif



struct cpoly_t
{
	int			num_vertices;
	vector_t	vertices[32];

	int			texdef;
	int			lightdef;

	bool		forcez;	// forces a ztest

	cpoly_t *next;
};



class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();

#ifdef RENDERER
	void LoadWorld(CWorld *map);	// loads shaders needed for world
									// also loads the lightmaps
	void UnLoadWorld(void);

	void LoadBase(void);
	void UnLoadBase(void);
#endif

	void LoadShader(int bin, int index, const char *name);	// loads a specific shader, creates the default if it isn't found
	void GetDims(char *name, int &width, int &height);	// get width & height of first non-lightmap layer of shader

	
	void CacheAdd(cpoly_t *p);
	void CachePurge(void);
	cpoly_t*	GetPoly(void);
	void		ReturnPoly(cpoly_t *p);


	CShader *GetShader(int bin, int index)	{	return (mShaders[mBins[bin].indices[index]]);	}

	CShader *mShaders[MAX_SHADERS];

	int  BinInit(int num);
	void BinDestroy(int bin);

	int mNumLightmaps;
	int mLightmapBin;

	int	mWorldBin;	// shader bin that holds the world shaders
	int	mBaseBin;	// shader bin that holds the base shaders

private:

	struct shader_bin_t
	{
		shader_bin_t()
		{
			num = -1;
			indices = NULL;
		}

		hShader *indices;
		int		num;
	};

	shader_bin_t mBins[MAX_TEXTURE_BINS];
	cpoly_t		**mCache[CACHE_PASS_NUM];	// world poly cache

	cpoly_t*	PolyAlloc(void);
	void		CacheDestroy(void);

	cpoly_t		*mFreePolys;
	cpoly_t		*mCacheAllocs[POLY_CACHE_ALLOCS];
	int			mNumCacheAllocs;




	int mNumShaders;
	void ParseShaders(const char *shaderfile);	// parse all shaders into memory
};


extern	CShaderManager	*g_pShaders;

#endif

