#ifndef	RAST_GL_H
#define RAST_GL_H

#include "Rast_main.h"
#include "gl.h"


class COpenGLRast : public CRasterizer
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

	void TextureSet(hTexture texnum);
	void TextureLoad(hTexture index, const TextureData &texdata);
	void TextureUnLoad(hTexture index);
	void TextureClamp(bool clamp);


	void MatrixReset(void);
	void MatrixRotateX(float degrees);
	void MatrixRotateY(float degrees);
	void MatrixRotateZ(float degrees);
	void MatrixTranslate(float x, float y, float z);
	void MatrixScale(vector_t &factors);
	void MatrixPush(void);
	void MatrixPop(void);

	void PolyDraw(void);

	void ClearBuffers(int buffers);
	void ProjectionMode(EProjectionMode mode);
	void ReportErrors(void);
	void FrameEnd(void);
	void ScreenShot(unsigned char *dest);
	void SetVidSynch(int v);

	void SetFocus(void);

private:


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

	GLuint	m_glnames[MAX_TEXTURES];



	EPolyType	mType;
	DWORD		mColor;
//	float		mAlpha;
};

#endif
