#ifndef TEXTURE_H
#define TEXTURE_H

//#include "gl.h"
#include "Tex_image.h"
#include "Ren_cache.h"



typedef int dimension_t[2];
typedef struct
{
	unsigned int num_textures;
	unsigned int num_lightmaps;
	GLuint		*base_names;
	GLuint		*tex_names;
	GLuint		*light_names;
	dimension_t	*dims;
	cpoly_t		**polycaches;
} tex_t;


class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	bool Init(char *basepath);	//Loads base game textures
	bool Shutdown();			//Unload everything
	
	bool LoadWorldTextures(world_t *map);
	bool UnloadWorldTextures();

private:

	enum ETextures
	{
		NO_TEXTURES,
		BASE_TEXTURES,
		ALL_TEXTURES
	};

	int			m_numBaseTextures;
	int			m_numWorldTextures;

	ETextures	m_loaded;

	int  LoadTexture(CImage *texture, unsigned char **stream);
	int  LoadTexture(CImage *texture, const char *filename);
	bool LoadBaseTexture(CImage *texture,const char *filename);
};


extern tex_t *tex;
extern CTextureManager * g_pTex;


#endif