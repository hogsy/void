#include "Com_image.h"

#include "Com_file.h"

void dprintf(char *string,...);

#include <string.h>
#include <stdio.h>
#include <memory.h>

extern unsigned char quake2_pal[];
extern unsigned char quake1_pal[];

typedef unsigned short WORD;

typedef struct TGAHeader
{
	byte	id_length;
	byte	colormap_type;
	byte	image_type;
	byte	colormap_spec[5];
	byte	image_spec[10];
}TGA_header;


/*
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
Write TGA file
==========================================
*/
bool WriteTGAFromWal(const char *filename, CImageData *src)
{
	// write output file 
	FILE *fp;

	///FIX ME
	char filepath[COM_MAXPATH];
	strcpy(filepath,filename);
	int end = strlen(filename)-4;
	if(strcmp(filepath+end,".tga"))
		memcpy(filepath+end,".tga",4);

	
	fp = fopen(filepath, "w+b");
	if(fp == NULL)
		return false;


	TGA_header thead;
	
	thead.id_length = 0;
	thead.colormap_type = 0;
	thead.image_type = 2;
	memset(thead.colormap_spec,0,5);
	memset(thead.image_spec,0,10);
	thead.image_spec[4] = src->width & 0x00ff;
	thead.image_spec[5] = src->width & 0xff00;
	thead.image_spec[6] = src->height & 0x00ff;
	thead.image_spec[7] = src->height & 0xff00;
	thead.image_spec[8] = 24;		//24bit image
	thead.image_spec[9] = 0;

	fwrite(&thead,sizeof(TGA_header),1,fp);

	//data
	int i=0;
	int w=0, h=0;
	for(h=src->height-1;h>=0; h--)
	{
		for(w=0; w < src->width; w++)
		{
			fwrite(&quake2_pal[((src->data[w+(h*src->width)]*3) +2)],1,1,fp);
			fwrite(&quake2_pal[((src->data[w+(h*src->width)]*3) +1)],1,1,fp);
			fwrite(&quake2_pal[((src->data[w+(h*src->width)]*3)   )],1,1,fp);
		}
		i++;
	}

	dprintf("lines written : %d\nheight:%d,width:%d\n",i,src->height,src->width);

	fclose(fp);
	return true;
}



/*
==========================================

==========================================
*/

bool WriteTGAFromQ1PCX(const char *filename, CImageData *src)
{
	// write output file 
	FILE *fp;

	///FIX ME
	char filepath[COM_MAXPATH];
	strcpy(filepath,filename);
	strcat(filepath,".tga");

	fp = fopen(filepath,"wb");
	if(fp == NULL)
	{
		dprintf("Error opening destination file\n");
		return false;
	}


	TGA_header thead;
	
	thead.id_length = 0;
	thead.colormap_type = 0;
	thead.image_type = 2;
	memset(thead.colormap_spec,0,5);
	memset(thead.image_spec,0,10);
	thead.image_spec[4] = src->width & 0x00ff;
	thead.image_spec[5] = src->width & 0xff00;
	thead.image_spec[6] = src->height & 0x00ff;
	thead.image_spec[7] = src->height & 0xff00;
	thead.image_spec[8] = 24;		//24bit image
	thead.image_spec[9] = 0;

	fwrite(&thead,sizeof(TGA_header),1,fp);

	//data
	int i=0;
	int w=0, h=0;
	for(h=src->height-1;h>=0; h--)
	{
		for(w=0; w < src->width; w++)
		{
			fwrite(&quake1_pal[((src->data[w+(h*src->width)]*3) +2)],1,1,fp);
			fwrite(&quake1_pal[((src->data[w+(h*src->width)]*3) +1)],1,1,fp);
			fwrite(&quake1_pal[((src->data[w+(h*src->width)]*3)   )],1,1,fp);
		}
		i++;
	}

	dprintf("Bytes written: %d\nHeight:%d,Width:%d\n",src->height*src->width*3,src->height,src->width);

	fclose(fp);
	return true;
}



/*
==============================================================================================

==============================================================================================
*/

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
==========================================
Load a PCX file into the given buffer
==========================================
*/

bool WritePalImagetoAsc(const char *filename, CImageData *src, byte *palette);

bool WritePCXPalette(const char *filename, CImageData *dest)
{
	FILE	*fp;
	PCX_header header;
	byte ch;
	int number, color;
	int countx, county, i;

	fp = fopen(filename,"r+b");
	if(!fp)
		return false;

	fread(&header, sizeof(PCX_header),1, fp);

	if((header.manufacturer != 10) || 
	   (header.version != 5) || 
	   (header.encoding != 1)|| 
	   (header.bits_per_pixel != 8))
	{
		dprintf("Error reading PCX file\nManufacturer %d\nVersion %d\nEncoding %d\nBPP	%d\n",
				 header.manufacturer,header.version,header.encoding,header.bits_per_pixel);
		fclose(fp);
		return false;
	}

	dest->width = header.width - header.x + 1;
	dest->height = header.height - header.y + 1;
	dest->data = new byte[(dest->width * dest->height *4)];
	memset(dest->data, 0xFF, dest->width * dest->height * 4);

	// for every line
	for (county = 0; county < dest->height; county++)
	{
		// decode this line for each r g b a
		for (color = 0; color < header.num_planes; color++)
		{
			for (countx = 0; countx < dest->width; )
			{
				ch = getc(fp);

				// Verify if the uppers two bits are sets
				if ((ch >= 192) && (ch <= 255))
				{
					number = ch - 192;
					ch = getc(fp);
				}
				else
					number = 1;

				for (i = 0; i < number; i++)
				{
					dest->data[(county * 4 * (dest->width)) + (4 * countx) + color] = ch;
					countx++;
				}
			}
		}
	}

	//Check for palette
	if (header.num_planes == 1)
	{
		byte palette[768]; // r,g,b 256 times
		
		fread(palette, 1, 1, fp);
		if (palette[0] != 0x0c)
		{
			dprintf("Pallete not found\n");
			fclose(fp);
			return false;
		}

		fread(palette, 768, 1, fp);
		dprintf("Found pallete, writing.....\n");
		WritePalImagetoAsc(filename,dest,palette);
		fclose(fp);
		return true;
	}
	fclose(fp);
	return false;
}


/*
==========================================
Write the image data to ascii file
==========================================
*/

bool WritePalImagetoAsc(const char *filename, CImageData *src, byte *palette)
{
	if(!src || !filename )
		return false;

	if((src->height * src->width) *3 != 768)
	{
		dprintf("Image does not seem to be a 256 color pallete, len %d\n",src->height * src->width);
	}

	FILE	*fp;
	char fname[256];
	memset(fname,0,256);
	strcpy(fname,filename);
	int len=strlen(filename) -4;
	strcpy(fname+len,".txt");

	int index = 0;

	fp = fopen(fname,"w");
	if(!fp)
		return false;

	char buf[12];
	memset(buf,0,12);

	for (int i = 0; i <32; i++)
	{
		for (int j = 0; j < 8; j++)
		{
			index = src->data[(i*4*(8)) + (4*j)];
			
			sprintf(buf,"%d,",palette[index*3  ]);
			sprintf(buf+4,"%d,",palette[index*3+1]);
			sprintf(buf+8,"%d,",palette[index*3+2]);

			fwrite(buf,12,1,fp);
			memset(buf,0,12);
		}
		fputc('\n',fp);
	}
	fclose(fp);
	return true;
}