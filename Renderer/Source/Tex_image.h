#ifndef _TEX_IMAGE_
#define _TEX_IMAGE_

#include "Standard.h"

enum EImageFormat
{
	FORMAT_NONE,
	FORMAT_TGA,
	FORMAT_PCX
};

/*
==========================================
ImageReader Class
==========================================
*/
class CImageReader
{
public:

	CImageReader();
	virtual ~CImageReader();

	int GetHeight() { return height;}
	int GetWidth()  { return width; }
	
	EImageFormat GetFormat(){ return format; }
	byte *		 GetData()  { return data; }

	void LockBuffer(uint size);
	void UnlockBuffer();

	void LockMipMapBuffer(uint size);
	void UnlockMipMapBuffer();

	bool Read(const char *file);				//Read texture from path
	bool ReadLightMap(unsigned char **stream);	//Read lightmap textures from world file

	int  GetMipCount();
	void ImageReduce();			//Reduce image data by 2, used for mip maps
	
	void Reset();
	bool DefaultTexture();
	void ColorKey();

	static void SetTextureDir(const char * dir);

protected:

	uint			buffersize;
	uint			mipbuffersize;
	
	byte *			mipmapdata;
	byte *			data;
	int				width,
					height;
	int				type;
	EImageFormat	format;

	bool	Read_PCX();
	bool	Read_TGA();

	CFileBuffer m_fileReader;

	static  char m_texturepath[MAX_PATH];
};

/*
==========================================
ImageWriter
==========================================
*/

class CImageWriter
{
public:

	CImageWriter(int iwidth, int iheight, 
				 const byte * idata);
	CImageWriter(CImageReader * pImage);

	virtual ~CImageWriter();

	void Write(const char *name, EImageFormat iformat=FORMAT_TGA);

protected:

	int		m_width, m_height;
	const   byte * m_pData;

	void	Write_TGA( FILE *fp);
	void	Write_PCX( FILE *fp);
};


//======================================================================================
//======================================================================================

void	ImageReduce32(byte *dest, byte *src, int nwidth, int nheight);
void	ImageReduce24(byte *dest, byte *src, int nwidth, int nheight);
byte*   ImageConvert(byte *src, int format, int width, int height);

#endif