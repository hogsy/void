
#include "ShaderManager.h"
#include "Tex_image.h"

CShaderManager	*g_pShaders=0;
extern const char * BaseTextureList[];



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

	CFileBuffer	 fileReader;
	if (!fileReader.Open("Shaders/shaderlist.txt"))
		return;


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

	sprintf(path, "Shaders/%s.txt", shaderfile);

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
LoadWorld
===========
*/
void CShaderManager::LoadWorld(CWorld *map)
{

	// load lightmaps
	if (map->nlightdefs && map->light_size)
	{

		if (mLightmapBin != -1)
			ComPrintf("CShaderManager::LoadShaders - lightmaps already loaded\n");

		mNumLightmaps = map->nlightdefs;
		mLightmapBin = g_pRast->TextureBinInit(mNumLightmaps);

		CImageReader texReader;

		unsigned char *ptr = map->lightdata;
		for (int t=0; t<mNumLightmaps; t++)
		{
			texReader.ReadLightMap(&ptr);

			// create all mipmaps
			tex_load_t tdata;
			tdata.format = texReader.GetFormat();
			tdata.height = texReader.GetHeight();
			tdata.width  = texReader.GetWidth();
			tdata.mipmaps= texReader.GetNumMips();
			tdata.mipdata= texReader.GetMipData();
			tdata.mipmap = true;
			tdata.clamp  = true;

			int mipcount = tdata.mipmaps - 1;
			while (mipcount > 0)
			{
				texReader.ImageReduce(mipcount);
				mipcount--;
			}

			g_pRast->TextureLoad(mLightmapBin, t, &tdata);

		}
	}


	// load textures

	//Count number of textures
	int n=0;
	while (map->textures[n][0] != '\0')
		n++;

	if (mWorldBin != -1)
			ComPrintf("CShaderManager::LoadWorld - world bin in use\n");

	mWorldBin = BinInit(n);
	for (int t=0; t<n; t++)
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
	if (mLightmapBin != -1)
		g_pRast->TextureBinDestroy(mLightmapBin);
	mLightmapBin = -1;
	mNumLightmaps = 0;


	// unload textures
	if (mWorldBin != -1)
		BinDestroy(mWorldBin);
	mWorldBin = -1;
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









