
#include "Standard.h"
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
	tex_bin = -1;
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
	g_pRast->TextureBinDestroy(tex_bin);
	tex_bin = -1;
}

