#ifndef _TEX_IMAGE_
#define _TEX_IMAGE_

#include "Standard.h"


void	ImageReduce32(byte *dest, byte *src, int nwidth, int nheight);
void	ImageReduce24(byte *dest, byte *src, int nwidth, int nheight);
byte*   ImageConvert(byte *src, int format, int width, int height);

/*
==========================================
Image File Class
==========================================
*/

class CImage
{
public:
	enum EImageFormat
	{
		FORMAT_NONE,
		FORMAT_TGA,
		FORMAT_PCX
	};

	byte *			data;
	int				width,
					height;
	int				type;
	EImageFormat	format;

	static  char m_texturepath[MAX_PATH];

	CImage();
	~CImage();

	//This will have to be redone once gamecontent files are setup
	bool	Read(const char *file);			//Read texture from path
	bool	Read(unsigned char **stream);	//Read lightmap textures from world file
	
	void	Write(const char *name, EImageFormat iformat=FORMAT_NONE);

	void    ImageReduce();			//Reduce image data by 2, used for mip maps

	bool	SnapShot();				//Fills itself with data on screen
	void	Reset();
	bool	DefaultTexture();
	void    ColorKey();

	//Temporary functions to speed up loading textures
	//Wont need them once the game filesystem is functional
	void	AllocFileBuffer();
	void	FreeFileBuffer();

private:

	static  byte *m_filebuffer;

	bool	Read_PCX(const byte *stream);
	bool	Read_TGA(const byte *stream);

	void	Write_TGA( FILE *fp);
	void	Write_PCX( FILE *fp);

};

#endif