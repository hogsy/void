#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

/*
==========================================
The Texture Manager
Loads/Unloads lightmaps and textures
==========================================
*/

class CImageReader;

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

	enum
	{
		TEX_MAXTEXTURESIZE = 1048832,
		TEX_MAXMIPMAPSIZE  = 262400	
	};

	enum ETextures
	{
		NO_TEXTURES,
		BASE_TEXTURES,
		ALL_TEXTURES
	};

	int				m_numBaseTextures;
	int				m_numWorldTextures;
	ETextures		m_loaded;

	char			m_textureDir[10];

	CImageReader *	m_texReader;

	void LoadTexture(const char *filename);
};

extern CTextureManager * g_pTex;

#endif