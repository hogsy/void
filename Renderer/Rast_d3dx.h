#ifndef	RAST_D3DX_H
#define RAST_D3DX_H

#define D3D_OVERLOADS
#include <windows.h>
#include <d3d.h>
#include <d3dx.h>

#include "Rast_main.h"

class CRastD3DX : public CRasterizer
{
public:

	CRastD3DX();
	~CRastD3DX();

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

	void LockVerts(void) { }
	void UnLockVerts(void) { }

private:

	LPDIRECTDRAWSURFACE7 mTexSurfs[MAX_TEXTURES];


	void RestoreSurfaces(void);

	ID3DXContext		*m_pD3DX;
	LPDIRECTDRAW7		m_pDD;
	LPDIRECT3D7			m_pD3D;
	LPDIRECT3DDEVICE7	m_pD3DDevice;
	LPDIRECT3DVERTEXBUFFER7	m_pvbVertices;

	LPD3DXMATRIXSTACK	m_matView;
	D3DXMATRIX			m_matWorld;
	D3DXMATRIX			m_matProjection;


	bool		mVidSynch;

	HRESULT		mhError;
	bool		m_bInitialized;
};


#endif
