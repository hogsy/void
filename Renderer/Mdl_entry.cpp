#include "Standard.h"
#include "Mdl_entry.h"
#include "Tex_image.h"
#include "ShaderManager.h"

/*
=======================================
Constructor 
=======================================
*/
CModelCacheEntry::CModelCacheEntry()
{
	mRefCount = 1;
	num_skins = 0;
	skin_bin = -1;
	mShaderBin = -1;
	skin_names = NULL;
	num_frames = 0;
}

/*
=======================================
Destructor 
=======================================
*/
CModelCacheEntry::~CModelCacheEntry()
{
	if (modelfile)
		delete [] modelfile;

	if (skin_bin != -1)
		g_pRast->TextureBinDestroy(skin_bin);

	if (mShaderBin != -1)
		g_pShaders->BinDestroy(mShaderBin);

	if (skin_names)
	{
		for (int s=0; s<num_skins; s++)
			delete [] skin_names[s];
		delete [] skin_names;
	}
}


/*
=======================================
LoadSkins
=======================================
*/
void CModelCacheEntry::LoadSkins(void)
{
	if (skin_bin != -1)
	{
		ComPrintf("CModelCacheEntry::LoadSkins() - skins already loaded\n");
		return;
	}

	if (mShaderBin != -1)
	{
		ComPrintf("CModelCacheEntry::LoadSkins() - shaders already loaded\n");
		return;
	}

	char texname[260];
	char path[260];

	strcpy(path, modelfile);
	for (int c=strlen(path); c>=0; c--)
	{
		if ((path[c] == '\\') || (path[c] == '/'))
		{
			path[c] = '\0';
			break;
		}
	}

	skin_bin = g_pRast->TextureBinInit(num_skins);
	mShaderBin = g_pShaders->BinInit(num_skins);

	TextureData  tData;

	for (int s=0; s<num_skins; s++)
	{
		strcpy(texname, path);
		strcat(texname, "/");
		strcat(texname, skin_names[s]);

		if ((strcmp(skin_names[s], "none")==0) || !CImageReader::GetReader().Read(texname, tData))
			CImageReader::GetReader().DefaultTexture(tData);

		// create all mipmaps
		tData.bMipMaps = true;
		tData.bClamped = false;

		int mipCount = tData.numMipMaps - 1;
		while (mipCount > 0)
		{
			CImageReader::GetReader().ImageReduce(mipCount);
			mipCount--;
		}

		g_pShaders->LoadShader(mShaderBin, s, texname);
		g_pRast->TextureLoad(skin_bin, s, tData);
	}
}

/*
=======================================
UnLoadSkins
=======================================
*/
void CModelCacheEntry::UnLoadSkins(void)
{
	if (skin_bin != -1)
		g_pRast->TextureBinDestroy(skin_bin);

	if (mShaderBin != -1)
		g_pShaders->BinDestroy(mShaderBin);

	mShaderBin = -1;
	skin_bin = -1;
}

