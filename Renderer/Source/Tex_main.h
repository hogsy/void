#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include "Tex_image.h"

#define TEX_MAXTEXTURESIZE	1048832
#define TEX_MAXMIPMAPSIZE	262400	

class CTextureManager
{
public:
	CTextureManager();
	~CTextureManager();

	bool Init();		//Loads base game textures
	bool Shutdown();	//Unload everything
	
	bool LoadWorldTextures(world_t *map);
	bool UnloadWorldTextures();

private:

	enum ETextures
	{
		NO_TEXTURES,
		BASE_TEXTURES,
		ALL_TEXTURES
	};

	int				m_numBaseTextures;
	int				m_numWorldTextures;
	ETextures		m_loaded;

	CImageReader	m_texReader;

	void LoadTexture(const char *filename);
};

extern CTextureManager * g_pTex;

#endif