
#include "Img_entry.h"
#include "Tex_image.h"
#include "ShaderManager.h"

/*
=======================================
Constructor 
=======================================
*/
CImageCacheEntry::CImageCacheEntry(const char *file)
{
	imagefile = new char[strlen(file)+1];
	strcpy(imagefile, file);
	tex_bin = -1;
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
		delete imagefile;

	if (tex_bin != -1)
		g_pRast->TextureBinDestroy(tex_bin);

	if (mShaderBin != -1)
		g_pShaders->BinDestroy(mShaderBin);
}

/*
=======================================
LoadTexture
=======================================
*/
void CImageCacheEntry::LoadTexture(void)
{
	if (tex_bin != -1)
	{
		ComPrintf("CImageCacheEntry::LoadTexture() - texture already loaded\n");
		return;
	}

	if (mShaderBin != -1)
	{
		ComPrintf("CImageCacheEntry::LoadTexture() - shaders already loaded\n");
		return;
	}

	mShaderBin = g_pShaders->BinInit(1);
	g_pShaders->LoadShader(mShaderBin, 0, imagefile);


	tex_bin = g_pRast->TextureBinInit(1);
	CImageReader *texReader = new CImageReader();

	texReader->Read(imagefile);

	// create all mipmaps
	tex_load_t tdata;
	tdata.format = texReader->GetFormat();
	tdata.height = texReader->GetHeight();
	tdata.width  = texReader->GetWidth();
	tdata.mipmaps= texReader->GetNumMips();
	tdata.mipdata= texReader->GetMipData();
	tdata.mipmap = true;
	tdata.clamp  = false;

	int mipcount = tdata.mipmaps - 1;
	while (mipcount > 0)
	{
		texReader->ImageReduce(mipcount);
		mipcount--;
	}

	g_pRast->TextureLoad(tex_bin, 0, &tdata);
	delete texReader;
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

	if (tex_bin != -1)
		g_pRast->TextureBinDestroy(tex_bin);
	tex_bin = -1;
}