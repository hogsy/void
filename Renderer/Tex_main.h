#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

/*
==========================================
The Texture Manager
Loads/Unloads lightmaps and textures
==========================================
*/


class CTextureManager
{
public:

	CTextureManager();
	~CTextureManager();

	hTexture Load(const char *filename, TextureData &tData);
	hTexture Load(unsigned char **data, TextureData &tData);
	void UnLoad(hTexture tex);

	void LoadAll(void);

private:

	struct TextureName_s
	{
		TextureName_s() { refCount = 0; }

		unsigned char *ptr;	// if not null, it's a pointer to lightmap data
		char file[64];		// otherwise it's a filename

		bool	mipmap;		// do we have mipmaps created for this texture?
							// FIXME - do we really need to do this?
		int		refCount;
	};

	TextureName_s mNames[MAX_TEXTURES];







};

extern CTextureManager * g_pTex;

#endif