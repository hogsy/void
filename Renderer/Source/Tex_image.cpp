#include "Tex_image.h"



#define IMAGE_565	0
#define IMAGE_1555	1
#define IMAGE_4444	2

#define MAX_TEXTURESIZE		4195328

char  CImage::m_texturepath[MAX_PATH];
byte *CImage::m_filebuffer = 0;

/*
============
PCX_Texture 
============
*/
typedef struct PCX_header
{
	byte manufacturer;
	byte version;
	byte encoding;
	byte bits_per_pixel;
	WORD x, y;
	WORD width, height;
	WORD x_res, y_res;
	byte ega_palette[48];
	byte reserved;
	byte num_planes;
	WORD bytes_per_line;
	WORD palette_type;
	byte junk[58];
} PCX_header;


/*
typedef struct TGA_Header
{
	byte	id_length;
	byte	colormap_type;
	byte	image_type;
	byte	colormap_spec[5];
	byte	image_spec[10];
}TGA_header;

Image ID - Field 6 (variable):......................10
Color Map Data - Field 7 (variable):................10
Image Data - Field 8 (variable):....................10
Image Type                   Description

         0                No Image Data Included
         1                Uncompressed, Color-mapped Image
         2                Uncompressed, True-color Image
         3                Uncompressed, Black-and-white image
         9                Run-length encoded, Color-mapped Image
         10               Run-length encoded, True-color Image
         11               Run-length encoded, Black-and-white image
*/


/*
==========================================
Base Texture class
==========================================
*/
CImage::CImage()
{
	data = 0;
	width = height = 0;
	type = 0;
	format = FORMAT_NONE;
}

CImage::~CImage()		
{	Reset();
}

/*
==========================================
Pretty bad.
Sets up the transparent color
==========================================
*/
void CImage::ColorKey()
{
	if(!data)
		return;

	for (int p = 0; p < width * height * 4; p+=4)
	{
		if ((data[p  ] == 228) &&
			(data[p+1] == 41) &&
			(data[p+2] == 226))
			data[p+3] = 0;
	}
	type = IMAGE_1555;
}


/*
==========================================
Reset the image data
==========================================
*/
void CImage::Reset()
{
	if(data)
		free(data);
		//delete [] data;
	data = 0;
	type = 0;
	height = 0;
	width = 0;
	format = FORMAT_NONE;
}



/*
==========================================
Load a default image 
used as a replacement when we can't find a texture
==========================================
*/
bool CImage::DefaultTexture()
{
	width = 16;
	height = 64;

	// all raw pic data is 32 bits
//	data = new byte[(width * height * 4)];
	data = (byte *) MALLOC(width * height * 4);

	if (data == NULL) 
	{
		FError("CImage::DefaultTexture: No mem for pic data");
		return false;
	}

	for (unsigned int p = 0; p < width * height * 4; p++)
	{
		data[p] = rand();
		if (p%4 == 3)
			data[p] = 255;	// make sure it's full alpha
	}
	type = IMAGE_565;
	format = FORMAT_TGA;
	return true;
}


/*
==========================================
Read screen data into self
used to screenshots, 

GL dependent code shouldnt be here
==========================================
*/
bool CImage::SnapShot()
{
	if(data)
		Reset();

	width  = rInfo->width;
	height = rInfo->height;
	type   = 0;
//	data   = new byte[(width * height * 4)];
	data = (byte *) MALLOC(width * height * 4);

	if (data == NULL) 
	{
		Error("CImage::SnapShot:No mem for writing pcx");
		return false;
	}
	glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
	format = FORMAT_TGA;
	return true;
}



/*
==========================================
Read Image data from a stream
used for reading Lightmaps from the world file
==========================================
*/
bool CImage::Read(unsigned char **stream)
{
	// read the data from the stream
	width = **stream;
	(*stream)++;
	height = **stream;
	(*stream)++;

	if (!width || !height)
		return false;

	// all raw pic data is 32 bits
//	data = new byte[(width * height * 4)];	
	data = (byte *) MALLOC(width * height * 4);


	if (data == NULL) 
	{
		FError("CImage::DefaultTexture: No mem for pic data");
		return false;
	}

	for (unsigned int p = 0; p < width * height * 4; p++)
	{
		if (p%4 == 3)
			data[p] = 255;	//make sure it's full alpha
		else
		{
			data[p] = **stream;
			(*stream)++;
		}
	}
	type = IMAGE_565;
	format = FORMAT_TGA;
	return true;
}


/*
==========================================
Read a texture from the given path
==========================================
*/
bool CImage::Read(const char *file)
{
	if(data)
		Reset();

	EImageFormat fileformat;
	char filename[MAX_PATH];

	//default to a pcx
	sprintf(filename,"%s/%s.tga",m_texturepath,file);


/*	HANDLE hfile;
	hfile = CreateFile(filename,		//pointer to name of the file
					   GENERIC_READ,    //access (read-write) mode
					   0,               //DWORD dwShareMode,  // share mode
					   0,				//LPSECURITY_ATTRIBUTES lpSecurityAttributes,//pointer to security attributes
					   OPEN_EXISTING,	//how to create
					   FILE_ATTRIBUTE_NORMAL, // file attributes //FILE_FLAG_SEQUENTIAL_SCAN
					   0				//handle to file with attributes to 
					   );


	if(hfile == INVALID_HANDLE_VALUE)
	{
		sprintf(filename,"%s/%s.pcx",m_texturepath,file);
		hfile = CreateFile(filename,		//pointer to name of the file
				   GENERIC_READ,    //access (read-write) mode
				   0,               //DWORD dwShareMode,  // share mode
				   0,				//LPSECURITY_ATTRIBUTES lpSecurityAttributes,//pointer to security attributes
				   OPEN_EXISTING,	//how to create
				   FILE_ATTRIBUTE_NORMAL, // file attributes //FILE_FLAG_SEQUENTIAL_SCAN
				   0				//handle to file with attributes to 
				   );
		if(hfile == INVALID_HANDLE_VALUE)
		{
			ConPrint("CImage::Read: Couldnt open %s\n",filename);
			return false;
		}
		fileformat = FORMAT_PCX;
	}
	else
		fileformat = FORMAT_TGA;
 
	unsigned long fsize = GetFileSize(hfile,NULL);
	unsigned long fread = 0;

	if(fsize == 0xFFFFFFFF)
	{
		ConPrint("CImage::Read: Couldnt get size of %s\n",filename);
		fileformat = FORMAT_NONE;
		return false;
	}

	byte * imagedata;
	bool err=false;

	if(m_filebuffer)
		imagedata = m_filebuffer;
	else
	    imagedata = (unsigned char *)MALLOC(fsize);


	if(!ReadFile(hfile,          // handle of file to read
				imagedata,      // pointer to buffer that receives data
				fsize,			// number of bytes to read
				&fread,			// pointer to number of bytes read
				0))				// pointer to structure for data
	{
		ConPrint("CImage::Read: Couldnt read %s\n",filename);
		fileformat = FORMAT_NONE;
		return false;
	}

	if(fileformat == FORMAT_PCX)
		err = Read_PCX(imagedata);
	else if(fileformat == FORMAT_TGA)
		err = Read_TGA(imagedata);

	CloseHandle(hfile);
*/ 

	
	


	

	FILE  *fp =  ::fopen(filename,"rb");

	if(fp == NULL)
	{	
		//try a tga
		sprintf(filename,"%s/%s.pcx",m_texturepath,file);
		if((fp = fopen(filename,"rb")) == NULL)
		{
			ConPrint("CImage::Read: COuldnt open %s\n",filename);
			return false;
		}
		fileformat = FORMAT_PCX;
	}
	else
		fileformat = FORMAT_TGA;

	bool err=false;
	fseek(fp,0,SEEK_END);
	long filesize = ftell(fp);
	fseek(fp,0,SEEK_SET);
	
	if(filesize)
	{
		byte * imagedata;
		if(m_filebuffer)
			imagedata = m_filebuffer;
		else
		    imagedata = (unsigned char *)MALLOC(filesize);
		fread(imagedata,filesize,1,fp);

		if(fileformat == FORMAT_PCX)
			err = Read_PCX(imagedata);
		else if(fileformat == FORMAT_TGA)
			err = Read_TGA(imagedata);

		if(!m_filebuffer)
			free(imagedata);
	}
//	::fflush(fp);
	::fclose(fp);


	if(err==false)
		ConPrint("CImage::Read:Error Reading %s\n", filename);

	return err;
}


/*
==========================================
Read a PCX file from the given stream
==========================================
*/
bool CImage::Read_PCX(const byte * stream)
{
	PCX_header header;
	int pos = sizeof(header);

	memcpy((unsigned char *)&header,stream,pos);

	if((header.manufacturer != 10) || 
	   (header.version != 5) || 
	   (header.encoding != 1)|| 
	   (header.bits_per_pixel != 8))
	{
		ConPrint("CImage::Read_PCX:Bad texture file");
		return false;
	}

	byte ch;
	int number, color;
	int countx, county, i;

	width = header.width - header.x + 1;
	height = header.height - header.y + 1;

	// always store the pic in 32 bit rgba
//	data = new byte[(width*height*4)];
	data = (byte *) MALLOC(width * height * 4);

	if(data == NULL)
	{
		FError("CPCX_Texture::Read:Not enough memory for texture");
		return false;
	}

	memset(data, 0xFF, width * height * 4);

	// for every line

	for (county = 0; county < height; county++)
	{
		// decode this line for each r g b a
		for (color = 0; color < header.num_planes; color++)
		{
			for (countx = 0; countx < width; )
			{
				//ch = getc(fp);
				ch = stream[pos++];

				// Verify if the uppers two bits are sets
				if ((ch >= 192) && (ch <= 255))
				{
					number = ch - 192;
					//ch = getc(fp);
					ch = stream[pos++];
				}
				else
					number = 1;

				for (i = 0; i < number; i++)
				{
					data[(county * 4 * (width)) + (4 * countx) + color] = ch;
					countx++;
				}
			}
		}
	}

	// de-paletteify if we have to
	if (header.num_planes == 1)
	{
		byte palette[768]; // r,g,b 256 times
		int index;

		//fread(palette, 1, 1, fp);
		palette[0] = stream[pos];

		if (palette[0] != 0x0c)
		{
			ConPrint("CPCX_Texture::Read:palette not found!");
			return false;
		}

		//fread(palette, 768, 1, fp);
		memcpy((unsigned char *)palette,stream+pos,768);

		for (county = 0; county < height; county++)
		{
			for (countx = 0; countx < width; countx++)
			{
				index = data[(county*4*(width)) + (4*countx)];

				data[(county*4*(width)) + (4*countx)    ] = palette[index*3  ];
				data[(county*4*(width)) + (4*countx) + 1] = palette[index*3+1];
				data[(county*4*(width)) + (4*countx) + 2] = palette[index*3+2];
			}
		}
	}

	type = IMAGE_565;
	format=FORMAT_PCX;
	return true;
}


/*
==========================================
Read a TGA file from the given stream
==========================================
*/
bool CImage::Read_TGA(const byte *imagedata)
{
	int pos = 12;
	
	//UG
	width   = imagedata[pos++];
	width  |= imagedata[pos++] << 8;
	height  = imagedata[pos++];
	height |= imagedata[pos++] << 8;

	int comp = imagedata[pos++] / 8;

	imagedata[pos++];	// alpha/no alpha?  we already know that


	// always store the pic in 32 bit rgba
//	data = new byte[(width * height * 4)];
	data = (byte *) MALLOC(width * height * 4);


	if (!data)
	{	
		Error("CImage::Read_TGA:Not enough memory for texture");
		return false;
	}
	
	memset(data, 0xFF, width * height * 4);

	int w=0, h=0;

	for (h=height-1; h>=0; h--)
	{
		for (w=0; w<width; w++)
		{
			data[h*width*4 + w*4 + 2] = imagedata[pos++];
			data[h*width*4 + w*4 + 1] = imagedata[pos++];
			data[h*width*4 + w*4    ] = imagedata[pos++];
			if (comp == 4)
				data[h*width*4 + w*4 + 3] = imagedata[pos++];
		}
	}

	type = IMAGE_565;
	format=FORMAT_TGA;
	return true;

}

/*
==========================================
Write am Image to Disk
==========================================
*/

void CImage::Write(const char *name, EImageFormat iformat)
{
	if(!data)
		return;

	//no format specifed
	if(iformat == FORMAT_NONE)
		iformat = format;

	FILE *fp;
	fp = fopen(name,"wb");
	if(!fp)
	{
		ConPrint("CImage::Write:%s not found\n", name);
		return;
	}

	switch(format)
	{
	case FORMAT_PCX:
		Write_PCX(fp);
		break;
	case FORMAT_TGA:
		Write_TGA(fp);
		break;
	}
	fclose(fp);

	ConPrint("CImage::Write:Wrote %s\n", name);
}

/*
==========================================
Write a pcx file 
==========================================
*/
void CImage::Write_PCX( FILE *fp)
{
	int		i, j, p, length;
	PCX_header	pcx;
	byte		*pack;
	byte		*packdata;


	pcx.manufacturer = 0x0a;	// PCX id
	pcx.version = 5;			// 256 color
 	pcx.encoding = 1;		// uncompressed
	pcx.bits_per_pixel = 8;		// 256 color
	pcx.x      = 0;
	pcx.y      = 0;
	pcx.width  = (short)(width-1);
	pcx.height = (short)(height-1);
	pcx.x_res  = (short)width;
	pcx.y_res  = (short)height;
	memset (pcx.ega_palette,0,sizeof(pcx.ega_palette));
	pcx.num_planes = 3;
	pcx.bytes_per_line = (short)width;
	pcx.palette_type = 2;
	memset (pcx.junk, 0, sizeof(pcx.junk));

	//pack the image
	pack = (byte*) MALLOC(width * height * 4);

	if (pack == NULL) 
		FError("CImage:Write_PCX::mem for writing pcx");
	
	byte *pack2 = pack;

// gl reads from bottom to top, so write it backwards
	for (i = (height - 1); i >= 0; i--)
	{
		for (p = 0; p < pcx.num_planes; p++)
		{
			packdata = data + (i*width*4) + p;

			for (j = 0; j<width; j++)
			{
				if ( (*data & 0xc0) != 0xc0)
				{
					*pack++ = *packdata;
					packdata += 4;
				}

				else
				{
					*pack++ = 0xc1;
					*pack++ = *packdata;
					packdata += 4;
				}
			}
		}
	}

	length = pack - pack2;

	// write output file 
	// write the header
	fwrite(&pcx, sizeof(PCX_header), 1, fp);
	// write the rest
	fwrite(pack2, length, 1, fp);
	
	free(pack2);
}



/*
==========================================
Write TGA file
==========================================
*/
void CImage::Write_TGA( FILE *fp)
{
	// write output file 
	fputc(0, fp);
	fputc(0, fp);
	fputc(2, fp);
	
	fputc(0, fp);
	fputc(0, fp);
	fputc(0, fp);
	fputc(0, fp);
	fputc(0, fp);
	
	fputc(0, fp);
	fputc(0, fp);
	fputc(0, fp);
	fputc(0, fp);
	fputc((char)(width & 0x00ff), fp);
	fputc((char)((width & 0xff00) >> 8), fp);
	fputc((char)(height & 0x00ff), fp);
	fputc((char)((height & 0xff00) >> 8), fp);
//	if(alpha)
		fputc(32, fp);
//	else
//		fputc(24, fp);

//	if(pic->alpha)
		fputc(0x01, fp);
//	else
//		fputc(0x00, fp);

	int w, h, components = 4;
//	if (pic->alpha) components = 4;

	for (h = 0; h < height; h++)
	{
		for (w = 0; w < width; w++)
		{
			fwrite(&data[h*width*components + w*components + 2], 1, 1, fp);	// blue
			fwrite(&data[h*width*components + w*components + 1], 1, 1, fp);	// green
			fwrite(&data[h*width*components + w*components    ], 1, 1, fp);	// red
//			if (pic->alpha)
				fwrite(&data[h*width*components + w*components + 3], 1, 1, fp);	// alpha
		}
	}
}


/*
==========================================
Reduce the Size of the image by 2x
Used for mip maps
==========================================
*/

void CImage::ImageReduce()
{
	DWORD color;
	int r=0, c=0, s=0;
	
	width /=2;
	height /=2;

	int	sfactor = 1;
	int	tfactor = 1;

	if (!width)
	{
		width = 1;
		sfactor = 0;
	}

	if (!height)
	{
		height = 1;
		tfactor = 0;
	}

	int size = (width)*(height)*4;

//	byte *temp = new byte[size];
	byte * temp = (byte *) MALLOC(size);

	if(!temp)
	{
		ConPrint("OUT OF MEMORY !!!!!\n");
		return;
	}

	for (r = 0; r < height; r++)
	{
		for (c = 0; c < width; c++)
		{
			for (s = 0; s < 4; s++)
			{
				color =  data[ ((2*r)		  *width*8) + ((2*c)		 *4)+ s];
				color += data[ ((2*r)		  *width*8) + ((2*c)+sfactor)*4 + s];
				color += data[(((2*r)+tfactor)*width*8) + ((2*c)		 *4)+ s];
				color += data[(((2*r)+tfactor)*width*8) + ((2*c)+sfactor)*4 + s];
				color /= 4;

				temp[(r*width*4) + (c*4) + s] = (byte) color;
			}
		}
	}
//	delete [] data;
	free(data);
	data = temp;
}



/*
==========================================
Buffer alloc/free functions
Buffer is used to speed up texture reading
==========================================
*/
void CImage::AllocFileBuffer()
{
	if(m_filebuffer != 0)
	{
		m_filebuffer = (unsigned char *)
			::VirtualAlloc(NULL,						// address of region to reserve or commit
					     MAX_TEXTURESIZE,				// size of region
						 MEM_COMMIT,					// type of allocation
						 PAGE_READWRITE|PAGE_EXECUTE);	// type of access protection
	}
}

void CImage::FreeFileBuffer()
{
	if(m_filebuffer == 0)
		return;
	::VirtualFree(m_filebuffer,// address of region of committed pages
				  0,		   // size of region, needs to be Zero for MEM_RELEASE
				  MEM_RELEASE);// type of free operation
	m_filebuffer = 0;
}





//======================================================================================================
//	FUNCTIONS TO MANIPULATE IMAGES
//======================================================================================================

/**********************************************************
reduce dimensions of an 8888 tex by 2.  !!assumes dimensions allow it!!
**********************************************************/
void ImageReduce32(byte *dest, byte *src, int &width, int &height)
{
	DWORD color;
	int r, c, s;


	int sfactor = 0;
	int tfactor = 0;

	if (width/2 != 0)
	{
		sfactor = 1;
		width /= 2;
	}

	if (height/2 != 0)
	{
		tfactor = 1;
		height /= 2;
	}

	for (r = 0; r < height; r++)
	{
		for (c = 0; c < width; c++)
		{
			for (s = 0; s < 4; s++)
			{
				color =  src[ ((2*r)		 *width*8) + ((2*c)			*4)+ s];
				color += src[ ((2*r)		 *width*8) + ((2*c)+sfactor)*4 + s];
				color += src[(((2*r)+tfactor)*width*8) + ((2*c)			*4)+ s];
				color += src[(((2*r)+tfactor)*width*8) + ((2*c)+sfactor)*4 + s];
				color /= 4;

				dest[(r*width*4) + (c*4) + s] = (byte) color;
			}
		}
	}
}

/**********************************************************
reduce dimensions of an 888 tex by 2.  !!assumes dimensions allow it!!
**********************************************************/
void ImageReduce24(byte *dest, byte *src, int nwidth, int nheight)
{
	DWORD color;
	int r, c, s;

	for (r = 0; r < nheight; r++)
	{
		for (c = 0; c < nwidth; c++)
		{
			for (s = 0; s < 3; s++)
			{
				color =  src[ ((2*r)   *nwidth*6) + ((2*c)   *3)+ s];
				color += src[ ((2*r)   *nwidth*6) + ((2*c)+1)*3 + s];
				color += src[(((2*r)+1)*nwidth*6) + ((2*c)   *3)+ s];
				color += src[(((2*r)+1)*nwidth*6) + ((2*c)+1)*3 + s];
				color /= 4;

				dest[(r*nwidth*3) + (c*3) + s] = (byte) color;
			}
		}
	}
}

/**********************************************************
convert a tex format from 8888 to something else
**********************************************************/
byte* ImageConvert(byte *src, int format, int width, int height)
{
	byte *dest = (byte*)MALLOC(width * height * 2);
	if (dest == NULL) FError("mem for pic format conversion");

// convert from 8888 to 565
	if (format == IMAGE_565)
	{
		for (unsigned int p = 0; p < width * height; p++)
		{
			dest[p*2+1]  = (src[(p*4)]   * 32 / 256) << 3;
			dest[p*2+1] |= (src[(p*4)+1] * 64 / 256) >> 3;

			dest[p*2]    = (src[(p*4)+1] * 64 / 256) << 5;
			dest[p*2]   |= (src[(p*4)+2] * 32 / 256);
		}
	}
// convert from 8888 to 1555
// FIXME - also do convertions to 4444
	else
	{
		for (unsigned int p = 0; p < width * height; p++)
		{
			dest[p*2  ]	= 0x00;
			dest[p*2+1]	= 0x00;

		// alpha
			if (!(dest[(p*4)  ] == 228) &&
				!(dest[(p*4)+1] == 41) &&
				!(dest[(p*4)+2] == 226))
			{	
				dest[p*2+1] |= (src[(p*4)]   * 32 / 256) << 2;
				dest[p*2+1] |= (src[(p*4)+1] * 32 / 256) >> 3;

				dest[p*2]   |= (src[(p*4)+1] * 32 / 256) << 5;
				dest[p*2]   |= (src[(p*4)+2] * 32 / 256);

				dest[p*2+1] |= 0x80;
			}
		}
	}
	return dest;
}









