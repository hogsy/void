
#include "Standard.h"
#include "Rast_d3dx.h"



/*
=======================================
Constructor 
=======================================
*/
CRastD3DX::CRastD3DX()
{
	m_bInitialized = false;

	m_pD3DX	= NULL;
	m_pDD	= NULL;
	m_pD3D	= NULL;
	m_pvbVertices = NULL;
	mhError = S_OK;
	m_matView = NULL;

	mNumVerts = 0;
}


/*
==========================================
Destructor
==========================================
*/
CRastD3DX::~CRastD3DX()
{
}


/*
==========================================
Init
==========================================
*/
bool CRastD3DX::Init()
{

	RECT wrect;
	wrect.left = m_cWndX.ival;
	wrect.top = m_cWndY.ival;
	wrect.right = m_cWndX.ival + g_rInfo.width;
	wrect.bottom= m_cWndY.ival + g_rInfo.height;
	
	//Adjusts Client Size
	::AdjustWindowRect(&wrect, 
					   WS_CAPTION,
					   FALSE);

	int width = wrect.right - wrect.left;
	int height = wrect.bottom - wrect.top;

	::SetWindowPos(g_rInfo.hWnd,
				   HWND_TOP,
				   wrect.left,
				   wrect.top,
			       width,
			       height,
				   0);



	HRESULT hr;

	if(FAILED(hr = D3DXInitialize()))
	{
		mhError = hr;
		return false;
	}

    if(g_rInfo.rflags & RFLAG_FULLSCREEN)
    {
		hr = D3DXCreateContextEx(D3DX_DEFAULT, D3DX_CONTEXT_FULLSCREEN, g_rInfo.hWnd, NULL, g_rInfo.bpp, 0,
			 D3DX_DEFAULT, 0, 1, g_rInfo.width, g_rInfo.height, D3DX_DEFAULT, &m_pD3DX);

        if(FAILED(hr))
        {
			hr = D3DXCreateContextEx(D3DX_DEFAULT, D3DX_CONTEXT_FULLSCREEN, g_rInfo.hWnd, NULL, D3DX_DEFAULT, 0,
				 D3DX_DEFAULT, 0, 1, g_rInfo.width, g_rInfo.height, D3DX_DEFAULT, &m_pD3DX);
        }
    }
    else
    {
		hr = D3DXCreateContextEx(D3DX_DEFAULT, 0, g_rInfo.hWnd, NULL, D3DX_DEFAULT, 0,
			 D3DX_DEFAULT, 0, 1, g_rInfo.width, g_rInfo.height, D3DX_DEFAULT, &m_pD3DX);
    }

	if(FAILED(hr))
	{
		mhError = hr;
		return false;
	}

	if(!(m_pDD = m_pD3DX->GetDD()))
	{
		mhError = E_FAIL;
		return false;
	}

    if(!(m_pD3D = m_pD3DX->GetD3D()))
	{
		mhError = E_FAIL;
		return false;
	}

    if(!(m_pD3DDevice = m_pD3DX->GetD3DDevice()))
	{
		mhError = E_FAIL;
		return false;
	}


	if (FAILED (hr = D3DXCreateMatrixStack(0, &m_matView)))
	{
		mhError = hr;
		return false;
	}

	m_matView->LoadIdentity();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW,		(D3DMATRIX *)m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_WORLD,		(D3DMATRIX *)D3DXMatrixIdentity(&m_matWorld));
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION,(D3DMATRIX *)D3DXMatrixIdentity(&m_matProjection));

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_LIGHTING, FALSE);

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_CLIPPLANEENABLE, 0);
	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_CULLMODE, D3DCULL_NONE);

	g_rInfo.ready = true;
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool CRastD3DX::Shutdown()
{
	//Update Win Pos
	RECT rect;
	if(GetWindowRect(g_rInfo.hWnd, &rect))
	{
		if(rect.left < 40)	rect.left= 40;
		if(rect.top  < 20)	rect.top = 20;

		SetWindowCoords(rect.left, rect.top);
	}

	if (m_matView)
		m_matView->Release();
	m_matView = NULL;

	if (m_pD3DDevice)
		m_pD3DDevice->Release();
	m_pD3DDevice = NULL;

	if (m_pD3D)
		m_pD3D->Release();
	m_pD3D = NULL;

	if (m_pDD)
		m_pDD->Release();
	m_pDD = NULL;

	if (m_pD3DX)
		m_pD3DX->Release();
	m_pD3DX = NULL;

    D3DXUninitialize();

	g_rInfo.ready = false;
	return true;
}


/*
==========================================
RestoreSurfaces
==========================================
*/
void CRastD3DX::RestoreSurfaces(void)
{
	m_pD3DX->RestoreSurfaces();

/*
	for (int i=0; i<MAX_TEXTURE_BINS; i++)
	{
		for (int t=0; t<mTexBins[i].num; t++)
		{
			if (mTexBins[i].tex_surfs[t])
				mTexBins[i].tex_surfs[t]->


		}
	}
*/
}


/*
==========================================
Update Default window coords
==========================================
*/
void CRastD3DX::SetWindowCoords(int wndX, int wndY)
{
	//Dont bother with initial co-ords
	if(!m_bInitialized || (g_rInfo.rflags&RFLAG_FULLSCREEN))
		return;

	m_cWndX.Set(wndX);
	m_cWndY.Set(wndY);
}

/*
==========================================
Resize the Window
==========================================
*/
void CRastD3DX::Resize()
{
	RECT crect;
	GetClientRect(g_rInfo.hWnd, &crect);
	g_rInfo.width  = crect.right - crect.left;
	g_rInfo.height = crect.bottom - crect.top;


	D3DVIEWPORT7 viewData;
	memset(&viewData, 0, sizeof(D3DVIEWPORT7));
	viewData.dwX = 0;
	viewData.dwY = 0;
	viewData.dwWidth  = g_rInfo.width;
	viewData.dwHeight = g_rInfo.height;
	viewData.dvMinZ = 0.0f;     
	viewData.dvMaxZ = 1.0f;

	m_pD3DDevice->SetViewport(&viewData);
}


/*
==========================================
Updates display settings
==========================================
*/
bool CRastD3DX::UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)
{
	Shutdown();

	// record old stats
	bool oldfull = g_rInfo.rflags & RFLAG_FULLSCREEN;
	uint oldwidth= g_rInfo.width;
	uint oldheight= g_rInfo.height;
	uint oldbpp	  = g_rInfo.bpp;

	g_rInfo.bpp		= bpp;
	g_rInfo.width	= width;
	g_rInfo.height	= height;

	if (fullscreen)
		g_rInfo.rflags |= RFLAG_FULLSCREEN;
	else
		g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

	if (!Init())
	{
		ComPrintf("D3DX::UpdateDisplaySettings: Unable to change to new settings\n");

		// switch everythign back;
		if (oldfull)
			g_rInfo.rflags |= RFLAG_FULLSCREEN;
		else
			g_rInfo.rflags &= ~RFLAG_FULLSCREEN;

		g_rInfo.bpp	= oldbpp;
		g_rInfo.width	= oldwidth;
		g_rInfo.height	= oldheight;

		Init();
		return false;
	}

	ComPrintf("D3DX::UpdateDisplaySettings::Display change successful\n");
	return true;
}


/*
==========================================
SetFocus
==========================================
*/
void CRastD3DX::SetFocus()
{
}



//=========================================================================================================================
// D3DX implementation of drawing functions
//=========================================================================================================================



void CRastD3DX::DepthFunc(EDepthFunc func)
{
	switch (func)
	{
	case VRAST_DEPTH_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
		return;

	case VRAST_DEPTH_FILL:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,   D3DCMP_ALWAYS);
		return;

	case VRAST_DEPTH_LEQUAL:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL);
		return;
	}
}

void CRastD3DX::DepthWrite(bool write)
{
	if (write)
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	else
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
}

void CRastD3DX::BlendFunc(ESourceBlend src, EDestBlend dest)
{
	int source = 0;
	int destination = 0;

	switch (src)
	{
	case VRAST_SRC_BLEND_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		return;
	case VRAST_SRC_BLEND_ZERO:
		source = D3DBLEND_ZERO;
		break;
	case VRAST_SRC_BLEND_SRC_ALPHA:
		source = D3DBLEND_SRCALPHA;
		break;
	}

	switch (dest)
	{
	case VRAST_DEST_BLEND_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		return;
	case VRAST_DEST_BLEND_SRC_COLOR:
		destination = D3DBLEND_SRCCOLOR;
		break;
	case VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA:
		destination = D3DBLEND_INVSRCALPHA;
		break;
	}

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, source);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, destination);
}


/*
========
TextureBinInit
========
*/
int CRastD3DX::TextureBinInit(int num)
{
	for (int i=0; i<MAX_TEXTURE_BINS; i++)
	{
		if (mTexBins[i].num == -1)
		{
			mTexBins[i].num = num;
			mTexBins[i].tex_surfs = new LPDIRECTDRAWSURFACE7[num];
			if (!mTexBins[i].tex_surfs)
				FError("d3dx - not enough mem for texture surf pointers");

			m_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFN_LINEAR);
			m_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
			m_pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_POINT);
//			m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
//			m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);

			m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


			m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT );
			m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

			for (int t=0; t<num; t++)
				mTexBins[i].tex_surfs[t] = NULL;

			return i;
		}
	}

	Error("CRastD3DX::TextureBinInit - out of texture bins\n");
	return -1;
}


/*
========
TextureBinDestroy
========
*/
void CRastD3DX::TextureBinDestroy(int bin)
{

	if ((bin < 0) || (bin > MAX_TEXTURE_BINS) || (mTexBins[bin].num == -1))
	{
		Error("destroying non-existant texture bin!");
		return;
	}

	for (int t=0; t<mTexBins[bin].num; t++)
		if (mTexBins[bin].tex_surfs[t])
			mTexBins[bin].tex_surfs[t]->Release();

	delete mTexBins[bin].tex_surfs;

	mTexBins[bin].tex_surfs = NULL;
	mTexBins[bin].num = -1;
}



void CRastD3DX::TextureSet(int bin, int texnum)
{
	m_pD3DDevice->SetTexture(0, mTexBins[bin].tex_surfs[texnum]);
}

void CRastD3DX::TextureLoad(int bin, int num, const tex_load_t *texdata)
{
	D3DX_SURFACEFORMAT ext_format, int_format;

	int bpp = texdata->format == IMG_RGB ? 3 : 4;

	int w = texdata->width;
	int h = texdata->height;

	// have to swap all blue and red values
	for (int m=texdata->mipmaps-1; m>=0; m--)
	{
		for (int r=0; r<h; r++)
		{
			for (int c=0; c<w; c++)
			{
				char tmp;
				tmp = texdata->mipdata[m][r*w*bpp + c*bpp + 0];
				texdata->mipdata[m][r*w*bpp + c*bpp + 0] = texdata->mipdata[m][r*w*bpp + c*bpp + 2];
				texdata->mipdata[m][r*w*bpp + c*bpp + 2] = tmp;
			}
		}
		h /= 2;
		w /= 2;
		if (!h) h = 1;
		if (!w) w = 1;
	}

	if (texdata->format == IMG_RGB)
	{
		ext_format = D3DX_SF_R8G8B8;
		int_format = D3DX_SF_R8G8B8;
	}
	else
	{
		ext_format = D3DX_SF_A8R8G8B8;
		int_format = D3DX_SF_A8R8G8B8;
	}

	DWORD mipmap = texdata->mipmap ? 0 : D3DX_TEXTURE_NOMIPMAP;
	DWORD nummips= texdata->mipmap ? texdata->mipmaps : 0;
	DWORD width = texdata->width;
	DWORD height = texdata->height;

	D3DXCreateTexture(m_pD3DDevice,
					  &mipmap,
					  &width,
					  &height,
					  &int_format,
					  NULL,
					  &mTexBins[bin].tex_surfs[num],
					  &nummips);



	if (texdata->mipmap)
	{
		for (int m=texdata->mipmaps-1; m>=0; m--)
		{
			D3DXLoadTextureFromMemory(m_pD3DDevice,
									  mTexBins[bin].tex_surfs[num],
									  texdata->mipmaps-1-m,
									  texdata->mipdata[m],
									  NULL,
									  ext_format,
									  D3DX_DEFAULT,
									  NULL,
									  D3DX_FT_LINEAR);
		}
	}

	else
	{
		D3DXLoadTextureFromMemory(m_pD3DDevice,
								  mTexBins[bin].tex_surfs[num],
								  0,
								  texdata->mipdata[texdata->mipmaps-1],
								  NULL,
								  ext_format,
								  D3DX_DEFAULT,
								  NULL,
								  D3DX_FT_LINEAR);
	}
}


/*
========
Matrix*
========
*/
void CRastD3DX::MatrixReset(void)
{
	m_matView->LoadIdentity();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}
void CRastD3DX::MatrixRotateX(float degrees)
{
	D3DXMATRIX mat;
	D3DXMatrixRotationX(&mat, degrees * PI/180);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}
void CRastD3DX::MatrixRotateY(float degrees)
{
	D3DXMATRIX mat;
	D3DXMatrixRotationY(&mat, degrees * PI/180);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}
void CRastD3DX::MatrixRotateZ(float degrees)
{
	D3DXMATRIX mat;
	D3DXMatrixRotationZ(&mat, degrees * PI/180);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}


void CRastD3DX::MatrixTranslate(vector_t &dir)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, -dir.x, -dir.z, dir.y);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}


void CRastD3DX::MatrixScale(vector_t &factors)
{
}



void CRastD3DX::MatrixPush(void)
{
	m_matView->Push();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}

void CRastD3DX::MatrixPop(void)
{
	m_matView->Pop();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}



/*
========
Poly*
========
*/
void CRastD3DX::PolyStart(EPolyType type)
{
	mNumVerts = 0;
	mType = type;
}


/*
========
PolyEnd
========
*/
void CRastD3DX::PolyEnd(void)
{
	int i;
	D3DLVERTEX *vptr;

	switch (mType)
	{
	case VRAST_TRIANGLE_FAN:
		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, &mVerts, mNumVerts, 0 );
		break;

	case VRAST_TRIANGLE_STRIP:
		m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, D3DFVF_LVERTEX, &mVerts, mNumVerts, 0 );
		break;

	case VRAST_QUADS:
		vptr = mVerts;
		for (i=0; i<mNumVerts; i+=4)
		{
			vptr = &mVerts[i];
			m_pD3DDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, D3DFVF_LVERTEX, vptr, 4, 0);
		}
		break;
	}

	mNumVerts = 0;
}

void CRastD3DX::PolyVertexf(vector_t &vert)
{
	mVerts[mNumVerts] = D3DLVERTEX( D3DVECTOR(vert.x, vert.z, -vert.y),
									RGBA_MAKE((int)(mColor.x*255), (int)(mColor.y*255), (int)(mColor.z*255), (int)(mAlpha*255)),
									RGBA_MAKE(255, 255, 255, 255),
									mTexCoords[0], mTexCoords[1]);
	mNumVerts++;
}
void CRastD3DX::PolyVertexi(int x, int y)
{
	mVerts[mNumVerts] = D3DLVERTEX( D3DVECTOR(x, y, 0),
									RGBA_MAKE((int)(mColor.x*255), (int)(mColor.y*255), (int)(mColor.z*255), (int)(mAlpha*255)),
									RGBA_MAKE(255, 255, 255, 255),
									mTexCoords[0], mTexCoords[1]);
	mNumVerts++;
}
void CRastD3DX::PolyTexCoord(float s, float t)
{
	mTexCoords[0] = s;
	mTexCoords[1] = t;
}
void CRastD3DX::PolyColor3f(float r, float g, float b)
{
	VectorSet(&mColor, r, g, b);
}
void CRastD3DX::PolyColor4f(float r, float g, float b, float a)
{
	VectorSet(&mColor, r, g, b);
	mAlpha = a;
}


/*
========
ClearBuffers
========
*/
void CRastD3DX::ClearBuffers(int buffers)
{
	HRESULT hr = m_pD3DDevice->BeginScene();
	if (FAILED(hr))
	{
		mhError = hr;
		ReportErrors();
	}

//	glClearColor(0, 0, 1, 1);
	int b = 0;

	if (buffers & VRAST_COLOR_BUFFER)
		b |= D3DCLEAR_TARGET;
	if (buffers & VRAST_DEPTH_BUFFER)
		b |= D3DCLEAR_ZBUFFER;
	if (buffers & VRAST_STENCIL_BUFFER)
		b |= D3DCLEAR_STENCIL ;

	m_pD3DX->Clear(b);

}


/*
========
ProjectionMode
========
*/
void CRastD3DX::ProjectionMode(EProjectionMode mode)
{
	float x;
	float z;

	float r = g_rInfo.width;
	float t = g_rInfo.height;

	switch (mode)
	{
	case VRAST_PERSPECTIVE:
		// FIXME - access fov here
		x = (float) tan(90.0f*(PI/180) * 0.5f);
		z = x * 0.75f;						// always render in a 3:4 aspect ratio
		D3DXMatrixPerspectiveOffCenter(&m_matProjection, -x, x, -z, z, 1, 10000);
		break;

	case VRAST_ORTHO:
		D3DXMatrixOrthoOffCenter(&m_matProjection, 0, g_rInfo.width, 0, g_rInfo.height, -1, 1);

		break;
	}

	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)m_matProjection);
}


void CRastD3DX::ReportErrors(void)
{
	if (mhError == S_OK)
		return;

	if(D3DXERR_CAPSNOTSUPPORTED == mhError) 
	{
		ComPrintf("D3DXERR_CAPSNOTSUPPORTED\n\n"
					"This device lacks required capabilities.  "
					"Try using the reference rasterizer.");
	}
	else
	{
		char errStr[256];
		D3DXGetErrorString(mhError, 256, errStr);
		ComPrintf("d3dx error - %s\n", errStr);
	}

	mhError = S_OK;
}

/*
========
ClearBuffers
========
*/
void CRastD3DX::FrameEnd(void)
{
	Sleep(1);
	m_pD3DDevice->EndScene();
	// Update frame
  
	HRESULT hr = m_pD3DX->UpdateFrame(0);
	if (FAILED(hr))
		mhError = hr;


    if(DDERR_SURFACELOST == hr || DDERR_SURFACEBUSY == hr)
    {
        hr = m_pDD->TestCooperativeLevel();

        if(SUCCEEDED(hr))
            RestoreSurfaces();
    }
}


void CRastD3DX::ScreenShot(unsigned char *dest)
{
}


/*
========
SetVidSynch - assumes that it can be done
========
*/
void CRastD3DX::SetVidSynch(int v)
{
}