#ifndef	RAST_NONE_H
#define RAST_NONE_H

#include "Rasterizer.h"

class CRastNone : public I_Rasterizer
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

	int  TextureBinInit(int num);
	int  TextureCount(int bin) { return 0; }
	void TextureBinDestroy(int bin);
	void TextureSet(int bin, int texnum);
	void TextureLoad(int bin, int num, const tex_load_t *texdata);

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

};


#endif
