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


const int mipdatasizes[MAX_MIPMAPS] = 
{
	1*1*4*4,
	2*2*4*4,
	4*4*4*4,
	8*8*4*4,
	16*16*4*4,
	32*32*4*4,
	64*64*4*4,
	128*128*4*4,
	256*256*4*4,
	512*512*4*4,
	1024*1024*4*4
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
	const tex_load_t * GetData()   const { return &loaddata; }
	const byte * GetImageData()    { return mipmapdata[GetMipCount()-1]; }
	const EImageFormat & GetFormat()  const { return format; }	//BYTES per pixel

	bool Read(const char * file);
	
//FIXME, should this really be there ?
	//Read lightmap textures from world file
	bool ReadLightMap(unsigned char **stream);	

	int  GetMipCount();
	int  ConfirmMipData();		//allocate mem if we need to
	void ImageReduce(int m);			//Reduce image data by 2x. used to create mipmaps
	
//	void Reset();
	bool DefaultTexture();
	void ColorKey(unsigned char *data);			//Only works for RGBA images

	//Hack until a full featured memory manager is done
	//static buffer is locked for all i/o when loading map textures for speed
//	void LockBuffer(int size);
//	void LockMipMapBuffer(int size);
//	void UnlockBuffer();
//	void UnlockMipMapBuffer();

protected:

//	int		buffersize;
//	int		mipbuffersize;
	
	byte *	mipmapdata[MAX_MIPMAPS];;
	tex_load_t	loaddata;
//	byte *	data;
	
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