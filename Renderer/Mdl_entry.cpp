#include "Standard.h"
#include "Mdl_entry.h"
#include "Tex_image.h"
//#include "Client.h"


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
		delete modelfile;

	if (skin_bin != -1)
		g_pRast->TextureBinDestroy(skin_bin);

	if (skin_names)
	{
		for (int s=0; s<num_skins; s++)
			delete skin_names[s];
		delete skin_names;
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
	CImageReader *texReader = new CImageReader();

	for (int s=0; s<num_skins; s++)
	{
		strcpy(texname, path);
		strcat(texname, "/");
		strcat(texname, skin_names[s]);

		if ((strcmp(skin_names[s], "none")==0) || !texReader->Read(texname))
			texReader->DefaultTexture();

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

		g_pRast->TextureLoad(skin_bin, s, &tdata);
	}

	delete texReader;
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
	skin_bin = -1;
}

