
#include "Standard.h"
#include "Rast_none.h"



/*
=======================================
Constructor 
=======================================
*/
CRastNone::CRastNone()
{
}


/*
==========================================
Destructor
==========================================
*/
CRastNone::~CRastNone()
{
}


/*
==========================================
Init
==========================================
*/
bool CRastNone::Init()
{
	return true;
}


/*
==========================================
Shutdown
==========================================
*/
bool CRastNone::Shutdown()
{
	return true;
}


/*
==========================================
Update Default window coords
==========================================
*/
void CRastNone::SetWindowCoords(int wndX, int wndY)
{
}

/*
==========================================
Resize the Window
==========================================
*/
void CRastNone::Resize()
{
}

/*
==========================================
Updates display settings
==========================================
*/
bool CRastNone::UpdateDisplaySettings(int width, int height, int bpp, bool fullscreen)
{
	return true;
}


/*
==========================================
SetFocus
==========================================
*/
void CRastNone::SetFocus()
{
}





//=========================================================================================================================
// Empty rasterizer class implementation
//=========================================================================================================================



void CRastNone::DepthFunc(EDepthFunc func)
{
}

void CRastNone::DepthWrite(bool write)
{
}

void CRastNone::BlendFunc(ESourceBlend src, EDestBlend dest)
{
}


/*
========
TextureBinInit
========
*/
int CRastNone::TextureBinInit(int num)
{
	return 0;
}


/*
========
TextureBinDestroy
========
*/
void CRastNone::TextureBinDestroy(int bin)
{
}


void CRastNone::TextureSet(int bin, int texnum)
{
}

void CRastNone::TextureLoad(int bin, int num, const tex_load_t *texdata)
{
}


/*
========
Matrix*
========
*/
void CRastNone::MatrixReset(void)
{
}
void CRastNone::MatrixRotateX(float degrees)
{
}
void CRastNone::MatrixRotateY(float degrees)
{
}
void CRastNone::MatrixRotateZ(float degrees)
{
}


void CRastNone::MatrixTranslate(vector_t &dir)
{
}


void CRastNone::MatrixScale(vector_t &factors)
{
}



void CRastNone::MatrixPush(void)
{
}

void CRastNone::MatrixPop(void)
{
}



/*
========
Poly*
========
*/
void CRastNone::PolyStart(EPolyType type)
{
}


/*
========
PolyEnd
========
*/
void CRastNone::PolyEnd(void)
{
}

void CRastNone::PolyVertexf(vector_t &vert)
{
}
void CRastNone::PolyVertexi(int x, int y)
{
}
void CRastNone::PolyTexCoord(float s, float t)
{
}
void CRastNone::PolyColor3f(float r, float g, float b)
{
}
void CRastNone::PolyColor4f(float r, float g, float b, float a)
{
}


/*
========
ClearBuffers
========
*/
void CRastNone::ClearBuffers(int buffers)
{
}


/*
========
ProjectionMode
========
*/
void CRastNone::ProjectionMode(EProjectionMode mode)
{
}


void CRastNone::ReportErrors(void)
{
}


/*
========
ClearBuffers
========
*/
void CRastNone::FrameEnd(void)
{
}


void CRastNone::ScreenShot(unsigned char *dest)
{
}


/*
========
SetVidSynch
========
*/
void CRastNone::SetVidSynch(int v)
{
}