
#include "Util_tga.h"
#include "images.h"
/*
typedef struct TGA_Header
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
*/


/*
Image Type                   Description

         0                No Image Data Included
         1                Uncompressed, Color-mapped Image
         2                Uncompressed, True-color Image
         3                Uncompressed, Black-and-white image
         9                Run-length encoded, Color-mapped Image
         10               Run-length encoded, True-color Image
         11               Run-length encoded, Black-and-white image
*/


typedef struct
{
	byte *data;
	int width, height;
	bool alpha;
	unsigned int size;
}tgadata_t;



/**********************************************************
write a tga file
**********************************************************/
void tga_write(char *name, tgadata_t *pic)
{
	// write output file 

	FILE *fptr = fopen(name, "wb");
	if (!fptr)
	{
		ConPrint("couldn't create file %s\n", name);
		return;
	}

	fputc(0, fptr);
	fputc(0, fptr);
	fputc(2, fptr);
	
	fputc(0, fptr);
	fputc(0, fptr);
	fputc(0, fptr);
	fputc(0, fptr);
	fputc(0, fptr);
	
	fputc(0, fptr);
	fputc(0, fptr);
	fputc(0, fptr);
	fputc(0, fptr);
	fputc((char)(pic->width & 0x00ff), fptr);
	fputc((char)((pic->width & 0xff00) >> 8), fptr);
	fputc((char)(pic->height & 0x00ff), fptr);
	fputc((char)((pic->height & 0xff00) >> 8), fptr);
	if(pic->alpha)
		fputc(32, fptr);
	else
		fputc(24, fptr);

	if(pic->alpha)
		fputc(0x01, fptr);
	else
		fputc(0x00, fptr);

//	fwrite(pic->data,pic->size,1,fptr);
	int w, h, components = 3;
	if (pic->alpha) components = 4;

	for (h = 0; h < pic->height; h++)
	{
		for (w = 0; w < pic->width; w++)
		{
			fwrite(&pic->data[h*pic->width*components + w*components + 2], 1, 1, fptr);	// blue
			fwrite(&pic->data[h*pic->width*components + w*components + 1], 1, 1, fptr);	// green
			fwrite(&pic->data[h*pic->width*components + w*components    ], 1, 1, fptr);	// red
			if (pic->alpha)
				fwrite(&pic->data[h*pic->width*components + w*components + 3], 1, 1, fptr);	// alpha
		}
	}
	ConPrint("Wrote %s - %d bytes\n", name,pic->size);
}


/**********************************************************
read a tga file - FIXME - make this more robust?
**********************************************************/
bool tga_read(char *path, pic_t *pic)
{
	FILE *fin;


	fin = fopen (path, "rb");
	if (!fin)
	{
		ConPrint("%s not found\n", path);
		return false;
	}

// all these reads match skid's fputc's in the write func.  not sure what they are for
	fgetc(fin);
	fgetc(fin);
	fgetc(fin);

	fgetc(fin);
	fgetc(fin);
	fgetc(fin);
	fgetc(fin);
	fgetc(fin);

	fgetc(fin);
	fgetc(fin);
	fgetc(fin);
	fgetc(fin);

	pic->width   = fgetc(fin);
	pic->width  |= fgetc(fin) << 8;
	pic->height  = fgetc(fin);
	pic->height |= fgetc(fin) << 8;

	int comp = fgetc(fin) / 8;

	fgetc(fin);	// alpha/no alpha?  we already know that


	// always store the pic in 32 bit rgba
	pic->data = (byte*)MALLOC(pic->width * pic->height * 4);
	if (pic->data == NULL)	Error("Not enough memory for texture: %s", path);
	memset(pic->data, 0xFF, pic->width * pic->height * 4);

	int w, h;

	for (h=pic->height-1; h>=0; h--)
	{
		for (w=0; w<pic->width; w++)
		{
			fread(&pic->data[h*pic->width*comp + w*comp + 2], 1, 1, fin);
			fread(&pic->data[h*pic->width*comp + w*comp + 1], 1, 1, fin);
			fread(&pic->data[h*pic->width*comp + w*comp    ], 1, 1, fin);
			if (comp == 4)
				fread(&pic->data[h*pic->width*comp + w*comp + 3], 1, 1, fin);
		}
	}

	fclose(fin);
	return true;
}






/*
======================================
take a screen shot
======================================
*/
void Screenshot_TGA(char *filename, int type)
{
	int     i=0; 
	char	tganame[80]; 
	char	checkname[260];
	FILE	*f;

// 
// find a file name to save it to 
// 

	sprintf(checkname, "%s\\%s\\", rInfo->working_dir, "screenshots");
	ConfirmDir(checkname);


	if (!filename)
	{
		strcpy(tganame,"void00.tga");

		for (i=0 ; i<=99 ; i++) 
		{
			tganame[4] = i/10 + '0';
			tganame[5] = i%10 + '0';

			sprintf (checkname, "%s\\%s\\%s", rInfo->working_dir, "screenshots", tganame);

			f = fopen(checkname, "rb");
			if (!f)
				break;

			fclose(f);
		}

		if (i==100) 
		{
			ConPrint("too many screen shots - try deleteing or moving some\n"); 
			return;
		}
	}

	else
	{
		sprintf(checkname, "%s\\%s\\%s", rInfo->working_dir, "screenshots", filename);
		DefaultExtension(checkname, ".tga");
	}

// 
// get and save the pcx file 
// 

	tgadata_t pic;
	pic.width  = rInfo->width;
	pic.height = rInfo->height;

	if(type == GL_RGB)
	{
		pic.size = pic.width * pic.height * 4;
		pic.alpha = false;
	}
	else
	{
		pic.size = pic.width * pic.height * 4;
		pic.alpha = true;
	}
	pic.data   = (byte*) MALLOC(pic.size);

	if (pic.data == NULL) 
		Error("mem for writing pcx");

	if(type == GL_RGB)
		glReadPixels(0, 0, pic.width, pic.height, GL_RGB, GL_UNSIGNED_BYTE, pic.data);
	else
		glReadPixels(0, 0, pic.width, pic.height, GL_RGBA, GL_UNSIGNED_BYTE, pic.data);
	tga_write(checkname,&pic);
	free(pic.data);
}








