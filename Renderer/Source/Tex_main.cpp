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
	tex->base_names = new GLuint[m_numBaseTextures];
	if (tex->base_names == NULL) 
	{
		FError("CTextureManager::Init:No mem for base texture names");
		return false;
	}

	ConPrint("CTextureManager::Init:Creating base textures");

	m_texReader->LockBuffer(TEX_MAXTEXTURESIZE);

	//Generate and load base textures
	glGenTextures(m_numBaseTextures, tex->base_names);
	for(count=0;count<m_numBaseTextures;count++)
	{
		LoadTexture(BaseTextureList[count]);

		glBindTexture(GL_TEXTURE_2D, tex->base_names[count]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

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
		
		m_texReader->Reset();
	}
	m_texReader->UnlockBuffer();
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

	glDeleteTextures(m_numBaseTextures, tex->base_names);
	delete [] tex->base_names;
	
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

	tex->num_lightmaps= 0;
	tex->num_textures = 0;
	
	//Count number of textures
	while (map->textures[tex->num_textures][0] != '\0')
		tex->num_textures++;
	
	tex->tex_names = new GLuint[tex->num_textures];
	if (tex->tex_names == NULL) 
	{
		FError("mem for texture names");
		return false;
	}

	tex->polycaches = new cpoly_t* [tex->num_textures];
	if (!tex->polycaches) 
	{
		FError("mem for map tex cache");
		return false;
	}
	memset(tex->polycaches, 0, sizeof(cpoly_t**) * tex->num_textures);

	tex->dims = new dimension_t[tex->num_textures];
	if (!tex->dims) 
	{
		FError("mem for map tex dims");
		return false;
	}

	m_numWorldTextures = tex->num_textures;
	
	glGenTextures(tex->num_textures, tex->tex_names);

	m_texReader->LockBuffer(TEX_MAXTEXTURESIZE);
	m_texReader->LockMipMapBuffer(TEX_MAXMIPMAPSIZE);

	for (t = 0; t < tex->num_textures; t++)
	{
		LoadTexture(map->textures[t]);
		mipcount = m_texReader->GetMipCount();
			
		//Set initial dimensions
		tex->dims[t][0] = m_texReader->GetWidth();
		tex->dims[t][1] = m_texReader->GetHeight();

		glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

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
	}

// FIXME - temp hack to get lightmapping working
	if (!map->nlightdefs || !map->light_size)
		return true;

	tex->num_lightmaps = map->nlightdefs;

	tex->light_names = new GLuint[tex->num_lightmaps];
	if (tex->light_names == NULL) 
		FError("mem for lightmap names");

	glGenTextures(tex->num_lightmaps, tex->light_names);

	unsigned char *ptr = map->lightdata;
	for (t = 0; t < tex->num_lightmaps; t++)
	{
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
			int_format = GL_RGB8;
		}
		else
		{
			ext_format = GL_RGBA;
			int_format = GL_RGBA8;
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
	}

	m_texReader->UnlockBuffer();
	m_texReader->UnlockMipMapBuffer();

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

	if (!tex || !tex->num_textures)
		return false;

	ConPrint("Destroying map textures: ");

	glDeleteTextures(tex->num_textures, tex->tex_names);
	delete [] tex->tex_names;
	tex->num_textures = 0;

//	free (tex->cache);
//	tex->cache = NULL;
	delete [] tex->polycaches;
	tex->polycaches = NULL;

	// free lightmaps
	if (tex->num_lightmaps)
	{
		glDeleteTextures(tex->num_lightmaps, tex->light_names);
		delete [] tex->light_names;
		tex->num_lightmaps = 0;
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
	{
		m_texReader->Reset();
		m_texReader->DefaultTexture();
	}
}
