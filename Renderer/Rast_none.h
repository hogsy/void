#ifndef	RAST_NONE_H
#define RAST_NONE_H

#include "Rast_main.h"

class CRastNone : public CRasterizer
{
public:

	CRastNone();
	~CRastNone();

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
	void TextureClamp(bool clamp) {}

	void MatrixReset(void);
	void MatrixRotateX(float degrees);
	void MatrixRotateY(float degrees);
	void MatrixRotateZ(float degrees);
	void MatrixTranslate(float x, float y, float z);
	void MatrixScale(vector_t &factors);
	void MatrixPush(void);
	void MatrixPop(void);

	void PolyDraw(void) {};

	void ClearBuffers(int buffers);
	void ProjectionMode(EProjectionMode mode);
	void ReportErrors(void);
	void FrameEnd(void);
	void ScreenShot(unsigned char *dest);
	void SetVidSynch(int v);

	void SetFocus(void);

private:

};


#endif
