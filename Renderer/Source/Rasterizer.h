#ifndef RASTERIZER_H
#define RASTERIZER_H


#include "standard.h"
#include "Com_cvar.h"


// max number of possible mipmaps 10 -> 1024 max dim size
#define MAX_MIPMAPS 11
#define MAX_TEXTURE_BINS 1024

// depth buffer functions
enum EDepthFunc
{
	VRAST_DEPTH_NONE,
	VRAST_DEPTH_FILL,
	VRAST_DEPTH_LEQUAL
};

// source blend factors
enum ESourceBlend
{
	VRAST_SRC_BLEND_NONE,
	VRAST_SRC_BLEND_ZERO,
	VRAST_SRC_BLEND_SRC_ALPHA
};

// destination blend factors
enum EDestBlend
{
	VRAST_DEST_BLEND_NONE,
	VRAST_DEST_BLEND_SRC_COLOR,
	VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA
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

typedef struct
{
	unsigned char **mipdata;
	int mipmaps;	// number of mipmaps
	EImageFormat format;
	bool mipmap;
	bool clamp;
	int height;
	int width;
} tex_load_t;


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

/*
==========================================
Rasterizer Interface
==========================================
*/
class I_Rasterizer
{
public:
	I_Rasterizer() :	m_cWndX("r_wndx","80",CVAR_INT,CVAR_ARCHIVE),
						m_cWndY("r_wndy","40",CVAR_INT,CVAR_ARCHIVE)
		{	g_pConsole->RegisterCVar(&m_cWndX);	g_pConsole->RegisterCVar(&m_cWndY);	}

	virtual ~I_Rasterizer() { }

	//Startup/Shutdown
	virtual bool Init()=0;
	virtual bool Shutdown()=0;
	virtual void Resize()=0;
	virtual void SetWindowCoords(int wndX, int wndY)=0;
	virtual bool UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)=0;

	virtual void DepthFunc(EDepthFunc func)=0;
	virtual void DepthWrite(bool write)=0;
	virtual void BlendFunc(ESourceBlend src, EDestBlend dest)=0;

	virtual int  TextureBinInit(int num)=0;
	virtual int  TextureCount(int bin)=0;
	virtual void TextureBinDestroy(int bin)=0;
	virtual void TextureSet(int bin, int texnum)=0;
	virtual void TextureLoad(int bin, int num, const tex_load_t *texdata)=0;

	virtual void MatrixReset(void)=0;
	virtual void MatrixRotateX(float degrees)=0;
	virtual void MatrixRotateY(float degrees)=0;
	virtual void MatrixRotateZ(float degrees)=0;
	virtual void MatrixTranslate(vector_t &dir)=0;
	virtual void MatrixScale(vector_t &factors)=0;
	virtual void MatrixPush(void)=0;
	virtual void MatrixPop(void)=0;

	virtual void PolyStart(EPolyType type)=0;
	virtual void PolyEnd(void)=0;
	virtual void PolyVertexf(vector_t &vert)=0;
	virtual void PolyVertexi(int x, int y)=0;
	virtual void PolyTexCoord(float s, float t)=0;
	virtual void PolyColor3f(float r, float g, float b)=0;
	virtual void PolyColor4f(float r, float g, float b, float a)=0;

	virtual void ClearBuffers(int buffers)=0;
	virtual void ProjectionMode(EProjectionMode mode)=0;
	virtual void ReportErrors(void)=0;
	virtual void FrameEnd(void)=0;
	virtual void ScreenShot(unsigned char *dest)=0;
	virtual void SetVidSynch(int v)=0;

	virtual void SetFocus(void)=0;

protected:
	CVar    m_cWndX;        //Windowed X pos
	CVar    m_cWndY;        //Windowed Y pos

};

extern I_Rasterizer *g_pRast;

#endif

