#ifndef TEX_IMAGE_READER_H
#define TEX_IMAGE_READER_H

#include "I_file.h"

//==========================================================================
//==========================================================================

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


//==========================================================================
//==========================================================================

/*
==========================================
Singleton ImageReader Class
==========================================
*/
class CImageReader
{
public:

	static CImageReader & GetReader();

	~CImageReader();
	void Shutdown();

	bool Read(const char * file, TextureData &imgData);
	bool ReadLightMap(byte **stream, TextureData &imgData);	
	void DefaultTexture(TextureData &imgData);

	//Reduce image data by 2x. used to create mipmaps
	void ImageReduce(int m);	
	void FreeMipData();

private:

	//Hide constructor
	CImageReader();

	//Util functions
	void GetMipCount();
	void ConfirmMipData();		//allocate mem if we need to
	void ColorKey(byte *data);	//Only works for RGBA images

	//Supported formats
	bool Read_PCX(TextureData &imgData);
	bool Read_TGA(TextureData &imgData);
	bool Read_JPG(TextureData &imgData);

	byte *	m_mipmapdata[MAX_MIPMAPS];
	int		m_width,
			m_height;
	int		h, w;	// for mipmap creation
	int		m_miplevels;

	EImageFormat  m_format;
	I_FileReader *  m_pFile; //Reader;
};

/*
==========================================
ImageWriter
==========================================
*/

class CImageWriter
{
public:

	CImageWriter(int iwidth, 
				 int iheight, 
				 const byte * idata);
	~CImageWriter();

	void Write(const char *name, EImageFileFormat iformat);

protected:

	int		m_width, 
			m_height;
	const   byte *   m_pData;

	void	Write_TGA( FILE *fp);
	void	Write_PCX( FILE *fp);
};


#endif