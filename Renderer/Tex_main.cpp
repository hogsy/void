#include "Standard.h"
#include "Tex_main.h"
#include "Tex_image.h"



//tex_t			* tex=0;
CTextureManager * g_pTex=0;
extern	CVar *	g_p32BitTextures;

/*
==========================================
Constructor
==========================================
*/

CTextureManager::CTextureManager()
{
	strcpy(m_textureDir,"textures");
	
	m_loaded = NO_TEXTURES;
//	m_numBaseTextures = 0;
//	m_numWorldTextures = 0;
}

/*
==========================================
Destructor
==========================================
*/
CTextureManager::~CTextureManager()
{	CImageReader::GetReader().FreeMipData();
}


/*
==========================================
Initialize
Load base game textures
==========================================
*/
bool CTextureManager::Init()
{
/*
	//allocate all mem
	tex = new tex_t();
	if (tex == NULL) 
	{
		FError("CTextureManager::Init:No mem for tex struct");
		return false;
	}

	//Get count of base textures
	for(int count=0; BaseTextureList[count] != 0; count++)
		m_numBaseTextures =  count +1;

	//Alloc space for base textures
//	tex->bin_base = g_pRast->TextureBinInit(m_numBaseTextures);

	ComPrintf("CTextureManager::Init:Creating base textures");

	TextureData tData;
	// load base textures
	for(count=0;count<m_numBaseTextures;count++)
	{
		LoadTexture(BaseTextureList[count], tData);

		tData.bClamped = true;
		tData.bMipMaps = false;

		g_pRast->TextureLoad(tex->bin_base, count, tData);
	}
	m_loaded = BASE_TEXTURES;
	
	CImageReader::GetReader().FreeMipData();
*/
	return true;
}


/*
==========================================
Shutdown
Release all textures
==========================================
*/
bool CTextureManager::Shutdown()
{
/*	if (!tex)
		return false;

	UnloadWorldTextures();

	ComPrintf("CTextureManager::Shutdown:Destroying base textures :");

//	g_pRast->TextureBinDestroy(tex->bin_base);

	delete tex;	
	tex = 0;

	m_loaded = NO_TEXTURES;

	ComPrintf("OK\n");
*/
	return true;
}

/*
==========================================
LoadWorld Textures
==========================================
*/
bool CTextureManager::LoadWorldTextures(CWorld * pWorld)
{
/*
	if(m_loaded != BASE_TEXTURES)
		return false;

	if (!tex)
		return false;

	if (!pWorld)
		return false;

	uint   mipcount = 0,
		   t=0,m=0;

	m_numWorldTextures = 0;

*/	//Count number of textures
//	while (pWorld->textures[m_numWorldTextures][0] != '\0')
//		m_numWorldTextures++;


//	tex->bin_world = g_pRast->TextureBinInit(m_numWorldTextures);
/*
	// allocate the poly cache
	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		tex->polycaches[i] = new cpoly_t* [world->ntextures];
		if (!tex->polycaches[i]) 
		{
			FError("mem for map tex cache");
			return false;
		}
		memset(tex->polycaches[i], 0, sizeof(cpoly_t**) * world->ntextures);
	}

	// allocate dim's
	tex->dims = new dimension_t[m_numWorldTextures];
	if (!tex->dims) 
	{
		FError("mem for map tex dims");
		return false;
	}
*/
/*
	TextureData tData;
	tData.bMipMaps = true;
	tData.bClamped  = false;

	for (t=0; t<m_numWorldTextures; t++)
	{
		LoadTexture(pWorld->textures[t], tData);

		tex->dims[t][0] = tData.width;
		tex->dims[t][1] = tData.height;
*/
		// create all mipmaps
/*		int mipcount = tData.numMipMaps - 1;
		while (mipcount > 0)
		{
			CImageReader::GetReader().ImageReduce(mipcount);
			mipcount--;
		}
*/
//		g_pRast->TextureLoad(tex->bin_world, t, tData);
//	}
/*
// FIXME - temp hack to get lightmapping working
	if (!pWorld->nlightdefs || !pWorld->light_size)
	{
		m_loaded = ALL_TEXTURES;
		CImageReader::GetReader().FreeMipData();
		return true;
	}

	tex->bin_light = g_pRast->TextureBinInit(pWorld->nlightdefs);	// each lightdef has a unique lightmap

	byte *ptr = pWorld->lightdata;
	for (t = 0; t < g_pRast->TextureCount(tex->bin_light); t++)
	{

		CImageReader::GetReader().ReadLightMap(&ptr, tData);

		tData.bMipMaps = true;
		tData.bClamped = true;

		int mipcount = tData.numMipMaps - 1;
		while (mipcount > 0)
		{
			CImageReader::GetReader().ImageReduce(mipcount);
			mipcount--;
		}

		g_pRast->TextureLoad(tex->bin_light, t, tData);
	}

	CImageReader::GetReader().FreeMipData();
*/
	m_loaded = ALL_TEXTURES;
	return true;
}


/*
==========================================
UnloadWorld Textures
==========================================
*/
bool CTextureManager::UnloadWorldTextures()
{
/*
	if(m_loaded != ALL_TEXTURES)
		return true;

	if (!tex)
		return false;

	ComPrintf("Destroying map textures: ");

	g_pRast->TextureBinDestroy(tex->bin_world);
	tex->bin_world = -1;
*/
/*	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		delete [] tex->polycaches[i];
		tex->polycaches[i] = 0;
	}

	// free lightmaps
	if (tex->bin_light != -1)
	{
		g_pRast->TextureBinDestroy(tex->bin_light);
		tex->bin_light = -1;
	}

	if(tex->dims)
	{
		delete [] tex->dims;
		tex->dims = 0;
	}
*/
	ComPrintf("OK\n");

	m_loaded = BASE_TEXTURES;
	return true;
}

/*
==========================================
Load a map texture
==========================================
*/
void CTextureManager::LoadTexture(const char *filename, TextureData &tData)
{
	static char texname[COM_MAXPATH];
	sprintf(texname,"%s/%s",m_textureDir,filename);

	if(!CImageReader::GetReader().Read(texname, tData))
		CImageReader::GetReader().DefaultTexture(tData);
}
