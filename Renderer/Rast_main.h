#ifndef RASTERIZER_H
#define RASTERIZER_H


// max number of possible mipmaps 10 -> 1024 max dim size
#define MAX_MIPMAPS 11
#define MAX_TEXTURES 2048

#define MAX_INDICES		(8192*2)
#define MAX_ELEMENTS	(8192*2)

// depth buffer functions
enum EDepthFunc
{
	VRAST_DEPTH_NONE,
	VRAST_DEPTH_ALWAYS,
	VRAST_DEPTH_LEQUAL,
	VRAST_DEPTH_EQUAL
};

// source blend factors
enum ESourceBlend
{
	VRAST_SRC_BLEND_NONE,
	VRAST_SRC_BLEND_ZERO,
	VRAST_SRC_BLEND_ONE,
	VRAST_SRC_BLEND_SRC_ALPHA,
	VRAST_SRC_BLEND_INV_SRC_ALPHA,
	VRAST_SRC_BLEND_DST_COLOR,
	VRAST_SRC_BLEND_INV_DST_COLOR
};

// destination blend factors
enum EDestBlend
{
	VRAST_DST_BLEND_NONE,
	VRAST_DST_BLEND_ZERO,
	VRAST_DST_BLEND_ONE,
	VRAST_DST_BLEND_SRC_COLOR,
	VRAST_DST_BLEND_INV_SRC_COLOR,
	VRAST_DST_BLEND_SRC_ALPHA,
	VRAST_DST_BLEND_INV_SRC_ALPHA
};


// texture bins - base, world, lightmap
#define VRAST_TEXBIN_BASE	0
#define VRAST_TEXBIN_WORLD	1
#define VRAST_TEXBIN_LIGHT	2


// struct to hold all data needed to load a texture
enum EImageFormat
{
	IMG_NONE = 0,
	IMG_ALPHA =1,
	IMG_RGB   =3,
	IMG_RGBA  =4
};

/*
================================================

================================================
*/
struct TextureData
{
	TextureData() : bMipMaps(false), bClamped(false), numMipMaps(1),
					height (0), width(0), data(0) 	{}
	~TextureData(){ data = 0; }

	bool	bMipMaps;
	bool	bClamped;
	int		numMipMaps;

	int		height;
	int		width;
	byte ** data;

	EImageFormat  format;
};

typedef int hTexture;

// buffers that may be cleared
#define VRAST_COLOR_BUFFER		1
#define VRAST_DEPTH_BUFFER		2
#define VRAST_STENCIL_BUFFER	4


// projection modes
enum EProjectionMode
{
	VRAST_PERSPECTIVE,
	VRAST_ORTHO
};


enum EPolyType
{
	VRAST_TRIANGLE_FAN,
	VRAST_TRIANGLE_STRIP,
	VRAST_QUADS
};



#ifdef RENDERER

class CShader;

/*
==========================================
Rasterizer Interface
==========================================
*/
class CRasterizer
{
public:
	CRasterizer();
	virtual ~CRasterizer() { }

	//Startup/Shutdown
	virtual bool Init()=0;
	virtual bool Shutdown()=0;
	virtual void Resize()=0;
	virtual void SetWindowCoords(int wndX, int wndY)=0;
	virtual bool UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)=0;

	virtual void TextureLoad(hTexture index, const TextureData &texdata)=0;
	virtual void TextureUnLoad(hTexture index)=0;
	virtual void TextureClamp(bool clamp)=0;

	virtual void MatrixReset(void)=0;
	virtual void MatrixRotateX(float degrees)=0;
	virtual void MatrixRotateY(float degrees)=0;
	virtual void MatrixRotateZ(float degrees)=0;
	virtual void MatrixTranslate(float x, float y, float z)=0;
	virtual void MatrixScale(vector_t &factors)=0;
	virtual void MatrixPush(void)=0;
	virtual void MatrixPop(void)=0;

	void PolyStart(EPolyType type);
	void PolyEnd(void);
	void PolyVertexf(vector_t &vert);
	void PolyVertexi(int x, int y);
	void PolyTexCoord(float s, float t);
	void PolyColor(float r, float g, float b, float a);

	void ShaderSet(CShader *shader);
	void TextureTexDef(bspf_texdef_t *def);
	void TextureLightDef(bspf_texdef_t *def);

	virtual void ClearBuffers(int buffers)=0;
	virtual void ProjectionMode(EProjectionMode mode)=0;
	virtual void ReportErrors(void)=0;
	virtual void FrameEnd(void)=0;
	virtual void ScreenShot(unsigned char *dest)=0;
	virtual void SetVidSynch(int v)=0;

	virtual void SetFocus(void)=0;

	unsigned int GetNumTris(void)	{ return mTrisDrawn;	}
	void UseLights(bool l)	{	mUseLights = l;		}
	void ConAlpha(byte t, byte b)	{	Flush();	mConAlphaTop = t;	mConAlphaBot = b;	}


private:

	void DrawLayer(int l);
	virtual void PolyDraw(void)=0;
	virtual void TextureSet(hTexture texnum)=0;

	virtual void DepthFunc(EDepthFunc func)=0;
	virtual void DepthWrite(bool write)=0;
	virtual void BlendFunc(ESourceBlend src, EDestBlend dest)=0;

	// called just before / after pushing verts through to the rasterizer
	virtual void LockVerts(void)=0;
	virtual void UnLockVerts(void)=0;



protected:
	void Flush(void);

	// current rasterizer states
	EDepthFunc		mCurDepthFunc;
	bool			mCurDepthWrite;
	ESourceBlend	mCurSrcBlend;
	EDestBlend		mCurDstBlend;


	// arrays to store poly data

	typedef struct
	{
		float	pos[3];
		DWORD	color;
		float	tex1[2];
	} rast_vertex_t;


	CShader	*mShader;	// current shader
	bspf_texdef_t	*mTexDef;
	bspf_texdef_t	*mLightDef;


	float	mTexCoords[MAX_ELEMENTS][2];
	float	mLightCoords[MAX_ELEMENTS][2];

	rast_vertex_t	mVerts[MAX_ELEMENTS];
	unsigned short	mIndices[MAX_INDICES];
	DWORD			mColor;	// current color
	EPolyType		mType;

	int			mNumIndices;
	int			mNumElements;
	int			mFirstIndex;
	int			mFirstElement;

	int			mMaxElements;
	int			mMaxIndices;

	byte		mConAlphaTop;	// alpha value used for generating console alpha's
	byte		mConAlphaBot;

	unsigned int	mTrisDrawn;	// number of tris pushed through per frame
	bool			mUseLights;	// use any kind of light (light ents or lightmaps)
};

// cvars the rasterizers use

extern CVar * g_varWndX;        //Windowed X pos
extern CVar * g_varWndY;        //Windowed Y pos

extern CVar * g_var32BitTextures;
extern CVar * g_varFov;
extern CVar * g_varD3DXShift;
extern CVar * g_varGLDriver;

#endif	// RENDERER
#endif
