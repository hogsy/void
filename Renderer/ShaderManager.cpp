#ifdef RENDERER
	#include "Standard.h"
	#include "Tex_main.h"
	#include "Tex_image.h"
#else
	#include "Com_defs.h"
	#include "Com_vector.h"
	#include "Rast_main.h"
	#include "I_file.h"
	#include "../Devvoid/Std_lib.h"
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
	mLightmaps	 = NULL;
	mWorldBin	 = -1;
	mBaseBin	 = -1;
	mFreePolys	 = NULL;
	mNumCacheAllocs = 0;
	
	mCache = 0;
	for(int i=0; i<MAX_SHADERS;i++)
		mShaders[i] = 0;


	I_FileReader * pFile = CreateFileReader(FILE_BUFFERED);

	if (!pFile->Open("scripts/shaderlist.txt"))
	{
		FError("CShaderManager:: Unable to open shaderlist.txt\n");
		return;
	}

	char token[1024];
	while (1)
	{
		pFile->GetToken(token, true);
		if (!token[0])
			break;
		ParseShaders(token);
	}
	pFile->Destroy();
}

CShaderManager::~CShaderManager()
{
	for (int s=0; s<mNumShaders; s++)
	{
		if(mShaders[s])
			delete mShaders[s];
	}

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
	I_FileReader * pFile = CreateFileReader(FILE_BUFFERED);

	char path[260];

	sprintf(path, "scripts/%s.shader", shaderfile);

	if (!pFile->Open(path))
		return;

	if (mNumShaders == MAX_SHADERS)
		FError("too many shaders - tell js\n");

	char token[1024];

	while (1)
	{
		pFile->GetToken(token, true);
		if (!token[0])
			break;
		mShaders[mNumShaders] = new CShader(token);
		pFile->GetToken(token, true);
		mShaders[mNumShaders]->Parse(pFile);
		mNumShaders++;
	}
	pFile->Destroy();
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
HasLightmap
===========
*/
bool CShaderManager::HasLightmap(char *name)
{
	// find it if we already have it loaded
	for (int s=0; s<mNumShaders; s++)
	{
		if (mShaders[s]->IsShader(name))
			return mShaders[s]->HasLightmap();
	}

	// default shader always has a lightmap
	// (except for models, but that should never be called for this)
	return true;
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
	mCache = new cpoly_t* [world->ntextures];
	if (!mCache) 
	{
		FError("mem for map tex cache");
		return;
	}
	memset(mCache, 0, sizeof(cpoly_t**) * map->ntextures);


	// load lightmaps
	if (map->nlightdefs && map->light_size)
	{

		if (mLightmaps)
		{
			ComPrintf("CShaderManager::LoadShaders - lightmaps already loaded\n");
			delete [] mLightmaps;
		}

		mNumLightmaps = map->nlightdefs;
		mLightmaps = new hTexture[mNumLightmaps];

		TextureData	 tData;
		tData.bClamped = true;
		tData.bMipMaps = true;

		byte *ptr = map->lightdata;
		for (int t=0; t<mNumLightmaps; t++)
			mLightmaps[t] = g_pTex->Load(&ptr, tData);

	}


	// load textures

	//Count number of textures
	if (mWorldBin != -1)
			ComPrintf("CShaderManager::LoadWorld - world bin in use\n");

	mWorldBin = BinInit(world->ntextures);
	for (int t=0; t<world->ntextures; t++)
		LoadShader(mWorldBin, t, map->textures[t]);
}

/*
===========
UnLoadWorld
===========
*/
void CShaderManager::UnLoadWorld(void)
{
	// unload lightmaps
	if (mLightmaps)
		delete [] mLightmaps;
	mLightmaps = NULL;
	mNumLightmaps = 0;


	// unload textures
	if (mWorldBin != -1)
		BinDestroy(mWorldBin);
	mWorldBin = -1;

	// destroy poly cache
	if (mCache)
		delete [] mCache;
	mCache = 0;
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
	// sort by both shader
	int index = world->texdefs[p->texdef].texture;

	// first one for this shader
	if (!mCache[index])
	{
		mCache[index] = p;
		p->next = NULL;
	}

	// sort by lightmap
	else
	{
		for (cpoly_t *prev = mCache[index]; prev->next; prev=prev->next)
		{
			if (prev->next->lightdef < p->lightdef)
				break;
		}
		p->next = prev->next;
		prev->next = p;
	}
}


/*
========
CachePurge
========
*/
void CShaderManager::CachePurge(void)
{
	g_pRast->DepthFunc(VRAST_DEPTH_LEQUAL);
	for (int p=0; p<CACHE_PASS_NUM; p++)
	{
		for (int t=0; t<world->ntextures; t++)
		{
			if (!mCache[t])
				continue;

			CShader *shader = GetShader(mWorldBin, t);
			if (shader->mPass != p)
				continue;

			cpoly_t *poly = mCache[t];

			if (shader->mPass != p)
				continue;

			// sky brushes never depthwrite
			g_pRast->DepthWrite((shader->GetContentFlags() & CONTENTS_SKY) == 0);

			g_pRast->PolyColor(1, 1, 1, 1);
			g_pRast->ShaderSet(shader);
			while (poly)
			{
				if (world->light_size)
					g_pRast->TextureLightDef(&world->lightdefs[poly->lightdef]);
				g_pRast->TextureTexDef(&world->texdefs[poly->texdef]);

				g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
				for (int v = 0; v < poly->num_vertices; v++)
				{
					g_pRast->PolyVertexf(poly->vertices[v]);
				}
				g_pRast->PolyEnd();

				poly = poly->next;
			}

			ReturnPoly(mCache[t]);
			mCache[t] = NULL;
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
	{
		if(mCacheAllocs[i])
			delete [] mCacheAllocs[i];
	}

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