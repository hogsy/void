#ifndef TEX_IMAGE_READER_H
#define TEX_IMAGE_READER_H

#include "I_file.h"

//======================================================================================

enum EImageFileFormat
{
	FORMAT_NONE,
	FORMAT_TGA,
	FORMAT_PCX,
	FORMAT_JPG
};


const int mipdatasizes[MAX_MIPMAPS] = 
{
	1*1*4,
	2*2*4,
	4*4*4,
	8*8*4,
	16*16*4,
	32*32*4,
	64*64*4,
	128*128*4,
	256*256*4,
	512*512*4,
	1024*1024*4
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
/*
	const int & GetHeight() const { return height;}		//Height
	const int & GetWidth()  const { return width; }		//Width
	const int & GetNumMips()const { return miplevels; }
	byte ** GetMipData()   { return &mipmapdata[0]; }
	const EImageFormat & GetFormat()  const { return format; }	//BYTES per pixel
*/

	bool Read(const char * file, TextureData &imgData);
	bool ReadLightMap(unsigned char **stream, TextureData &imgData);	
	bool DefaultTexture(TextureData &imgData);

	void ImageReduce(int m);			//Reduce image data by 2x. used to create mipmaps
	void FreeMipData();

protected:

	byte *	mipmapdata[MAX_MIPMAPS];
	int		width,
			height;
	int		h, w;	// for mipmap creation
	int		miplevels;

	EImageFormat format;
	CFileBuffer	 m_fileReader;

	// util functions
	void GetMipCount();
	void ConfirmMipData();		//allocate mem if we need to
	void ColorKey(unsigned char *data);			//Only works for RGBA images

	//==========================================
	//Supported formats

	bool Read_PCX(TextureData &imgData);
	bool Read_TGA(TextureData &imgData);
	bool Read_JPG(TextureData &imgData);
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