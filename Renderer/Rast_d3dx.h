#ifndef	RAST_D3DX_H
#define RAST_D3DX_H

#define D3D_OVERLOADS
#include <windows.h>
#include <d3d.h>
#include <d3dx.h>

#include "Rasterizer.h"

class CRastD3DX : public I_Rasterizer
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
			num = -1;
			tex_surfs = NULL;
		}

		int		num;
		LPDIRECTDRAWSURFACE7 *tex_surfs;
	};
	tex_bin_t mTexBins[MAX_TEXTURE_BINS];


	void RestoreSurfaces(void);

	ID3DXContext		*m_pD3DX;
	LPDIRECTDRAW7		m_pDD;
	LPDIRECT3D7			m_pD3D;
	LPDIRECT3DDEVICE7	m_pD3DDevice;
	LPDIRECT3DVERTEXBUFFER7	m_pvbVertices;

	LPD3DXMATRIXSTACK	m_matView;
	D3DXMATRIX			m_matWorld;
	D3DXMATRIX			m_matProjection;

	int					mNumVerts;
	D3DLVERTEX			mVerts[16000];


	vector_t	mColor;
	float		mAlpha;
	EPolyType	mType;

	bool		mVidSynch;

	HRESULT		mhError;
	bool	m_bInitialized;
};


#endif
