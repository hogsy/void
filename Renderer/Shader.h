#ifndef SHADER_H
#define SHADER_H

#define MAX_ANIMATION		8
#define MAX_SHADER_LAYERS	8


#define CACHE_PASS_SKY			0
#define	CACHE_PASS_ZFILL		1
#define	CACHE_PASS_TRANSPARENT	2
// #define CACHE_PASS_NUM			3	// !! change this in shadermanager.h !!


struct I_FileReader;


// where texture coords come from
enum ETexGen
{
	TEXGEN_BASE,
	TEXGEN_SKY,
	TEXGEN_LIGHT,
	TEXGEN_VECTOR
};


//===================================================================================================
// tcmod's - modify texture coords
class CTCModBase
{
public:
	CTCModBase() : mpNext(NULL) {}
	~CTCModBase()	{	if (mpNext)	delete mpNext;	}	// delete the entire chain

	// add a tcmod to the chain
	void Add(CTCModBase *mod)
	{
		if (mpNext) mpNext->Add(mod);
		else mpNext = mod;
	}

	virtual void	Evaluate(float &s, float &t, float time)=0;	// modify the tex coords

	CTCModBase *mpNext;
};


// head - doesnt do anything, just the head of the list
class CTCModHead : public CTCModBase
{
public:
	CTCModHead() : CTCModBase()	{}
	void	Evaluate(float &s, float &t, float time)
	{	if (mpNext) mpNext->Evaluate(s, t, time);	}
};


// scroll
class CTCModScroll : public CTCModBase
{
public:
	CTCModScroll(float x, float y) : CTCModBase(), mX(x), mY(y) {}
	void	Evaluate(float &s, float &t, float time)
	{
		float _t = time; // - floorf(time);
		float add;

		add = _t*mX;
		s += add - floorf(add);

		add = _t*mY;
		t += add - floorf(add);


		if (mpNext) mpNext->Evaluate(s, t, time);
	}

private:
	float	mX, mY;
};


// scale
class CTCModScale : public CTCModBase
{
public:
	CTCModScale(float x, float y) : CTCModBase(), mX(x), mY(y) {}
	void	Evaluate(float &s, float &t, float time)
	{
		s /= mX;
		t /= mY;
		if (mpNext) mpNext->Evaluate(s, t, time);
	}
private:
	float	mX, mY;
};


//===================================================================================================
// where alpha values come from
enum EAlphaGen
{
	ALPHAGEN_IDENTITY,
	ALPHAGEN_CONSOLE
};


typedef struct
{
	EAlphaGen	func;
} alphagen_t;



class CShaderLayer
{
public:
	CShaderLayer();
	~CShaderLayer();

	void Parse(I_FileReader *layer, int &texindex);
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

	CTCModBase		*mHeadTCMod;

	bool			mTextureClamp;

	// alpha component generation
	alphagen_t		mAlphaGen;

};


class CShader
{
	friend class CRasterizer;
public:
	CShader(const char *name);
	~CShader();

	void Parse(I_FileReader *shader);
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

