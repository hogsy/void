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
	if (mShaderBin != -1)
	{
		ComPrintf("CImageCacheEntry::LoadTexture() - shaders already loaded\n");
		return;
	}

	mShaderBin = g_pShaders->BinInit(1);
	g_pShaders->LoadShader(mShaderBin, 0, imagefile, false);
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
}