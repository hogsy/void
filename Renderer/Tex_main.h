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
	
	bool LoadWorldTextures(CWorld *map);
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

	char			m_textureDir[10];

	CImageReader *	m_texReader;

	void LoadTexture(const char *filename, TextureData &tData);
};

extern CTextureManager * g_pTex;

#endif