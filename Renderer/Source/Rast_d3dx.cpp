
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
/*
	int source = 0;
	int destination = 0;

	switch (src)
	{
	case VRAST_SRC_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_SRC_BLEND_ZERO:
		source = GL_ZERO;
		break;
	case VRAST_SRC_BLEND_SRC_ALPHA:
		source = GL_SRC_ALPHA;
		break;
	}

	switch (dest)
	{
	case VRAST_DEST_BLEND_NONE:
		glDisable(GL_BLEND);
		return;
	case VRAST_DEST_BLEND_SRC_COLOR:
		destination = GL_SRC_COLOR;
		break;
	case VRAST_DEST_BLEND_ONE_MINUS_SRC_ALPHA:
		destination = GL_ONE_MINUS_SRC_ALPHA;
		break;
	}

	glEnable(GL_BLEND);
	glBlendFunc(source, destination);
*/
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
/*			mTexBins[i].glnames = new GLuint[num];
			if (!mTexBins[i].glnames)
				FError("not enough mem for gl names");

			glEnable(GL_TEXTURE_2D);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
			glGenTextures(mTexBins[i].num, mTexBins[i].glnames);
*/			return i;
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
/*
	glDeleteTextures(mTexBins[bin].num, mTexBins[bin].glnames);
	delete mTexBins[bin].glnames;
	mTexBins[bin].glnames = NULL;
*/	
	mTexBins[bin].num = -1;
}



void CRastD3DX::TextureSet(int bin, int texnum)
{
/*

	glBindTexture(GL_TEXTURE_2D, mTexBins[bin].glnames[texnum]);
*/
}

void CRastD3DX::TextureLoad(int bin, int num, const tex_load_t *texdata)
{
/*
	glBindTexture(GL_TEXTURE_2D, mTexBins[bin].glnames[num]);

	// clamping
	if (texdata->clamp)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	}

	// mipmapping
	if (texdata->mipmap)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	}
	else
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	}


	int ext_format, int_format;
	if (texdata->format == IMG_RGB)
	{
		ext_format = GL_RGB;
		int_format = GL_RGB8;
	}
	else
	{
		ext_format = GL_RGBA;
		int_format = GL_RGBA8;
	}

	int w = texdata->width;
	int h = texdata->height;

	if (texdata->mipmap)
	{
		for (int m=texdata->mipmaps-1; m>=0; m--)
		{
			glTexImage2D(GL_TEXTURE_2D,
					 texdata->mipmaps - m - 1,
					 GL_RGB, //int_format,
					 w,
					 h,
					 0,
					 GL_RGB, //ext_format,
					 GL_UNSIGNED_BYTE,
					 texdata->mipdata[m]);

			w /= 2;
			h /= 2;
			if (w==0) w=1;
			if (h==0) h=1;
		}
	}

	else
	{
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 int_format,
					 w,
					 h,
					 0,
					 ext_format,
					 GL_UNSIGNED_BYTE,
					 texdata->mipdata[texdata->mipmaps - 1]);
	}
*/
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
									rand(),
									0,
									mTexCoords[0], mTexCoords[1]);
	mNumVerts++;
}
void CRastD3DX::PolyVertexi(int x, int y)
{
	mVerts[mNumVerts] = D3DLVERTEX( D3DVECTOR(x, y, 0),
									rand(),
									0,
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
	if (FAILED(m_pD3DDevice->BeginScene()))
		ComPrintf("failed to begin d3dx scene\n");

//	glClearColor(0, 0, 1, 1);
	int b = 0;

	if (buffers & VRAST_COLOR_BUFFER)
		b |= D3DCLEAR_TARGET;
	if (buffers & VRAST_DEPTH_BUFFER)
		b |= D3DCLEAR_ZBUFFER;
	if (buffers & VRAST_STENCIL_BUFFER)
		b |= D3DCLEAR_STENCIL ;

	if (FAILED(m_pD3DDevice->BeginScene()))
		ComPrintf("failed to begin d3dx scene\n");
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
/*
	D3DMATRIX mat;
	mat._11 = 2.0f / (r-0);
	mat._21 = 0;
	mat._31 = 0;
	mat._41 = 0;
	mat._12 = 0;
	mat._22 = 2.0f / (t-0);
	mat._32 = 0;
	mat._42 = 0;
	mat._13 = 0;
	mat._23 = 0;
	mat._33 = -2.0f / (1- -1);
	mat._43 = 0;
	mat._14 = (r+0) / (r-0);
	mat._24 = (t+0) / (t-0);
	mat._34 = (1+-1) / (1 - -1);
	mat._44 = 1;
*/
/*
	mat._11 = 2.0f / (r-0);
	mat._12 = 0;
	mat._13 = 0;
	mat._14 = 0;
	mat._21 = 0;
	mat._22 = 2.0f / (t-0);
	mat._23 = 0;
	mat._24 = 0;
	mat._31 = 0;
	mat._32 = 0;
	mat._33 = -2.0f / (1- -1);
	mat._34 = 0;
	mat._41 = (r+0) / (r-0);
	mat._42 = (t+0) / (t-0);
	mat._43 = (1+-1) / (1 - -1);
	mat._44 = 1;
*/

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

/*
    if(DDERR_SURFACELOST == hr || DDERR_SURFACEBUSY == hr)
    {
        hr = g_pDD->TestCooperativeLevel();

        if(SUCCEEDED(hr))
        {
            if(FAILED(hr = RestoreContext()))
                return hr;
        }
        else if(DDERR_WRONGMODE == hr)
        {
            if(FAILED(hr = ReleaseContext()))
                return hr;

            if(FAILED(hr = CreateContext()))
                return hr;
        }

    }
*/
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