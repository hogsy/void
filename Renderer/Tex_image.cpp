#ifdef RENDERER
#include "Standard.h"
#else
#include "Com_defs.h"
#include "Rast_main.h"
#include "../Devvoid/Std_lib.h"
#endif


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
		WORD m_width, m_height;
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
Constructor/Destructor
Construct is private so that only one
ImageReader object will ever exits
==========================================
*/
CImageReader::CImageReader()
{
	m_width = m_height = 0;
	m_format = IMG_NONE;

	for (int i=0; i<MAX_MIPMAPS; i++)
		m_mipmapdata[i] = NULL;
}

CImageReader::~CImageReader()		
{	FreeMipData();
}


/*
================================================
Static Access func
================================================
*/
CImageReader & ::CImageReader::GetReader()
{
	static CImageReader imgReader;
	return imgReader;
}

/*
==========================================
Free any data we have allocated
==========================================
*/
void CImageReader::FreeMipData(void)
{
	for (int i=0; i<MAX_MIPMAPS; i++)
	{
		if (m_mipmapdata[i])
#ifdef RENDERER
			g_pHunkManager->HunkFree(m_mipmapdata[i]);
#else
			free(m_mipmapdata[i]);
#endif
		m_mipmapdata[i] = 0;
	}
}

/*
==========================================
Key the transparent color
==========================================
*/
void CImageReader::ColorKey(byte *data)
{
	if(!data || m_format != IMG_RGBA)
		return;

	int size = m_width * m_height * 4;
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
Load a default image 
used as a replacement when we can't find a texture
==========================================
*/
void CImageReader::DefaultTexture(TextureData &imgData)
{
	m_width = DEFAULT_TEXTURE_WIDTH;
	m_height = DEFAULT_TEXTURE_HEIGHT;

	ConfirmMipData();

	for (int p = 0; p < m_width * m_height * 3; p++)
		m_mipmapdata[m_miplevels-1][p] = rand();

	m_format = IMG_RGB;

	imgData.height = m_height;
	imgData.width = m_width;
	imgData.data = &m_mipmapdata[0];
	imgData.format = m_format;
	imgData.numMipMaps = m_miplevels;
}

/*
==========================================
Read a texture from the given path
==========================================
*/
bool CImageReader::Read(const char * file, TextureData &imgData)
{
	char tgafilename[MAX_PATH];
	char pcxfilename[MAX_PATH];
	char jpgfilename[MAX_PATH];
	bool err = false;

	//default to a tga
	sprintf(tgafilename,"%s.tga",file);
	sprintf(pcxfilename,"%s.pcx",file);
	sprintf(jpgfilename,"%s.jpg",file);

	if(m_fileReader.Open(tgafilename) == true)
		err = Read_TGA(imgData);
	else if (m_fileReader.Open(pcxfilename) == true)
		err = Read_PCX(imgData);
	else if (m_fileReader.Open(jpgfilename) == true)
		err = Read_JPG(imgData);
	else
	{
		ComPrintf("CImageReader::Read: Couldnt open %s\n",file);
		return false;
	}

	//Create mipmaps if needed
	if(imgData.bMipMaps)
	{
		int mipcount = imgData.numMipMaps - 1;
		while (mipcount > 0)
		{
			ImageReduce(mipcount);
			mipcount--;
		}
	}

	m_fileReader.Close();
	return err;
}

//======================================================================================
//24 bit, image reading funcs
//======================================================================================

/*
==========================================
Read a PCX file from current filereader
==========================================
*/
bool CImageReader::Read_PCX(TextureData &imgData)
{
	PCX_header header;
	int bpp;

	m_fileReader.Read(&header,sizeof(header),1);
	if((header.manufacturer != 10) || 
	   (header.version != 5) || 
	   (header.encoding != 1)|| 
	   (header.bits_per_pixel != 8))
	{
		ComPrintf("CImageReader::Read_PCX:Bad texture file");
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
		m_format = IMG_RGBA;
		bpp = 4;
	}
	else
	{
		m_format = IMG_RGB;
		bpp = 3;
	}


	m_width = header.m_width - header.x + 1;
	m_height= header.m_height - header.y + 1;

	ConfirmMipData();
	memset(m_mipmapdata[m_miplevels-1], 0xFF, m_width * m_height * bpp);

	byte ch;
	int number, color;
	int countx, county, index;

	//for every line
	for (county = 0; county < m_height; county++)
	{
		//decode this line for each r g b		
		for (color = 0; color < header.num_planes; color++)
		{
			for (countx = 0; countx < m_width; )
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
					m_mipmapdata[m_miplevels-1][(county * bpp * (m_width)) + (bpp * countx) + color] = ch;
					countx++;
				}
			}
		}
	}

	if (colorkey)
		ColorKey(m_mipmapdata[m_miplevels-1]);

	imgData.height = m_height;
	imgData.width = m_width;
	imgData.data = &m_mipmapdata[0];
	imgData.format = m_format;
	imgData.numMipMaps = m_miplevels;
	return true;
}

/*
==========================================
Read a TGA file from the given file
==========================================
*/
bool CImageReader::Read_TGA(TextureData &imgData)
{
	m_fileReader.Seek(12,SEEK_SET);

	m_width   = m_fileReader.GetChar();
	m_width  |= m_fileReader.GetChar() << 8;
	m_height  = m_fileReader.GetChar();
	m_height |= m_fileReader.GetChar() << 8;
	int bpp = m_fileReader.GetChar() / 8;

	m_format = (EImageFormat)bpp;

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
		m_format = IMG_RGBA;

	// alpha/no alpha?  we already know that
	m_fileReader.GetChar();

	ConfirmMipData();
	memset(m_mipmapdata[m_miplevels-1], 0xFF, m_width * m_height * (int)m_format);

	for (int h= m_height-1; h>=0; h--)
	{
		for (int w=0; w<m_width; w++)
		{
			m_mipmapdata[m_miplevels-1][h*m_width*(int)m_format + w*(int)m_format + 2] = m_fileReader.GetChar();
			m_mipmapdata[m_miplevels-1][h*m_width*(int)m_format + w*(int)m_format + 1] = m_fileReader.GetChar();
			m_mipmapdata[m_miplevels-1][h*m_width*(int)m_format + w*(int)m_format    ] = m_fileReader.GetChar();

			if (bpp == 4)
				m_mipmapdata[m_miplevels-1][h*m_width*(int)m_format + w*(int)m_format + 3] = m_fileReader.GetChar();
		}
	}

	if (colorkey)
		ColorKey(m_mipmapdata[m_miplevels-1]);

	imgData.height = m_height;
	imgData.width = m_width;
	imgData.data = &m_mipmapdata[0];
	imgData.format = m_format;
	imgData.numMipMaps = m_miplevels;
	return true;
}

/*
==========================================
Read a jpeg
==========================================
*/
bool CImageReader::Read_JPG(TextureData &imgData)
{
	JPEG_CORE_PROPERTIES image;
	ZeroMemory( &image, sizeof( JPEG_CORE_PROPERTIES ) );

	if(ijlInit(&image) != IJL_OK)
	{
        ComPrintf("CImageReader::Read_JPG() : Cannot initialize Intel JPEG library\n");
		return false;
	}

	// read entire file into a buffer
	image.JPGSizeBytes = m_fileReader.GetSize();
	byte *jbuff = new byte[image.JPGSizeBytes];
	if (!jbuff)
		return false;

	m_fileReader.Read(jbuff, image.JPGSizeBytes, 1);
	image.JPGBytes = jbuff;

	IJLERR jerr = ijlRead(&image, IJL_JBUFF_READPARAMS);
	if (jerr != IJL_OK)
	{
		ComPrintf("CImageReader::Read_JPG() : Cannot read JPEG file header\n");
		delete [] jbuff;
		return false;
	}


	// Set up local data.
	m_width = image.JPGWidth;
	m_height= image.JPGHeight;
	m_format= IMG_RGB;
	ConfirmMipData();
	memset(m_mipmapdata[m_miplevels-1], 0xFF, m_width * m_height * (int)m_format);

	// Set up the info on the desired DIB properties.
	image.DIBWidth = m_width;
	image.DIBHeight = m_height; // Implies a bottom-up DIB.
	image.DIBChannels = 3;
	image.DIBColor = IJL_BGR;
	image.DIBPadBytes = 0;
	image.DIBBytes = m_mipmapdata[m_miplevels-1];


	// Set the JPG color space ... this will always be
	// somewhat of an educated guess at best because JPEG
	// is "color blind" (i.e., nothing in the bit stream
	// tells you what color space the data was encoded from).
	// However, in this example we assume that we are
	// reading JFIF files which means that 3 channel images
	// are in the YCbCr color space and 1 channel images are
	// in the Y color space.
	switch(image.JPGChannels)
	{
		case 1:
		{
			image.JPGColor = IJL_G;
			break;
		}
		case 3:
		{
			image.JPGColor = IJL_YCBCR;
			break;
		}
		default:
		{
			// This catches everything else, but no
			// color twist will be performed by the IJL.
			image.DIBColor = (IJL_COLOR)IJL_OTHER;
			image.JPGColor = (IJL_COLOR)IJL_OTHER;
			break;
		}
	}


	// Now get the actual JPEG image data into the pixel buffer.

	if (ijlRead(&image, IJL_JBUFF_READWHOLEIMAGE) != IJL_OK)
	{
		ComPrintf("CImageReader::Read_JPG() : Error reading JPEG\n");
		delete [] jbuff;
		return false;
	}

	// Clean up the IntelR JPEG Library.
	ijlFree(&image);
	delete [] jbuff;

	imgData.height = m_height;
	imgData.width = m_width;
	imgData.data = &m_mipmapdata[0];
	imgData.format = m_format;
	imgData.numMipMaps = m_miplevels;
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
void CImageReader::GetMipCount()
{
	int tmps = 1;
	m_miplevels = 0;
	
	// make it a power of 2
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (m_width & tmps)
			break;
	}
	w = m_width = tmps;
	
	for (tmps=1<<10; tmps; tmps>>=1)
	{
		if (m_height & tmps)
			break;
	}
	h = m_height = tmps;

	int largestdim = m_width;
	if (m_width < m_height)
		largestdim = m_height;

	// figure out how many mip map levels we should have
	for (m_miplevels=1, tmps=1; tmps < largestdim; tmps <<= 1)
		m_miplevels++;

	if(m_miplevels > 10)
		m_miplevels = 10;
}


/*
==========================================
Reduce the Size of the image by 2x
Used for mip maps
==========================================
*/
void CImageReader::ImageReduce(int m)
{
	if (m==0)
		return;

	DWORD color;
	int r=0, c=0, s=0;
	
	int	sfactor = 2;
	int	tfactor = 2;

	w /=2;
	h /=2;

	if (!w)
	{
		w = 1;
		sfactor = 1;
	}

	if (!h)
	{
		h = 1;
		tfactor = 1;
	}

	int bpp = (int)m_format;

	for (r = 0; r < h; r++)
	{
		for (c = 0; c < w; c++)
		{
			for (s = 0; s < bpp; s++)
			{
				color  = m_mipmapdata[m][(tfactor*r)*(sfactor*w)*bpp           +  (sfactor*c*bpp) + s];
				color += m_mipmapdata[m][(tfactor*r)*(sfactor*w)*bpp           + ((sfactor*c+sfactor-1)*bpp) + s];
				color += m_mipmapdata[m][(tfactor*r+tfactor-1)*(sfactor*w)*bpp +  (sfactor*c*bpp) + s];
				color += m_mipmapdata[m][(tfactor*r+tfactor-1)*(sfactor*w)*bpp + ((sfactor*c+sfactor-1)*bpp) + s];
				color /= 4;

				m_mipmapdata[m-1][(r*w*bpp) + (c*bpp) + s] = (byte) color;
			}
		}
	}
}

/*
==========================================
allocate / make sure we already have allocated
all the memory we will need for all mipmaps
==========================================
*/
void CImageReader::ConfirmMipData(void)
{
	GetMipCount();

	for (int m=m_miplevels-1; m>=0; m--)
	{
		if (m_mipmapdata[m])
			continue;

#ifdef RENDERER
		m_mipmapdata[m] = (byte*)g_pHunkManager->HunkAlloc(mipdatasizes[m]);
#else
		m_mipmapdata[m] = new byte[mipdatasizes[m]];
#endif
		if (!m_mipmapdata[m])
		{
			FError("CImageReader::ConfirmMipData: Failed to alloc %d\n", mipdatasizes);
			return;
		}
	}
}

/*
==========================================
Read Image data from a stream
used for reading Lightmaps from the world file
==========================================
*/
bool CImageReader::ReadLightMap(unsigned char **stream, TextureData &imgData)
{
	// read the data from the stream
	w = m_width = **stream;
	(*stream)++;
	h = m_height= **stream;
	(*stream)++;

	if(!m_width || !m_height)
		return false;

	ConfirmMipData();

	for (int p = 0; p < (m_width * m_height * 3); p++)
	{
		m_mipmapdata[m_miplevels-1][p] = **stream;
		(*stream)++;
	}
	
	imgData.height = m_height;
	imgData.width = m_width;
	imgData.format = IMG_RGB;
	imgData.data = &m_mipmapdata[0];
	imgData.numMipMaps = m_miplevels;

	return true;
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
		ComPrintf("CImageWriter::Write: No file data to write to %s\n", name);
		return;
	}

	FILE *fp;
	fp = fopen(name,"w+b");
	if(!fp)
	{
		ComPrintf("CImageWriter::Write: Unable to open %s for writing\n", name);
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

	ComPrintf("CImageReader::Write:Wrote %s\n", name);
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
	pcx.m_width  = (short)(m_width-1);
	pcx.m_height = (short)(m_height-1);
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

    tex->m_width = cinfo.output_width;
    tex->m_height = cinfo.output_height;

	if (cinfo.out_color_space == JCS_GRAYSCALE){
		/*tex->bpp=1;
		tex->mem = (byte *) malloc(1*tex->m_width*tex->m_height);
		if (!tex->mem) {
			tex->mem=NULL;
			return;
		}*/
		err->CriticalError(UNKNOWN_ERROR,"grayscale not supported!!!");
	}
	else{
		tex->bpp=3;
		tex->mem = (byte *) malloc(3*tex->m_width*tex->m_height);
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
