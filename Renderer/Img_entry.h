#ifndef IMG_ENTRY_H
#define IMG_ENTRY_H

class CImageCacheEntry
{
public:
	CImageCacheEntry(const char *file);
	~CImageCacheEntry();

	bool IsFile(const char *file) { return (strcmp(file, imagefile)==0); }

	void LoadTexture(void);
	void UnLoadTexture(void);

//	void Set(void) { if (tex_bin!=-1) g_pRast->TextureSet(tex_bin, 0); }
	void Set(void) { if (mShaderBin!=-1) g_pRast->ShaderSet(g_pShaders->GetShader(mShaderBin, 0)); }

	int  Release(void)	{ return --mRefCount; }
	void AddRef(void)	{ mRefCount++;	}

private:
	int mRefCount;

//	int	tex_bin;		// rasterizer texture name
	int mShaderBin;
	char *imagefile;	// text texture name
};

#endif