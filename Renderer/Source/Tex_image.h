#ifndef _TEX_IMAGE_
#define _TEX_IMAGE_

#include "Standard.h"

//======================================================================================

enum EImageFormat
{
	FORMAT_NONE,
	FORMAT_TGA,
	FORMAT_PCX,
	FORMAT_JPG
};

//======================================================================================

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

	const int & GetHeight() const { return height;}		//Height
	const int & GetWidth()  const { return width; }		//Width
	const int & GetBpp()    const { return bpp; }		//BYTES per pixel
	
	EImageFormat GetFormat() const { return format; }
	const byte * GetData()   const { return data; }

	bool Read(const char *file);				//Read texture from path
	bool ReadLightMap(unsigned char **stream);	//Read lightmap textures from world file

	int  GetMipCount();
	void ImageReduce();			//Reduce image data by 2, used for mip maps
	
	void Reset();
	bool DefaultTexture();
	void ColorKey();

	//Hack until a full featured memory manager is done
	//static buffer is locked for all i/o when loading map textures for speed
	void LockBuffer(int size);
	void LockMipMapBuffer(int size);
	void UnlockBuffer();
	void UnlockMipMapBuffer();

protected:

	int		buffersize;
	int		mipbuffersize;
	
	byte *	mipmapdata;
	byte *	data;
	
	int		width,
			height;
	int		bpp;

	EImageFormat format;
	CFileBuffer	 m_fileReader;

	//==========================================
	//Supported formats

	bool Read_PCX();
	bool Read_TGA();
	bool Read_JPG();
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

#endif