#include "Tex_image.h"
#include "Ijl/ijl.h"

//======================================================================================
//======================================================================================

namespace 
{
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

	const int DEFAULT_TEXTURE_WIDTH  = 16;
	const int DEFAULT_TEXTURE_HEIGHT = 64;
}

//======================================================================================
//======================================================================================

/*
==========================================
Image Reader COnstructor/Destructor
==========================================
*/
CImageReader::CImageReader()
{
	width = height = 0;
	format = IMG_NONE;

	buffersize = 0;
	data = 0;
	
	mipbuffersize = 0;
	mipmapdata = 0;
}

CImageReader::~CImageReader()		
{	
	UnlockBuffer();
	UnlockMipMapBuffer();
	Reset();
}

/*
==========================================
Key the transparent color
==========================================
*/
void CImageReader::ColorKey()
{
	if(!data || format != IMG_RGBA)
		return;

	int size = width * height * 4;
	for (int p = 0; p < size ; p+=4)
	{
		if ((data[p  ] == 228) && (data[p+1] == 41) &&	(data[p+2] == 226))
			 data[p+3] = 0;
		else
			data[p+3] = 255;
	}
}

/*
==========================================
Reset the image data
==========================================
*/
void CImageReader::Reset()
{
	//Dont release data buffer if its statically alloced
	if(data && !buffersize)
	{
		delete [] data;
		data = 0;
	}
	height = 0;
	width = 0;
	format = IMG_NONE;
}

/*
==========================================
Load a default image 
used as a replacement when we can't find a texture
==========================================
*/
bool CImageReader::DefaultTexture()
{
	width = DEFAULT_TEXTURE_WIDTH;
	height = DEFAULT_TEXTURE_HEIGHT;

	// all raw pic data is 32 bits
	if(buffersize < width * height * 3)
	{
		buffersize = width * height * 3;
		LockBuffer(buffersize);
	}

	for (int p = 0; p < width * height * 3; p++)
		data[p] = rand();

	format = IMG_RGB;
	return true;
}

/*
==========================================
Read a texture from the given path
==========================================
*/
bool CImageReader::Read(const char * file)
{
	if(data)
		Reset();

	char filename[MAX_PATH];
	bool err = false;

/*	sprintf(filename,"%s.jpg",file);
	if(m_fileReader.Open(filename) == true)
	{
		ComPrintf("Read JPEG!\n");
		bool err = Read_JPG();
		m_fileReader.Close();
		return err;
	}
*/
	//default to a tga
	sprintf(filename,"%s.tga",file);
	if(m_fileReader.Open(filename) == true)
	{
		err = Read_TGA();
		m_fileReader.Close();
		return err;
	}

	sprintf(filename,"%s.pcx",file);
	if(m_fileReader.Open(filename) == true)
	{
		err = Read_PCX();
		m_fileReader.Close();
		return err;
	}

	ConPrint("CImageReader::Read: Couldnt open %s\n",filename);
	return false;
}

//======================================================================================
//24 bit, image reading funcs
//======================================================================================

/*
==========================================
Read a PCX file from current filereader
==========================================
*/
bool CImageReader::Read_PCX()
{
	PCX_header header;
	int bpp;

	m_fileReader.Read(&header,sizeof(header),1);
	if((header.manufacturer != 10) || 
	   (header.version != 5) || 
	   (header.encoding != 1)|| 
	   (header.bits_per_pixel != 8))
	{
		ConPrint("CImageReader::Read_PCX:Bad texture file");
		return false;
	}

	// decide if we're going to need an alpha channel
	// do we have a '_' for color keying?

	bool colorkey = false;

	const char *tname = m_fileReader.GetFileName();
	for (int offset=strlen(tname)-1; offset>0; offset--)
	{
		if ((tname[offset] == '\\') || (tname[offset] == '/'))
		{
			if (tname[offset+1] == '_')
				colorkey = true;
			break;
		}
	}

	if (!offset)
		if (tname[0] == '_')
			colorkey = true;


	if (colorkey || (header.num_planes==4))
	{
		ConPrint("               %s\n", m_fileReader.GetFileName());
		format = IMG_RGBA;
		bpp = 4;
	}
	else
	{
		format = IMG_RGB;
		bpp = 3;
	}


	width = header.width - header.x + 1;
	height= header.height - header.y + 1;

	if(buffersize < width * height * bpp)
	{
		buffersize = width * height * bpp;
		LockBuffer(buffersize);
	}
	memset(data, 0xFF, width * height * bpp);

	byte ch;
	int number, color;
	int countx, county, index;

	//for every line
	for (county = 0; county < height; county++)
	{
		//decode this line for each r g b		
		for (color = 0; color < header.num_planes; color++)
		{
			for (countx = 0; countx < width; )
			{
				ch = m_fileReader.GetChar();

				// Verify if the uppers two bits are sets
				if ((ch >= 192) && (ch <= 255))
				{
					number = ch - 192;
					ch = m_fileReader.GetChar();
				}
				else
					number = 1;

				for (index = 0; index < number; index++)
				{
					data[(county * bpp * (width)) + (bpp * countx) + color] = ch;
					countx++;
				}
			}
		}
	}

	if (colorkey)
		ColorKey();
	return true;
}

/*
==========================================
Read a TGA file from the given file
==========================================
*/
bool CImageReader::Read_TGA()
{
	m_fileReader.Seek(12,SEEK_SET);

	width   = m_fileReader.GetChar();
	width  |= m_fileReader.GetChar() << 8;
	height  = m_fileReader.GetChar();
	height |= m_fileReader.GetChar() << 8;
	int bpp = m_fileReader.GetChar() / 8;

	format = (EImageFormat)bpp;

	bool colorkey = false;
	const char *tname = m_fileReader.GetFileName();
	for (int offset=strlen(tname)-1; offset>0; offset--)
	{
		if ((tname[offset] == '\\') || (tname[offset] == '/'))
		{
			if (tname[offset+1] == '_')
				colorkey = true;
			break;
		}
	}

	if (!offset)
		if (tname[0] == '_')
			colorkey = true;

	if (colorkey)
		format = IMG_RGBA;


	// alpha/no alpha?  we already know that
	m_fileReader.GetChar();

	if(buffersize < width * height * bpp)
	{
		buffersize = width * height * bpp;
		LockBuffer(buffersize);
	}
	memset(data, 0xFF, width * height * bpp);

	for (int h= height-1; h>=0; h--)
	{
		for (int w=0; w<width; w++)
		{
			data[h*width*(int)format + w*(int)format + 2] = m_fileReader.GetChar();
			data[h*width*(int)format + w*(int)format + 1] = m_fileReader.GetChar();
			data[h*width*(int)format + w*(int)format    ] = m_fileReader.GetChar();

			if (bpp == 4)
				data[h*width*(int)format + w*(int)format + 3] = m_fileReader.GetChar();
		}
	}


	if (colorkey)
		ColorKey();

	return true;
}

/*
==========================================
Read a jpeg
==========================================
*/
bool CImageReader::Read_JPG()
{
	JPEG_CORE_PROPERTIES jcprop;

	memset(&jcprop,0,sizeof(JPEG_CORE_PROPERTIES));

	if(ijlInit( &jcprop ) != IJL_OK ) 
	{
		ComPrintf("CImageReader::Read_JPG: Unable to initialize the Intel JPEG library\n");
		return false;
	}

	jcprop.JPGBytes = m_fileReader.GetData();
	jcprop.JPGSizeBytes = m_fileReader.GetSize();

	if(ijlRead( &jcprop, IJL_JBUFF_READPARAMS ) != IJL_OK )
	{
		ComPrintf("CImageReader::Read_JPG: Unable to read jpeg header\n");
		return false;
	}

	if(jcprop.JPGChannels != IMG_RGB)
	{
		ComPrintf("CImageReader::Read_JPB: Image is not RGB\n");
		ijlFree( &jcprop );
		return false;
	}

	width=jcprop.JPGWidth;
	height=jcprop.JPGHeight;

	if(buffersize < jcprop.JPGWidth * jcprop.JPGHeight * jcprop.JPGChannels)
	{
		buffersize = width * height * jcprop.JPGChannels;
		LockBuffer(buffersize);
	}
	memset(data, 0, width * height * jcprop.JPGChannels);

	jcprop.DIBWidth    = jcprop.JPGWidth;
	jcprop.DIBHeight   = jcprop.JPGHeight;
	jcprop.DIBChannels = jcprop.JPGChannels;
	jcprop.DIBBytes    = data; 
	jcprop.DIBColor	   = IJL_RGB;
	
	if( ijlRead( &jcprop, IJL_JBUFF_READWHOLEIMAGE ) != IJL_OK )
	{
		ComPrintf("CImageReader::Read_JPG: Can't read jpeg file\n");
		height = 0;
		width = 0;
		return false;
	}

	if( ijlFree( &jcprop ) != IJL_OK ) 
	{
		ComPrintf("CImageReader::Read_JPG: Can't free intel jpeg library!\n");
		return false;
	}
	format=IMG_RGB;
	return true;
}


//======================================================================================
//======================================================================================

/*
==========================================
Set the texture to be 2n in size,
and return number of possible mipmaps
==========================================
*/
int CImageReader::GetMipCount()
{
	int tmps = 1;
	int miplevels = 0;
	
	// make it a power of 2
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (width & tmps)
			break;
	}
	width = tmps;
	
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (height & tmps)
			break;
	}
	height = tmps;

	int largestdim = width;
	if (width < height)
		largestdim = height;

	// figure out how many mip map levels we should have
	for (miplevels=1, tmps=1; tmps < largestdim; tmps <<= 1)
		miplevels++;

	if(miplevels > 10)
		miplevels = 10;
	return miplevels;
}


/*
==========================================
Reduce the Size of the image by 2x
Used for mip maps
==========================================
*/
void CImageReader::ImageReduce()
{
	DWORD color;
	int r=0, c=0, s=0;
	
	int	sfactor = 1;
	int	tfactor = 1;

	width /=2;
	height /=2;

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

	int bpp = (int)format;
	int size = (width)*(height)*(bpp);

	for (r = 0; r < height; r++)
	{
		for (c = 0; c < width; c++)
		{
			for (s = 0; s < bpp; s++)
			{
                color =  data[ ((2*r)		  *width*bpp*2) + ((2*c)		 *bpp)+ s];
                color += data[ ((2*r)		  *width*bpp*2) + ((2*c)+sfactor)*bpp + s];
                color += data[(((2*r)+tfactor)*width*bpp*2) + ((2*c)		 *bpp)+ s];
                color += data[(((2*r)+tfactor)*width*bpp*2) + ((2*c)+sfactor)*bpp + s];
                color /= 4;

				mipmapdata[(r*width*bpp) + (c*bpp) + s] = (byte) color;
			}
		}
	}
	memcpy(data,mipmapdata, size);
}


//======================================================================================
//======================================================================================

/*
==========================================
Read Image data from a stream
used for reading Lightmaps from the world file
==========================================
*/
bool CImageReader::ReadLightMap(unsigned char **stream)
{
	// read the data from the stream
	width = **stream;
	(*stream)++;
	height = **stream;
	(*stream)++;

	if (!width || !height)
		return false;

	if(buffersize < width * height * 3)
	{
		buffersize = width * height * 3;
		LockBuffer(buffersize);
	}

	for (int p = 0; p < width * height * 3; p++)
	{
		data[p] = **stream;
		(*stream)++;
	}
	format = IMG_RGB;
	return true;
}

//======================================================================================
//======================================================================================

/*
==========================================
Lock a static buffer to do texture i/o
avoid continously allocing and freeing memory
==========================================
*/
void CImageReader::LockBuffer(int size)
{
	UnlockBuffer();
	data = (byte*) g_pHunkManager->HunkAlloc(size);
	if(!data)
	{
		FError("CImageReader::LockBuffer: Failed to alloc %d\n", size);
		return;
	}
	buffersize = size;
}

void CImageReader::UnlockBuffer()
{
	buffersize = 0;
	if(data)
	{
		g_pHunkManager->HunkFree(data);
		data = 0;
	}
}

void CImageReader::LockMipMapBuffer(int size)
{
	UnlockMipMapBuffer();
	mipmapdata = (byte*) g_pHunkManager->HunkAlloc(size);
	if(!mipmapdata)
	{
		FError("CImageReader::LockMipMapBuffer: Failed to alloc %d\n", size);
		return;
	}
	mipbuffersize = size;
}

void CImageReader::UnlockMipMapBuffer()
{
	mipbuffersize = 0;
	if(mipmapdata)
	{
		g_pHunkManager->HunkFree(data);
		mipmapdata = 0;
	}
}


//======================================================================================
//======================================================================================
//Image Writer Implementation
//======================================================================================
//======================================================================================


/*
==========================================
Constructor Destructores
==========================================
*/

CImageWriter::CImageWriter(int iwidth, int iheight, 
						   const byte * idata) 
	:m_width(iwidth), m_height(iheight), m_pData(idata)
{}

CImageWriter::CImageWriter(CImageReader * pImage)
{
	m_height = pImage->GetHeight();
	m_width = pImage->GetWidth();
	m_pData = pImage->GetData();
}

CImageWriter::~CImageWriter()
{	m_pData = 0;
}

/*
==========================================
Write am Image to Disk
==========================================
*/

void CImageWriter::Write(const char *name, EImageFileFormat iformat)
{
	if(!m_pData)
	{
		ConPrint("CImageWriter::Write: No file data to write to %s\n", name);
		return;
	}

	FILE *fp;
	fp = fopen(name,"w+b");
	if(!fp)
	{
		ConPrint("CImageWriter::Write: Unable to open %s for writing\n", name);
		return;
	}

	switch(iformat)
	{
	case FORMAT_PCX:
		Write_PCX(fp);
		break;
	case FORMAT_TGA:
		Write_TGA(fp);
		break;
	}
	fclose(fp);

	ConPrint("CImageReader::Write:Wrote %s\n", name);
}

/*
==========================================
Write a pcx file 
==========================================
*/
void CImageWriter::Write_PCX( FILE *fp)
{
	int		i, j, p, length;
	PCX_header	pcx;
	byte		*pack;
	const byte		*packdata;


	pcx.manufacturer = 0x0a;	// PCX id
	pcx.version = 5;			// 256 color
 	pcx.encoding = 1;		// uncompressed
	pcx.bits_per_pixel = 8;		// 256 color
	pcx.x      = 0;
	pcx.y      = 0;
	pcx.width  = (short)(m_width-1);
	pcx.height = (short)(m_height-1);
	pcx.x_res  = (short)m_width;
	pcx.y_res  = (short)m_height;
	memset (pcx.ega_palette,0,sizeof(pcx.ega_palette));
	pcx.num_planes = 3;
	pcx.bytes_per_line = (short)m_width;
	pcx.palette_type = 2;
	memset (pcx.junk, 0, sizeof(pcx.junk));

	//pack the image
	pack = new byte[m_width * m_height * 4];

	if (pack == NULL) 
		FError("CImageReader:Write_PCX::mem for writing pcx");
	
	byte *pack2 = pack;

// gl reads from bottom to top, so write it backwards
	for (i = (m_height - 1); i >= 0; i--)
	{
		for (p = 0; p < pcx.num_planes; p++)
		{
			packdata = m_pData + (i*m_width*4) + p;

			for (j = 0; j<m_width; j++)
			{
				if ( (*m_pData & 0xc0) != 0xc0)
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
	
	delete [] pack2;
}



/*
==========================================
Write TGA file
==========================================
*/
void CImageWriter::Write_TGA( FILE *fp)
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
	fputc((char)(m_width & 0x00ff), fp);
	fputc((char)((m_width & 0xff00) >> 8), fp);
	fputc((char)(m_height & 0x00ff), fp);
	fputc((char)((m_height & 0xff00) >> 8), fp);
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

	for (h = 0; h < m_height; h++)
	{
		for (w = 0; w < m_width; w++)
		{
			fwrite(&m_pData[h*m_width*components + w*components + 2], 1, 1, fp);	// blue
			fwrite(&m_pData[h*m_width*components + w*components + 1], 1, 1, fp);	// green
			fwrite(&m_pData[h*m_width*components + w*components    ], 1, 1, fp);	// red
//			if (pic->alpha)
				fwrite(&m_pData[h*m_width*components + w*components + 3], 1, 1, fp);	// alpha
		}
	}
}

//======================================================================================
//======================================================================================


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
Image Type           Description
	0                No Image Data Included
	1                Uncompressed, Color-mapped Image
	2                Uncompressed, True-color Image
	3                Uncompressed, Black-and-white image
	9                Run-length encoded, Color-mapped Image
	10               Run-length encoded, True-color Image
	11               Run-length encoded, Black-and-white image
*/






#if 0
//
// Load a jpeg using jpeglib
//

// This procedure is called by the IJPEG library when an error
// occurs.
static void error_exit (j_common_ptr pcinfo){
	// Create the message string
	char sz[256];
	(pcinfo->err->format_message) (pcinfo, sz);

	err->CriticalError(ERROR_FORMAT_NOT_SUPPORTED, "%s\n",sz);
}


static void init_source (j_decompress_ptr cinfo){
}


static boolean fill_input_buffer (j_decompress_ptr cinfo){
	struct jpeg_source_mgr * src = cinfo->src;
	static JOCTET FakeEOI[] = { 0xFF, JPEG_EOI };

	/* Generate warning */
	err->Log(UNKNOWN_ERROR, "Premature end of file\n");
  
	/* Insert a fake EOI marker */
	src->next_input_byte = FakeEOI;
	src->bytes_in_buffer = 2;

	return TRUE;
}


static void skip_input_data (j_decompress_ptr cinfo, long num_bytes) {
	struct jpeg_source_mgr * src = cinfo->src;
  
	if(num_bytes >= (long)src->bytes_in_buffer) {
		fill_input_buffer(cinfo);
		return;
	}

	src->bytes_in_buffer -= num_bytes;
	src->next_input_byte += num_bytes;
}


void term_source (j_decompress_ptr cinfo){
  /* no work necessary here */
}


void TexMng::JPG_Decode(VFile *vf, texinfo *tex){
	
	jpeg_decompress_struct cinfo;	// IJPEG decoder state.
	jpeg_error_mgr         jerr;	// Custom error manager.

	cinfo.err = jpeg_std_error (&jerr);
	jerr.error_exit = error_exit;	// Register custom error manager.

	jpeg_create_decompress (&cinfo);

	
	struct jpeg_source_mgr * src;
	//jpeg_stdio_src(&cinfo, fp);
	
	cinfo.src= (struct jpeg_source_mgr *) (*cinfo.mem->alloc_small) 
		((j_common_ptr) &cinfo, JPOOL_PERMANENT, sizeof(struct jpeg_source_mgr));

	
	src = cinfo.src;

	src->init_source = init_source;
	src->fill_input_buffer = fill_input_buffer;
	src->skip_input_data = skip_input_data;
	src->resync_to_restart = jpeg_resync_to_restart;	// use default method
	src->term_source = term_source;

	// Set up data pointer
	src->bytes_in_buffer = vf->size;
	src->next_input_byte = vf->mem;

	jpeg_read_header (&cinfo, TRUE);

	cinfo.do_fancy_upsampling = FALSE;		// fast decompression

    cinfo.dct_method = JDCT_FLOAT;			// Choose floating point DCT method.



	jpeg_start_decompress(&cinfo);

    tex->width = cinfo.output_width;
    tex->height = cinfo.output_height;

	if (cinfo.out_color_space == JCS_GRAYSCALE){
		/*tex->bpp=1;
		tex->mem = (byte *) malloc(1*tex->width*tex->height);
		if (!tex->mem) {
			tex->mem=NULL;
			return;
		}*/
		err->CriticalError(UNKNOWN_ERROR,"grayscale not supported!!!");
	}
	else{
		tex->bpp=3;
		tex->mem = (byte *) malloc(3*tex->width*tex->height);
		if (!tex->mem) {
			return;
		}
		
		byte *pDst=tex->mem;
		byte **ppDst=&pDst;
		int num_scanlines=0;
		while (cinfo.output_scanline < cinfo.output_height){
			num_scanlines=jpeg_read_scanlines (&cinfo, ppDst, 1);
			*ppDst += num_scanlines * 3 * cinfo.output_width;
		}
	}
	
	jpeg_finish_decompress(&cinfo);

	jpeg_destroy_decompress (&cinfo);
}
#endif
