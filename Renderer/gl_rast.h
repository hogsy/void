#ifndef	RAST_GL_H
#define RAST_GL_H

#include "Rast_main.h"
#include "gls.h"
#include "glext.h"


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

	void LockVerts(void);
	void UnLockVerts(void);

	void LightSet(bool enable);

private:


	HDC			hDC;		//device context
	HGLRC		hRC;		//the gl rendering context


	bool GoFull(void);
	bool GoWindowed(void);

	void EnumDisplayModes();
	bool SetupPixelFormat();
	void GetExtensions();

	DEVMODE*m_devmodes;		//all available display modes
	int		m_nummodes;		//Number of display modes


	GLuint	m_glnames[MAX_TEXTURES];

	bool	m_CVAsupported;
	bool	m_vsynchsupported;
	bool	m_MultiSupported;	// multitexturing

// gl extensions
//====================================================================================
	PFNGLMULTITEXCOORD2FARBPROC		glMultiTexCoord2fARB;
	PFNGLACTIVETEXTUREARBPROC		glActiveTextureARB;
	PFNGLLOCKARRAYSEXTPROC			glLockArraysEXT;
	PFNGLUNLOCKARRAYSEXTPROC		glUnlockArraysEXT;

	typedef BOOL (APIENTRY * WGLSWAPINTERVALEXT) (int interval);
	WGLSWAPINTERVALEXT		wglSwapIntervalEXT;

	EPolyType	mType;
	DWORD		mColor;
//	float		mAlpha;
};

#endif
