#ifndef	RAST_GL_H
#define RAST_GL_H

#include "Rasterizer.h"
#include "gl.h"


class COpenGLRast : public I_Rasterizer
{
public:

	COpenGLRast();
	~COpenGLRast();

	//Startup/Shutdown
	bool Init();
	bool Shutdown();

	void Resize();
	void SetWindowCoords(int wndX, int wndY);
	bool UpdateDisplaySettings(int width, 
							   int height, 
							   int bpp, 
							   bool fullscreen);


	void DepthFunc(EDepthFunc func);
	void DepthWrite(bool write);
	void BlendFunc(ESourceBlend src, EDestBlend dest);

	int  TextureBinInit(int num);
	int  TextureCount(int bin) { return mTexBins[bin].num; }
	void TextureBinDestroy(int bin);
	void TextureSet(int bin, int texnum);
	void TextureLoad(int bin, int num, const TextureData &texdata);

	void MatrixReset(void);
	void MatrixRotateX(float degrees);
	void MatrixRotateY(float degrees);
	void MatrixRotateZ(float degrees);
	void MatrixTranslate(vector_t &dir);
	void MatrixScale(vector_t &factors);
	void MatrixPush(void);
	void MatrixPop(void);

	void PolyStart(EPolyType type);
	void PolyEnd(void);
	void PolyVertexf(vector_t &vert);
	void PolyVertexi(int x, int y);
	void PolyTexCoord(float s, float t);
	void PolyColor3f(float r, float g, float b);
	void PolyColor4f(float r, float g, float b, float a);

	void ClearBuffers(int buffers);
	void ProjectionMode(EProjectionMode mode);
	void ReportErrors(void);
	void FrameEnd(void);
	void ScreenShot(unsigned char *dest);
	void SetVidSynch(int v);

	void SetFocus(void);

private:

	struct tex_bin_t
	{
		tex_bin_t()
		{
			glnames = NULL;
			num = -1;
		}

		GLuint *glnames;
		int		num;
	};



	HDC			hDC;		//device context
	HGLRC		hRC;		//the gl rendering context


	bool GoFull(void);
	bool GoWindowed(void);

	void EnumDisplayModes();
	bool SetupPixelFormat();

	DEVMODE*m_devmodes;		//all available display modes
	int		m_nummodes;		//Number of display modes

	bool	m_loadeddriver;
	bool	m_initialized;
	char	m_gldriver[256];

	bool	m_bInitialized;

	tex_bin_t mTexBins[MAX_TEXTURE_BINS];


	// arrays to store poly data
	vector_t	mVerts[MAX_ELEMENTS];
	float		mColors[MAX_ELEMENTS][4];
	float		mTexCoords[MAX_ELEMENTS][2];	// single pass only right now

	unsigned int	mIndices[MAX_INDICES];
	int			mNumIndices;
	int			mNumElements;
	int	mMaxElements;
	int mMaxIndices;

	EPolyType	mType;
	vector_t	mColor;
	float		mAlpha;
};

#endif
