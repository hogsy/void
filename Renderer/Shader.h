#ifndef SHADER_H
#define SHADER_H

#define MAX_ANIMATION		8
#define MAX_SHADER_LAYERS	8


#define CACHE_PASS_SKY			0
#define	CACHE_PASS_ZFILL		1
#define	CACHE_PASS_TRANSPARENT	2
// #define CACHE_PASS_NUM			3	// !! change this in shadermanager.h !!


class CFileBuffer;


enum ETexGen
{
	TEXGEN_BASE,
	TEXGEN_LIGHT,
	TEXGEN_VECTOR
};


class CShaderLayer
{
public:
	CShaderLayer();
	~CShaderLayer();

	void Parse(CFileBuffer *layer, int &texindex);
	void Default(const char *name, int &texindex);
	void GetDims(int &width, int &height);	// get width & height of first non-lightmap layer of shader



	// info about textures for all layers
	struct texname_t
	{
		int index;	// either index to CShader::mTextureBin or -1 means lightmap
		char filename[64];	// only relevant for non-lightmaps
	};

	bool	mIsLight;

	bool	mbMipMap;	// create mipmaps

	int mNumTextures;	// number of textures in this layer - 1 except for animations
	texname_t *mTextureNames;	// the names of the textures

	float	mAnimFreq;	// animation frequency

	// blend funcs
	ESourceBlend	mSrcBlend;
	EDestBlend		mDstBlend;

	// depth func
	EDepthFunc		mDepthFunc;
	bool			mDepthWrite;

	// tex coord generation
	ETexGen			mTexGen;
	vector_t		mTexVector[2];
};


class CShader
{
	friend class CRasterizer;
public:
	CShader(const char *name);
	~CShader();

	void Parse(CFileBuffer *shader);
	void Default(void);

	bool IsShader(const char *s) { return (_stricmp(s, mName)==0); }
	void GetDims(int &width, int &height);	// get width & height of first non-lightmap layer of shader

	unsigned int GetContentFlags(void)	{	return	mContentFlags;	}
	unsigned int GetSurfaceFlags(void)	{	return	mSurfaceFlags;	}

	void AddRef(void);
	void Release(void);

	void LoadTextures(void);
	void UnLoadTextures(void);

	int mPass;	// which pass it should be rendered as

private:


	char mName[64];

	int mRefCount;

	int		mNumLayers;
	CShaderLayer *mLayers[MAX_SHADER_LAYERS];

	unsigned int mSurfaceFlags;
	unsigned int mContentFlags;

	int mNumTextures;	// total number of textures for all layers
	int mTextureBin;	// rasterizer texture bin

};



#endif

