#ifndef _TEX_IMAGE_
#define _TEX_IMAGE_

#include "Standard.h"
#include "I_file.h"

//======================================================================================

enum EImageFileFormat
{
	FORMAT_NONE,
	FORMAT_TGA,
	FORMAT_PCX,
	FORMAT_JPG
};

enum EImageFormat
{
	IMG_NONE = 0,
	IMG_ALPHA =1,
	IMG_RGB   =3,
	IMG_RGBA  =4
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
	const byte * GetData()   const { return data; }
	const EImageFormat & GetFormat()  const { return format; }	//BYTES per pixel

	bool Read(const char * file);
	
//FIXME, should this really be there ?
	//Read lightmap textures from world file
	bool ReadLightMap(unsigned char **stream);	

	int  GetMipCount();
	void ImageReduce();			//Reduce image data by 2x. used to create mipmaps
	
	void Reset();
	bool DefaultTexture();
	void ColorKey();			//Only works for RGBA images

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

	void Write(const char *name, EImageFileFormat iformat=FORMAT_TGA);

protected:

	int		m_width, m_height;
	const   byte * m_pData;

	void	Write_TGA( FILE *fp);
	void	Write_PCX( FILE *fp);
};


//======================================================================================
//======================================================================================

#endif