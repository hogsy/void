#include "Standard.h"
#include "Tex_hdr.h"
#include "Tex_main.h"
#include "Tex_image.h"

//FIX ME, all the base textures are uploaded as RGBA right now
const char * BaseTextureList[] =
{
	"base/_ascii",
	"base/conback",
	0
};

tex_t			* tex=0;
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

	m_texReader = new CImageReader();

	m_loaded = NO_TEXTURES;
	m_numBaseTextures = 0;
	m_numWorldTextures = 0;
}

/*
==========================================
Destructor
==========================================
*/
CTextureManager::~CTextureManager()
{	delete m_texReader;
}


/*
==========================================
Initialize
Load base game textures
==========================================
*/
bool CTextureManager::Init()
{
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
	tex->bin_base = g_pRast->TextureBinInit(m_numBaseTextures);

	ConPrint("CTextureManager::Init:Creating base textures");

	// load base textures
	for(count=0;count<m_numBaseTextures;count++)
	{
		LoadTexture(BaseTextureList[count]);

		// create all mipmaps
		const tex_load_t *tdata = m_texReader->GetData();
		int mipcount = tdata->mipmaps - 1;
		while (mipcount > 0)
		{
			m_texReader->ImageReduce(mipcount);
			mipcount--;
		}

		g_pRast->TextureLoad(tex->bin_base, count, tdata);



/*
		glBindTexture(GL_TEXTURE_2D, tex->base_names[count]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		int ext_format, int_format;
		if (m_texReader->GetFormat() == IMG_RGB)
		{
			ext_format = GL_RGB;
			int_format = GL_RGB8;
		}
		else
		{
			ext_format = GL_RGBA;
			int_format = GL_RGBA8;
		}

		glTexImage2D(GL_TEXTURE_2D,
				 0,
				 int_format,
				 m_texReader->GetWidth(),
				 m_texReader->GetHeight(),
				 0,
				 ext_format,
				 GL_UNSIGNED_BYTE,
				 m_texReader->GetData());
*/
		
	}
	m_loaded = BASE_TEXTURES;
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
	if (!tex)
		return false;

	UnloadWorldTextures();

	ConPrint("CTextureManager::Shutdown:Destroying base textures :");

	g_pRast->TextureBinDestroy(tex->bin_base);

	delete tex;	
	tex = 0;

	m_loaded = NO_TEXTURES;

	ConPrint("OK\n");
	return true;
}


/*
==========================================
LoadWorld Textures
==========================================
*/
bool CTextureManager::LoadWorldTextures(world_t *map)
{
	if(m_loaded != BASE_TEXTURES)
		return false;

	if (!tex)
		return false;

	if (!map)
		return false;

	uint   mipcount = 0,
		   t=0,m=0;

	m_numWorldTextures = 0;

	//Count number of textures
	while (map->textures[m_numWorldTextures][0] != '\0')
		m_numWorldTextures++;

	tex->bin_world = g_pRast->TextureBinInit(m_numWorldTextures);

	// allocate the poly cache
	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		tex->polycaches[i] = new cpoly_t* [m_numWorldTextures];
		if (!tex->polycaches[i]) 
		{
			FError("mem for map tex cache");
			return false;
		}
		memset(tex->polycaches[i], 0, sizeof(cpoly_t**) * m_numWorldTextures);
	}

	// allocate dim's
	tex->dims = new dimension_t[m_numWorldTextures];
	if (!tex->dims) 
	{
		FError("mem for map tex dims");
		return false;
	}

	for (t=0; t<m_numWorldTextures; t++)
	{

		LoadTexture(map->textures[t]);

		//Set initial dimensions
		tex->dims[t][0] = m_texReader->GetWidth();
		tex->dims[t][1] = m_texReader->GetHeight();

		// create all mipmaps
		const tex_load_t *tdata = m_texReader->GetData();
		mipcount = tdata->mipmaps - 1;
		while (mipcount > 0)
		{
			m_texReader->ImageReduce(mipcount);
			mipcount--;
		}

		g_pRast->TextureLoad(tex->bin_world, t, tdata);


/*
		glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		int ext_format, int_format;
		if (m_texReader->GetFormat() == IMG_RGB)
		{
			ext_format = GL_RGB;

			if (g_p32BitTextures->ival)
				int_format = GL_RGB8;
			else
				int_format = GL_RGB5;
		}
		else
		{
			ext_format = GL_RGBA;
			if (g_p32BitTextures->ival)
				int_format = GL_RGBA8;
			else
				int_format = GL_RGBA4;
		}

		for (m = 0; m < mipcount; m++)
		{
			if(m)
				m_texReader->ImageReduce();

			glTexImage2D(GL_TEXTURE_2D,
						 m,
						 int_format,
						 m_texReader->GetWidth(),
						 m_texReader->GetHeight(),
						 0,
						 ext_format,
						 GL_UNSIGNED_BYTE,
						 m_texReader->GetData());
		}
*/
	}

// FIXME - temp hack to get lightmapping working
	if (!map->nlightdefs || !map->light_size)
	{
		m_loaded = ALL_TEXTURES;
		return true;
	}

	tex->bin_light = g_pRast->TextureBinInit(map->nlightdefs);	// each lightdef has a unique lightmap

	unsigned char *ptr = map->lightdata;
	for (t = 0; t < g_pRast->TextureCount(tex->bin_light); t++)
	{
/*
		m_texReader->ReadLightMap(&ptr);
		mipcount = m_texReader->GetMipCount();

		//Set initial dimensions
		glBindTexture(GL_TEXTURE_2D, tex->light_names[t]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		int ext_format, int_format;
		if (m_texReader->GetFormat() == IMG_RGB)
		{
			ext_format = GL_RGB;

			if (g_p32BitTextures->ival)
				int_format = GL_RGB8;
			else
				int_format = GL_RGB5;
		}
		else
		{
			ext_format = GL_RGBA;
			if (g_p32BitTextures->ival)
				int_format = GL_RGBA8;
			else
				int_format = GL_RGBA4;
		}

		for (m = 0; m < mipcount; m++)
		{
			if(m)
				m_texReader->ImageReduce();

			glTexImage2D(GL_TEXTURE_2D,
						 m,
						 int_format,
						 m_texReader->GetWidth(),
						 m_texReader->GetHeight(),
						 0,
						 ext_format,
						 GL_UNSIGNED_BYTE,
						 m_texReader->GetData());
		}
		m_texReader->Reset();
		*/
	}

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
	if(m_loaded != ALL_TEXTURES)
		return true;

	if (!tex)
		return false;

	ConPrint("Destroying map textures: ");

	g_pRast->TextureBinDestroy(tex->bin_world);
	tex->bin_world = -1;

	for (int i=0; i<CACHE_PASS_NUM; i++)
	{
		delete [] tex->polycaches[i];
		tex->polycaches[i] = NULL;
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

	ConPrint("OK\n");

	m_loaded = BASE_TEXTURES;
	return true;
}

/*
==========================================
Load a map texture
==========================================
*/
void CTextureManager::LoadTexture(const char *filename)
{
	static char texname[COM_MAXPATH];
	sprintf(texname,"%s/%s",m_textureDir,filename);

	if(!m_texReader->Read(texname))
		m_texReader->DefaultTexture();
}
