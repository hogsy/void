#include "Standard.h"
#include "ShaderManager.h"
#include "Img_entry.h"
#include "Tex_image.h"


/*
=======================================
Constructor 
=======================================
*/
CImageCacheEntry::CImageCacheEntry(const char *file)
{
	imagefile = new char[strlen(file)+1];
	strcpy(imagefile, file);
	
//	tex_bin = -1;
	mShaderBin = -1;
	mRefCount = 1;
	
	LoadTexture();
}

/*
=======================================
Destructor 
=======================================
*/
CImageCacheEntry::~CImageCacheEntry()
{
	if (imagefile)
		delete [] imagefile;

//	if (tex_bin != -1)
//		g_pRast->TextureBinDestroy(tex_bin);

	if (mShaderBin != -1)
		g_pShaders->BinDestroy(mShaderBin);
}

/*
=======================================
LoadTexture
=======================================
*/
void CImageCacheEntry::LoadTexture()
{
//	if (tex_bin != -1)
//	{
//		ComPrintf("CImageCacheEntry::LoadTexture() - texture already loaded\n");
//		return;
//	}

	if (mShaderBin != -1)
	{
		ComPrintf("CImageCacheEntry::LoadTexture() - shaders already loaded\n");
		return;
	}

	mShaderBin = g_pShaders->BinInit(1);
	g_pShaders->LoadShader(mShaderBin, 0, imagefile, false);

//	tex_bin = g_pRast->TextureBinInit(1);

//	TextureData   tData;
//	tData.bMipMaps = true;
//	tData.bClamped = false;

//	CImageReader::GetReader().Read(imagefile, tData);

//	g_pRast->TextureLoad(tex_bin, 0, tData);
}

/*
=======================================
UnLoadTexture
=======================================
*/
void CImageCacheEntry::UnLoadTexture(void)
{
	if (mShaderBin != -1)
		g_pShaders->BinDestroy(mShaderBin);
	mShaderBin = -1;

//	if (tex_bin != -1)
//		g_pRast->TextureBinDestroy(tex_bin);
//	tex_bin = -1;
}