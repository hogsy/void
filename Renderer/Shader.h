#ifndef SHADER_H
#define SHADER_H

#define MAX_ANIMATION		8
#define MAX_SHADER_LAYERS	8


struct I_FileReader;


// where texture coords come from
enum ETexGen
{
	TEXGEN_BASE,
	TEXGEN_LIGHT,
	TEXGEN_VECTOR,
	TEXGEN_ENVIRONMENT
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
	ALPHAGEN_CONSOLE,
	ALPHAGEN_WAVE
};


typedef struct
{
	EAlphaGen	func;
} alphagen_t;


//===================================================================================================


class CShaderLayer
{
public:
	CShaderLayer();
	~CShaderLayer();

	void Parse(I_FileReader *layer);
	void Default(const char *name);
	void GetDims(int &width, int &height);	// get width & height of first non-lightmap layer of shader



	// info about textures for all layers
	struct texname_t
	{
		int index;	// either index to global texture list or -1 means lightmap
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
	void Default(bool lightmap);

	bool IsShader(const char *s) { return (_stricmp(s, mName)==0); }
	void GetDims(int &width, int &height);	// get width & height of first non-lightmap layer of shader
	bool HasLightmap(void);	// does this shader have a lightmap

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


	// depth func
	EDepthFunc		mDepthFunc;
	bool			mDepthWrite;
	bool			mOriginTexture;

	unsigned int mSurfaceFlags;
	unsigned int mContentFlags;
};



#endif

