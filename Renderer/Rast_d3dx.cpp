#include "Standard.h"
#include "Rast_d3dx.h"

#include "Ren_exp.h"
extern CRenExp		  * g_pRenExp;



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
	mVidSynch = false;
}


/*
==========================================
Destructor
==========================================
*/
CRastD3DX::~CRastD3DX()
{
	if (m_bInitialized)
		Shutdown();
}


/*
==========================================
Init
==========================================
*/
bool CRastD3DX::Init()
{

	RECT wrect;
	wrect.left = g_varWndX->ival;
	wrect.top = g_varWndY->ival;
	wrect.right = g_varWndX->ival + g_rInfo.width;
	wrect.bottom= g_varWndY->ival + g_rInfo.height;
	
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

	if(g_pRenExp->m_varFull->bval)
	{
		hr = D3DXCreateContextEx(D3DX_DEFAULT, D3DX_CONTEXT_FULLSCREEN, g_rInfo.hWnd, NULL, g_pRenExp->m_varBpp->ival, 0,
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


	m_pD3DDevice->SetTextureStageState(0, D3DTSS_MINFILTER, D3DTFG_LINEAR);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTFG_LINEAR);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTFP_POINT);

	m_pD3DDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
	m_pD3DDevice->SetTextureStageState(1, D3DTSS_ALPHAOP, D3DTOP_DISABLE);


	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_CURRENT );
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_MODULATE);

	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_CURRENT );
	m_pD3DDevice->SetTextureStageState(0, D3DTSS_ALPHAOP, D3DTOP_MODULATE);

/*
	DDCAPS caps;
	caps.dwSize = sizeof(DDCAPS);
	m_pDD->GetCaps(&caps, NULL);
	bool vsynch = (caps.dwCaps2 & DDCAPS2_FLIPNOVSYNC) ? true : false;
*/

	m_bInitialized = true;
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

 	m_bInitialized = false;
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
}


/*
==========================================
Update Default window coords
==========================================
*/
void CRastD3DX::SetWindowCoords(int wndX, int wndY)
{
	//Dont bother with initial co-ords
	if(!m_bInitialized || g_pRenExp->m_varFull->bval)
		return;

	g_varWndX->Set(wndX);
	g_varWndY->Set(wndY);
}

/*
==========================================
Resize the Window
==========================================
*/
void CRastD3DX::Resize()
{
	if (!m_bInitialized)
		return;

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
	if (!m_bInitialized)
		return false;

	Shutdown();

	// record old stats
	bool oldfull = g_pRenExp->m_varFull->bval;
	uint oldwidth= g_rInfo.width;
	uint oldheight= g_rInfo.height;
	uint oldbpp	  = g_pRenExp->m_varBpp->ival;

	g_pRenExp->m_varBpp->ForceSet(bpp);
	g_rInfo.width	= width;
	g_rInfo.height	= height;

	if (fullscreen)
		g_pRenExp->m_varFull->ForceSet("1");
	else
		g_pRenExp->m_varFull->ForceSet("0");

	if (!Init())
	{
		ComPrintf("D3DX::UpdateDisplaySettings: Unable to change to new settings\n");

		// switch everythign back;
		if (oldfull)
			g_pRenExp->m_varFull->ForceSet("1");
		else
			g_pRenExp->m_varFull->ForceSet("0");

		g_pRenExp->m_varBpp->ForceSet((int)oldbpp);
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
	mCurDepthFunc = func;

	switch (func)
	{
	case VRAST_DEPTH_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, FALSE);
		return;

	case VRAST_DEPTH_ALWAYS:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,   D3DCMP_ALWAYS);
		return;

	case VRAST_DEPTH_LEQUAL:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,   D3DCMP_LESSEQUAL);
		return;

	case VRAST_DEPTH_EQUAL:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZENABLE, D3DZB_USEW);
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZFUNC,   D3DCMP_EQUAL);
		return;
	}
}

void CRastD3DX::DepthWrite(bool write)
{
	mCurDepthWrite = write;

	if (write)
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, TRUE);
	else
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ZWRITEENABLE, FALSE);
}

void CRastD3DX::BlendFunc(ESourceBlend src, EDestBlend dest)
{
	int source = 0;
	int destination = 0;

	mCurSrcBlend = src;
	mCurDstBlend = dest;

	switch (src)
	{
	case VRAST_SRC_BLEND_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		return;
	case VRAST_SRC_BLEND_ONE:
		source = D3DBLEND_ONE;
		break;
	case VRAST_SRC_BLEND_ZERO:
		source = D3DBLEND_ZERO;
		break;
	case VRAST_SRC_BLEND_SRC_ALPHA:
		source = D3DBLEND_SRCALPHA;
		break;
	case VRAST_SRC_BLEND_DST_COLOR:
		source = D3DBLEND_DESTCOLOR;
		break;
	case VRAST_SRC_BLEND_INV_DST_COLOR:
		source = D3DBLEND_INVDESTCOLOR;
		break;
	case VRAST_SRC_BLEND_INV_SRC_ALPHA:
		source = D3DBLEND_INVSRCALPHA;
		break;
	}

	switch (dest)
	{
	case VRAST_DST_BLEND_NONE:
		m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, FALSE);
		return;
	case VRAST_DST_BLEND_SRC_COLOR:
		destination = D3DBLEND_SRCCOLOR;
		break;
	case VRAST_DST_BLEND_INV_SRC_COLOR:
		destination = D3DBLEND_INVSRCCOLOR;
		break;
	case VRAST_DST_BLEND_ONE:
		destination = D3DBLEND_ONE;
		break;
	case VRAST_DST_BLEND_ZERO:
		destination = D3DBLEND_ZERO;
		break;
	case VRAST_DST_BLEND_SRC_ALPHA:
		destination = D3DBLEND_SRCALPHA;
		break;
	case VRAST_DST_BLEND_INV_SRC_ALPHA:
		destination = D3DBLEND_INVSRCALPHA;
		break;
	}

	m_pD3DDevice->SetRenderState(D3DRENDERSTATE_ALPHABLENDENABLE, TRUE);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_SRCBLEND, source);
    m_pD3DDevice->SetRenderState(D3DRENDERSTATE_DESTBLEND, destination);
}



void CRastD3DX::TextureClamp(bool clamp)
{
	if (clamp)
	{
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
	}
	else
	{
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_WRAP);
		m_pD3DDevice->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_WRAP);
	}
}



void CRastD3DX::TextureSet(hTexture texnum)
{
	if (texnum <= -1)
		return;
	m_pD3DDevice->SetTexture(0, mTexSurfs[texnum]);
}

void CRastD3DX::TextureLoad(hTexture index, const TextureData &texdata)
{
	D3DX_SURFACEFORMAT ext_format, int_format;

	int bpp = texdata.format == IMG_RGB ? 3 : 4;

	int w = texdata.width;
	int h = texdata.height;

	// have to swap all blue and red values
	for (int m=texdata.numMipMaps-1; m>=0; m--)
	{
		for (int r=0; r<h; r++)
		{
			for (int c=0; c<w; c++)
			{
				char tmp;
				tmp = texdata.data[m][r*w*bpp + c*bpp + 0];
				texdata.data[m][r*w*bpp + c*bpp + 0] = texdata.data[m][r*w*bpp + c*bpp + 2];
				texdata.data[m][r*w*bpp + c*bpp + 2] = tmp;
			}
		}
		h /= 2;
		w /= 2;
		if (!h) h = 1;
		if (!w) w = 1;
	}

	if (texdata.format == IMG_RGB)
	{
		ext_format = D3DX_SF_R8G8B8;
		int_format = g_var32BitTextures->bval ? D3DX_SF_R8G8B8 : D3DX_SF_R5G6B5;
	}
	else
	{
		ext_format = D3DX_SF_A8R8G8B8;
		int_format = g_var32BitTextures->bval ? D3DX_SF_A8R8G8B8 : D3DX_SF_A4R4G4B4;
	}

	DWORD mipmap = texdata.bMipMaps ? 0 : D3DX_TEXTURE_NOMIPMAP;
	DWORD nummips= texdata.bMipMaps ? texdata.numMipMaps : 0;
	DWORD width =  texdata.width;
	DWORD height = texdata.height;

	D3DXCreateTexture(m_pD3DDevice,
					  &mipmap,
					  &width,
					  &height,
					  &int_format,
					  NULL,
					  &mTexSurfs[index],
					  &nummips);



	if (texdata.bMipMaps)
	{
		for (int m=texdata.numMipMaps-1; m>=0; m--)
		{
			D3DXLoadTextureFromMemory(m_pD3DDevice,
									  mTexSurfs[index],
									  texdata.numMipMaps-1-m,
									  texdata.data[m],
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
								  mTexSurfs[index],
								  0,
								  texdata.data[texdata.numMipMaps-1],
								  NULL,
								  ext_format,
								  D3DX_DEFAULT,
								  NULL,
								  D3DX_FT_LINEAR);
	}
}

void CRastD3DX::TextureUnLoad(hTexture index)
{
	mTexSurfs[index]->Release();
}

/*
========
Matrix*
========
*/
void CRastD3DX::MatrixReset(void)
{
	Flush();
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


void CRastD3DX::MatrixTranslate(float x, float y, float z)
{
	D3DXMATRIX mat;
	D3DXMatrixTranslation(&mat, x, y, z);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}


void CRastD3DX::MatrixScale(vector_t &factors)
{
	D3DXMATRIX mat;
	D3DXMatrixScaling(&mat, factors.x, factors.y, factors.z);
	D3DXMatrixMultiply(m_matView->GetTop(), &mat, m_matView->GetTop());
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}



void CRastD3DX::MatrixPush(void)
{
	Flush();
	m_matView->Push();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}

void CRastD3DX::MatrixPop(void)
{
	Flush();
	m_matView->Pop();
	m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_VIEW, (D3DMATRIX *)m_matView->GetTop());
}



/*
========
PolyEnd
========
*/
void CRastD3DX::PolyDraw(void)
{
	// draw
	m_pD3DDevice->DrawIndexedPrimitive(D3DPT_TRIANGLELIST,
		D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1 | D3DFVF_TEXCOORDSIZE2(0),
		mVerts,
		mNumElements,
		mIndices,
		mNumIndices,
		0);
}


/*
========
ClearBuffers
========
*/
void CRastD3DX::ClearBuffers(int buffers)
{
	mTrisDrawn = 0;

	HRESULT hr = m_pD3DDevice->BeginScene();
	if (FAILED(hr))
	{
		mhError = hr;
		ReportErrors();
	}

//	glClearColor(0, 0, 0, 1);
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

	D3DXMATRIX mat;


	switch (mode)
	{
	case VRAST_PERSPECTIVE:
		x = (float) tan(g_varFov->ival*(PI/180) * 0.5f);
		z = x * 0.75f;						// always render in a 3:4 aspect ratio
		D3DXMatrixPerspectiveOffCenter(&m_matProjection, -x, x, -z, z, 1, 10000);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)m_matProjection);

		// switch to +y = north, +z = up coordinate system
		// have to write this out to add it to the projection matrix not the modelview

//		MatrixRotateX(-90);
		D3DXMatrixRotationX(&mat, -90.0f * PI/180);
		D3DXMatrixMultiply(&m_matProjection, &mat, &m_matProjection);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)m_matProjection);

//		MatrixRotateZ(90);
		D3DXMatrixRotationZ(&mat, 90.0f * PI/180);
		D3DXMatrixMultiply(&m_matProjection, &mat, &m_matProjection);
		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)m_matProjection);

		return;

	case VRAST_ORTHO:
		if (g_varD3DXShift->ival == 0)
			D3DXMatrixOrthoOffCenter(&m_matProjection, 0, g_rInfo.width, 0, g_rInfo.height, -1, 1);
		else
			D3DXMatrixOrthoOffCenter(&m_matProjection,	
									 1.0f/g_varD3DXShift->ival, 
									 g_rInfo.width + 1.0f/g_varD3DXShift->ival,
									-1.0f/g_varD3DXShift->ival, 
									 g_rInfo.height- 1.0f/g_varD3DXShift->ival, -1, 1);

		m_pD3DDevice->SetTransform(D3DTRANSFORMSTATE_PROJECTION, (D3DMATRIX *)m_matProjection);

		return;
	}
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
	// make sure everything's been drawn
	Flush();

	Sleep(1);
	m_pD3DDevice->EndScene();
	// Update frame

	HRESULT hr = m_pD3DX->UpdateFrame(mVidSynch ? 0 : D3DX_UPDATE_NOVSYNC);
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
/*
	LPDIRECTDRAWSURFACE7 primary = m_pD3DX->GetPrimary();
	if (primary)
	{
		primary->GetPrivateData(NULL, dest, g_rInfo.width*g_rInfo.height*3);


	}
*/
}


/*
========
SetVidSynch - assumes that it can be done
========
*/
void CRastD3DX::SetVidSynch(int v)
{
	mVidSynch = v ? true : false;
}