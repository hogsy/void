
#ifdef RENDERER
#include "Standard.h"
#include "Tex_image.h"
#else
#include "Com_defs.h"
#include "Com_Vector.h"
#include "Rast_main.h"
#include "../Devvoid/Std_lib.h"
#include "I_file.h"
#endif

#include "Shader.h"
#include "ShaderManager.h"




CShaderManager	*g_pShaders=0;


const char * BaseTextureList[] =
{
	"textures/base/_ascii",
	"textures/base/conback",
	0
};




/*
===================================================================================================
CShader
===================================================================================================
*/
CShaderManager::CShaderManager()
{
	mNumShaders = 0;
	mNumLightmaps = 0;
	mLightmapBin = -1;
	mWorldBin	 = -1;
	mBaseBin	 = -1;
	mFreePolys	 = NULL;
	mNumCacheAllocs = 0;


	CFileBuffer	 fileReader;
	if (!fileReader.Open("Scripts/shaderlist.txt"))
		return;


	for (int i=0; i<CACHE_PASS_NUM; i++)
		mCache[i] = 0;


	char token[1024];
	while (1)
	{
		fileReader.GetToken(token, true);
		if (!token[0])
			break;

		ParseShaders(token);
	}

	fileReader.Close();
}



CShaderManager::~CShaderManager()
{
	for (int s=0; s<mNumShaders; s++)
		delete mShaders[s];

#ifdef RENDERER
	CacheDestroy();
#endif
}


/*
===========
ParseShaders
===========
*/
void CShaderManager::ParseShaders(const char *shaderfile)
{
	CFileBuffer	 fileReader;
	char path[260];

	sprintf(path, "scripts/%s.shader", shaderfile);

	if (!fileReader.Open(path))
		return;

	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");


	char token[1024];

	while (1)
	{
		fileReader.GetToken(token, true);
		if (!token[0])
			break;

		mShaders[mNumShaders] = new CShader(token);
		fileReader.GetToken(token, true);
		mShaders[mNumShaders]->Parse(&fileReader);
		mNumShaders++;
	}

	fileReader.Close();
}


/*
===========
LoadShader
===========
*/
void CShaderManager::LoadShader(int bin, int index, const char *name)
{
	// find it if we already have it loaded
	for (int s=0; s<mNumShaders; s++)
	{
		if (mShaders[s]->IsShader(name))
		{
			mShaders[s]->AddRef();
			mBins[bin].indices[index] = s;
			return;
		}
	}

	// create the default
	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");

	mShaders[mNumShaders] = new CShader(name);
	mShaders[mNumShaders]->Default();
	mShaders[mNumShaders]->AddRef();
	mBins[bin].indices[index] = mNumShaders++;
}


/*
===========
GetContentFlags
===========
*/
unsigned int CShaderManager::GetContentFlags(char *name)
{
	// find it if we already have it loaded
	for (int s=0; s<mNumShaders; s++)
	{
		if (mShaders[s]->IsShader(name))
			return mShaders[s]->GetContentFlags();
	}

	// create the default
	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");

	mShaders[mNumShaders] = new CShader(name);
	mShaders[mNumShaders]->Default();
	mShaders[mNumShaders]->AddRef();
	return mShaders[mNumShaders]->GetContentFlags();
}


/*
===========
GetSurfaceFlags
===========
*/
unsigned int CShaderManager::GetSurfaceFlags(char *name)
{
	// find it if we already have it loaded
	for (int s=0; s<mNumShaders; s++)
	{
		if (mShaders[s]->IsShader(name))
			return mShaders[s]->GetSurfaceFlags();
	}

	// create the default
	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");

	mShaders[mNumShaders] = new CShader(name);
	mShaders[mNumShaders]->Default();
	mShaders[mNumShaders]->AddRef();
	return mShaders[mNumShaders]->GetSurfaceFlags();
}


/*
===========
GetDims
===========
*/
void CShaderManager::GetDims(char *name, int &width, int &height)
{
	// find it if we already have it loaded
	for (int s=0; s<mNumShaders; s++)
	{
		if (mShaders[s]->IsShader(name))
		{
			mShaders[s]->GetDims(width, height);
			return;
		}
	}

	// create the default
	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");

	mShaders[mNumShaders] = new CShader(name);
	mShaders[mNumShaders]->Default();
	mShaders[mNumShaders]->AddRef();
	mShaders[mNumShaders]->GetDims(width, height);
}


/*
===========
LoadWorld
===========
*/
#ifdef RENDERER
void CShaderManager::LoadWorld(CWorld *map)
{
	// create cache
	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		mCache[i] = new cpoly_t* [world->ntextures];
		if (!mCache[i]) 
		{
			FError("mem for map tex cache");
			return;
		}
		memset(mCache[i], 0, sizeof(cpoly_t**) * map->ntextures);
	}


	// load lightmaps
	if (map->nlightdefs && map->light_size)
	{

		if (mLightmapBin != -1)
			ComPrintf("CShaderManager::LoadShaders - lightmaps already loaded\n");

		mNumLightmaps = map->nlightdefs;
		mLightmapBin = g_pRast->TextureBinInit(mNumLightmaps);

		TextureData	 tData;
		tData.bClamped = true;
		tData.bMipMaps = true;

		byte *ptr = map->lightdata;
		for (int t=0; t<mNumLightmaps; t++)
		{
			CImageReader::GetReader().ReadLightMap(&ptr, tData);
			g_pRast->TextureLoad(mLightmapBin, t, tData);
		}
	}


	// load textures

	//Count number of textures
	if (mWorldBin != -1)
			ComPrintf("CShaderManager::LoadWorld - world bin in use\n");

	mWorldBin = BinInit(world->ntextures);
//	char texname[260];
	for (int t=0; t<world->ntextures; t++)
	{
//		strcpy(texname, "textures");
//		strcat(texname, "/");
//		strcat(texname, map->textures[t]);
		LoadShader(mWorldBin, t, map->textures[t]);
	}
}

/*
===========
UnLoadWorld
===========
*/
void CShaderManager::UnLoadWorld(void)
{
	// unload lightmaps
	if (mLightmapBin != -1)
		g_pRast->TextureBinDestroy(mLightmapBin);
	mLightmapBin = -1;
	mNumLightmaps = 0;


	// unload textures
	if (mWorldBin != -1)
		BinDestroy(mWorldBin);
	mWorldBin = -1;

	// destroy poly cache
	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		if (mCache[i])
			delete [] mCache[i];
		mCache[i] = 0;
	}
}


/*
===========
LoadBase
===========
*/
void CShaderManager::LoadBase(void)
{
	if (mBaseBin != -1)
			ComPrintf("CShaderManager::LoadBase - base bin in use\n");

	//Get count of base textures
	for(int count=0; BaseTextureList[count] != 0; count++);
	mBaseBin = BinInit(count);

	for (int t=0; t<count; t++)
		LoadShader(mBaseBin, t, BaseTextureList[t]);
}


/*
===========
UnLoadBase
===========
*/
void CShaderManager::UnLoadBase(void)
{
	// unload textures
	if (mBaseBin != -1)
		BinDestroy(mBaseBin);
	mBaseBin = -1;
}
#endif


/*
========
BinInit
========
*/
int CShaderManager::BinInit(int num)
{
	for (int i=0; i<MAX_SHADER_BINS; i++)
	{
		if (mBins[i].num == -1)
		{
			mBins[i].num = num;
			mBins[i].indices = new hShader[num];
			if (!mBins[i].indices)
				FError("not enough mem for shader bin indices");

			return i;
		}
	}

	Error("out of shader bins\n");
	return -1;
}


/*
========
BinDestroy
========
*/
void CShaderManager::BinDestroy(int bin)
{
	if ((bin < 0) || (bin > MAX_SHADER_BINS) || (mBins[bin].num == -1))
	{
		Error("destroying non-existant texture bin!");
		return;
	}

	// release all shaders
	for (int s=0; s<mBins[bin].num; s++)
		mShaders[mBins[bin].indices[s]]->Release();

	delete mBins[bin].indices;
	mBins[bin].indices = NULL;
	mBins[bin].num = -1;
}



/*
========
CacheAdd
========
*/
#ifdef RENDERER
void CShaderManager::CacheAdd(cpoly_t *p)
{
	int index = world->texdefs[p->texdef].texture;
	CShader *s = GetShader(mWorldBin, index);

	p->next = mCache[s->mPass][index];
	mCache[s->mPass][index] = p;
}


/*
========
CachePurge
========
*/
void CShaderManager::CachePurge(void)
{
	for (int p=0; p<CACHE_PASS_NUM; p++)
	{
		for (int t=0; t<world->ntextures; t++)
		{
			cpoly_t *poly = mCache[p][t];

			if (poly)
			{
				CShader *shader = GetShader(mWorldBin, t);
				if (shader->mPass != p)
					continue;

				g_pRast->ShaderSet(shader);
				while (poly)
				{
					g_pRast->TextureTexDef(&world->texdefs[poly->texdef]);
					g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
					for (int v = 0; v < poly->num_vertices; v++)
					{
						g_pRast->PolyVertexf(poly->vertices[v]);
					}
					g_pRast->PolyEnd();

					poly = poly->next;
				}

				ReturnPoly(mCache[p][t]);
				mCache[p][t] = NULL;
			}
		}
	}
}







// poly memory management funcs
cpoly_t* CShaderManager::PolyAlloc(void)
{
	if (mNumCacheAllocs == POLY_CACHE_ALLOCS)
		FError("too many cache alloc's!  ** tell Ripper **");

	// free_polys must be NULL
	mFreePolys = new cpoly_t[POLY_CACHE_POLYS];
	if (!mFreePolys)
		FError("mem for poly cache - %d allocated", POLY_CACHE_POLYS*mNumCacheAllocs);

	// set the linkage
	for (int i=0; i<POLY_CACHE_POLYS-1; i++)
		mFreePolys[i].next = &mFreePolys[i+1];
	mFreePolys[i].next = NULL;

	mCacheAllocs[mNumCacheAllocs] = mFreePolys;
	mNumCacheAllocs++;
	cpoly_t *ret = mFreePolys;
	mFreePolys = mFreePolys->next;

	return ret;
}


void CShaderManager::CacheDestroy(void)
{
	for (int i=0; i<mNumCacheAllocs; i++)
		delete [] mCacheAllocs[i];

	mNumCacheAllocs = 0;
	mFreePolys = NULL;
}


cpoly_t* CShaderManager::GetPoly(void)
{
	if (!mFreePolys)
		return PolyAlloc();

	cpoly_t *ret = mFreePolys;
	mFreePolys = mFreePolys->next;
	return ret;
}


void CShaderManager::ReturnPoly(cpoly_t *p)
{
	if (!p)
		return;

	cpoly_t *tmp = p;
	while (p->next)
		p = p->next;
	p->next = mFreePolys;
	mFreePolys = tmp;
}

#endif