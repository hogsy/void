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
Texture*
========
*/
void CRastNone::TextureSet(hTexture texnum)
{
}

void CRastNone::TextureLoad(hTexture index, const TextureData &texdata)
{
}

void CRastNone::TextureUnLoad(hTexture index)
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


void CRastNone::MatrixTranslate(float x, float y, float z)
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