#include "Standard.h"
#include "Tex_main.h"
#include "Tex_image.h"


CTextureManager * g_pTex=0;
extern	CVar *	g_p32BitTextures;

/*
==========================================
Constructor
==========================================
*/

CTextureManager::CTextureManager()
{
}

/*
==========================================
Destructor
==========================================
*/
CTextureManager::~CTextureManager()
{
#ifdef DEBUG
	// make sure there arent any textures referenced
	for (int i=0; i<MAX_TEXTURES; i++)
	{
		if (mNames[i].refCount)
			ComPrintf("CTextureManager::~CTextureManager() - %s still referenced\n", mNames[i].file);
	}
#endif
}

/*
==========================================
Load a map texture
==========================================
*/
hTexture CTextureManager::Load(const char *filename, TextureData &tData)
{
	hTexture available = -1;
	for (hTexture t=0; t<MAX_TEXTURES; t++)
	{
		// store first available slot
		if (mNames[t].refCount == 0)
			if (available == -1)
				available = t;
		else
		{
			// only check file textures, not lightmap (data) textures
			if (!mNames[t].ptr && (strcmp(filename, mNames[t].file) == 0) && (tData.bMipMaps == mNames[t].mipmap))
			{
				mNames[t].refCount++;
				return t;
			}
		}
	}

	if (available == -1)
		FError("no available texture slots\n");

	// create a new texture
	if (!CImageReader::GetReader().Read(filename, tData))
		CImageReader::GetReader().DefaultTexture(tData);

	g_pRast->TextureLoad(available, tData);

	mNames[available].ptr = NULL;
	strcpy(mNames[available].file, filename);
	mNames[available].mipmap = tData.bMipMaps;
	mNames[available].refCount = 1;

	// FIXME - we dont want this here
	CImageReader::GetReader().FreeMipData();

	return available;
}

hTexture CTextureManager::Load(unsigned char **data, TextureData &tData)
{
	hTexture available = -1;
	for (hTexture t=0; t<MAX_TEXTURES; t++)
	{
		// store first available slot
		if (mNames[t].refCount == 0)
			if (available == -1)
				available = t;

		else
		{
			// only check data textures, not file textures
			if (mNames[t].ptr && (*data == mNames[t].ptr) && (tData.bMipMaps == mNames[t].mipmap))
			{
				// this should really never happen
				mNames[t].refCount++;
				return t;
			}
		}

	}

	if (available == -1)
		FError("no available texture slots\n");

	// create a new texture
	mNames[available].ptr = *data;
	mNames[available].mipmap = tData.bMipMaps;
	mNames[available].refCount = 1;

	if (!CImageReader::GetReader().ReadLightMap(data, tData))
		CImageReader::GetReader().DefaultTexture(tData);

	g_pRast->TextureLoad(available, tData);

	// FIXME - we dont want this here
	CImageReader::GetReader().FreeMipData();

	return available;
}
	

void CTextureManager::UnLoad(hTexture tex)
{
	mNames[tex].refCount--;

	// free the texture
	if (!mNames[tex].refCount)
	{
		g_pRast->TextureUnLoad(tex);
	}
}


/*
==========================================
Loads all currently ref'd textures - for rasterizer restarts
==========================================
*/
void CTextureManager::LoadAll(void)
{
	TextureData tdata;

	for (int t=0; t<MAX_TEXTURES; t++)
	{
		if (mNames[t].refCount)
		{
			tdata.bMipMaps = mNames[t].mipmap;

			// lightmap
			if (mNames[t].ptr)
			{
				unsigned char *data = mNames[t].ptr;
				if (!CImageReader::GetReader().ReadLightMap(&data, tdata))
					CImageReader::GetReader().DefaultTexture(tdata);

			}

			// file texture
			else
			{
				if (!CImageReader::GetReader().Read(mNames[t].file, tdata))
					CImageReader::GetReader().DefaultTexture(tdata);
			}
			
			g_pRast->TextureLoad(t, tdata);
		}

	}
}










