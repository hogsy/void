#include "Standard.h"
#include "Tex_main.h"


tex_t *tex;
CTextureManager * g_pTex;


//Need to setup an image list that gets processed when
//the texture manager is initialized 
#define NUM_BASETEXTURES	1

const char * BaseTextureList[] =
{
	"base\\_ascii",
	"base\\conback",
	0
};


/*
==========================================
Constructor
==========================================
*/

CTextureManager::CTextureManager()
{
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
{
}


/*
==========================================
Initialize
Load base game textures
==========================================
*/

bool CTextureManager::Init(char *basepath)
{
	if(!basepath)
		return false;

	//allocate all mem
	tex = (tex_t*)MALLOC(sizeof(tex_t));
	if (tex == NULL) 
		FError("mem for tex struct");
	memset (tex, 0, sizeof(tex_t));


	int count=0;
	CImage texture;
	
	strcpy(CImage::m_texturepath,basepath);
	strcat(CImage::m_texturepath,"\\textures");

	ConPrint("Creating base textures: ");

	//Get count of base textures
	for(count=0;BaseTextureList[count];count++);
		m_numBaseTextures = count+1;

	
	tex->base_names = (GLuint*)MALLOC(sizeof(GLuint) * m_numBaseTextures);
	if (tex->base_names == NULL) 
			FError("mem for base texture names");

	glGenTextures(m_numBaseTextures, tex->base_names);

	for(count=0;count<m_numBaseTextures;count++)
	{
		LoadBaseTexture(&texture,BaseTextureList[count]);
		glBindTexture(GL_TEXTURE_2D, tex->base_names[count]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D,
				 0,
				 GL_RGBA,
				 texture.width,
				 texture.height,
				 0,
				 GL_RGBA,
				 GL_UNSIGNED_BYTE,
				 texture.data);
		texture.Reset();
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

	ConPrint("Destroying base textures: ");

	glDeleteTextures(m_numBaseTextures, tex->base_names);
	free (tex->base_names);

	free (tex);
	tex = NULL;

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

	int	   mipcount = 0,
		   t=0,m=0;

	tex->num_lightmaps= 0;
	tex->num_textures = 0;
	while (map->textures[tex->num_textures][0] != '\0')
		tex->num_textures++;

	tex->tex_names = (GLuint*)MALLOC(sizeof(GLuint) * tex->num_textures);
	if (tex->tex_names == NULL) 
		FError("mem for texture names");

	tex->polycaches = (cpoly_t**)MALLOC(sizeof(cpoly_t*) * tex->num_textures);
	if (!tex->polycaches) 
		FError("mem for map tex cache");
	memset(tex->polycaches, 0, sizeof(cpoly_t**) * tex->num_textures);

	tex->dims = (dimension_t*)MALLOC(sizeof(dimension_t) * tex->num_textures);
	if (!tex->dims) 
		FError("mem for map tex dims");

	m_numWorldTextures = tex->num_textures;

	CImage texture;
	texture.AllocFileBuffer();

	glGenTextures(tex->num_textures, tex->tex_names);

	for (t = 0; t < tex->num_textures; t++)
	{
		mipcount = LoadTexture(&texture,map->textures[t]);

		//Set initial dimensions
		tex->dims[t][0] = texture.width;
		tex->dims[t][1] = texture.height;

		glBindTexture(GL_TEXTURE_2D, tex->tex_names[t]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		for (m = 0; m < mipcount; m++)
		{
			if(m)
				texture.ImageReduce();

			glTexImage2D(GL_TEXTURE_2D,
						 m,
						 GL_RGBA,
						 texture.width,
						 texture.height,
						 0,
						 GL_RGBA,
						 GL_UNSIGNED_BYTE,
						 texture.data);
		}
		texture.Reset();
	}

	texture.FreeFileBuffer();	
	m_loaded = ALL_TEXTURES;


// FIXME - temp hack to get lightmapping working

	if (!map->nlightdefs || !map->light_size)
		return true;

	tex->num_lightmaps = map->nlightdefs;

	tex->light_names = (GLuint*)MALLOC(sizeof(GLuint) * tex->num_lightmaps);
	if (tex->light_names == NULL) 
		FError("mem for lightmap names");

	glGenTextures(tex->num_lightmaps, tex->light_names);

	unsigned char *ptr = map->lightdata;
	for (t = 0; t < tex->num_lightmaps; t++)
	{
		mipcount = LoadTexture(&texture, &ptr);

		//Set initial dimensions
		glBindTexture(GL_TEXTURE_2D, tex->light_names[t]);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

		for (m = 0; m < mipcount; m++)
		{
			if(m)
				texture.ImageReduce();

			glTexImage2D(GL_TEXTURE_2D,
						 m,
						 GL_RGBA,
						 texture.width,
						 texture.height,
						 0,
						 GL_RGBA,
						 GL_UNSIGNED_BYTE,
						 texture.data);
		}
		texture.Reset();
	}
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
	free (tex->tex_names);
	tex->num_textures = 0;

//	free (tex->cache);
//	tex->cache = NULL;
	free (tex->polycaches);
	tex->polycaches = NULL;

	// free lightmaps
	if (tex->num_lightmaps)
	{
		glDeleteTextures(tex->num_lightmaps, tex->light_names);
		free (tex->light_names);
		tex->num_lightmaps = 0;
	}

	if(tex->dims)
	{
		free(tex->dims);
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
int CTextureManager::LoadTexture(CImage *texture, const char *filename)
{
	if(!texture)
		return 0;

	if (filename)
	{
		if (!texture->Read(filename))
		{
			texture->Reset();
			texture->DefaultTexture();
		}
	}
	else
	{
		texture->DefaultTexture();
	}

	int tmps = 1;
	int miplevels = 0;
	
	// make it a power of 2
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (texture->width&tmps)
			break;
	}
	texture->width = tmps;
	
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (texture->height&tmps)
			break;
	}
	texture->height = tmps;

	int largestdim = texture->width;
	if (texture->width < texture->height)
		largestdim = texture->height;

	// figure out how many mip map levels we should have
	for (miplevels=1, tmps=1; tmps < largestdim; tmps <<= 1)
		miplevels++;

	if(miplevels > 10)
		miplevels = 10;
	return miplevels;
}

/*
==========================================
Load Lightmap textures
==========================================
*/
int  CTextureManager::LoadTexture(CImage *texture, unsigned char **stream)
{
	if(!texture)
		return 0;

	texture->Read(stream);

	int tmps = 1;
	int miplevels = 0;
	
	// make it a power of 2
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (texture->width&tmps)
			break;
	}
	texture->width = tmps;
	
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (texture->height&tmps)
			break;
	}
	texture->height = tmps;

	int largestdim = texture->width;
	if (texture->width < texture->height)
		largestdim = texture->height;

	// figure out how many mip map levels we should have
	for (miplevels=1, tmps=1; tmps < largestdim; tmps <<= 1)
		miplevels++;

	if(miplevels > 10)
		miplevels = 10;
	return miplevels;
}

/*
==========================================
Load Base Textures
NO mip maps, colorkeyed
==========================================
*/
bool CTextureManager::LoadBaseTexture(CImage *texture,const char *filename)
{
	if(!texture)
		return false;

	if (filename)
	{
		if (!texture->Read(filename))
		{
			texture->Reset();
			texture->DefaultTexture();
		}
	}
	else
	{
		texture->DefaultTexture();
	}
	texture->ColorKey();
	return true;
}





