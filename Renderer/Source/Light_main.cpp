
#include "standard.h"
#include "light_main.h"
#include "Tex_image.h"

#define LIGHT_VERSION	1
#define LIGHT_ID		('v'<<24 | 'l'<<16 | 'i'<<8 | 't')

light_info_t light;
bool		 lights_available;

/*
-------------------------------------------------
load light info
-------------------------------------------------
*/
typedef struct
{
	int			 version;	// version of the light info
	unsigned int world_id;	// random number that must be the same as the world one
	unsigned int num_lightmaps;
	unsigned int num_lights;
} light_header_t;

void create_light_data(char *file)
{
	light.num_lights = light.num_lightmaps = 0;
	light.light_names = NULL;
	light.sector_lights = NULL;
	lights_available = false;

/*
	ConPrint("creating light data:  \n");


	FILE *fin = fopen(file, "rb");
	if (!fin)
		return;

	unsigned int i, s;
	fread(&i, sizeof(unsigned int), 1, fin);
	if (i != LIGHT_ID)
	{
		ConPrint("!!! Not a valid light data file !!!\n");
		fclose(fin);
		return;
	}


	light_header_t h;
	fread(&h, sizeof(light_header_t), 1, fin);
	if (world->header.light_id != h.world_id)
	{
		ConPrint("!!! Light data doesn't match world file !!!\n");
		fclose(fin);
		return;
	}

	if (h.version != LIGHT_VERSION)
	{
		ConPrint("!!! bad light data version !!!\n");
		fclose(fin);
		return;
	}


// light definitions
	light.num_lights = h.num_lights;
	light.lights = (light_t*)MALLOC(sizeof(light_t) * light.num_lights);
	if (!light.lights) FError("mem for light defs");
	for (i=0; i<light.num_lights; i++)
	{
		fread(&light.lights[i].origin, sizeof(vector_t), 1, fin);
		fread(&light.lights[i].color[0], 1, 3, fin);
	}

// sector light lists
	light.sector_lights = (sector_lights_t*)MALLOC(sizeof(sector_lights_t) * world->header.num_sectors);
	if (!light.sector_lights) FError("mem for sector lights");
	for (s=0; s<world->header.num_sectors; s++)
	{
	// number of lights in this sector
		fread(&light.sector_lights[s].num_lights, sizeof(unsigned int), 1, fin);

	// list of lights
		light.sector_lights[s].lights = (unsigned int*)MALLOC(sizeof(unsigned int)*light.sector_lights[s].num_lights);
		if (!light.sector_lights[s].lights) FError("mem for sector light list");

		fread(light.sector_lights[s].lights, sizeof(unsigned int), light.sector_lights[s].num_lights, fin);
	}


// lightmap data
	light.num_lightmaps = h.num_lightmaps;
	light.light_names = (GLuint*)MALLOC(sizeof(GLuint) * light.num_lightmaps);

	glGenTextures(light.num_lightmaps, light.light_names);

	short width, height;
	int miplevels, largestdim, tmps;
	unsigned char *data1, *data2, *tmpd;
	data1 = (unsigned char*) MALLOC(512*512*3);	// max lightmap size - 512x512 - 3 components
	data2 = (unsigned char*) MALLOC(512*512*3);	// max lightmap size - 512x512 - 3 components
	if (!data1 || !data2)	FError("mem for tmp lightmap data!");


	for (i=0; i<light.num_lightmaps; i++)
	{
		fread(&width , sizeof(short), 1, fin);
		fread(&height, sizeof(short), 1, fin);

		fread(data1, width*height, 3, fin);

	// figure out how many mip levels we need
		largestdim = width;
		if (height>width)	largestdim = height;

		for (miplevels=1, tmps=1; tmps < largestdim; tmps<<=1)
			miplevels++;


	// bind the base texture
		glBindTexture(GL_TEXTURE_2D, light.light_names[i]);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);	// lightmaps are clamped
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexImage2D(GL_TEXTURE_2D,
					 0,
					 GL_RGB8,
					 width,
					 height,
					 0,
					 GL_RGB,
					 GL_UNSIGNED_BYTE,
					 data1);

		tmpd = data2;
		data2 = data1;
		data1 = tmpd;

		//create all other mip levels
		for (int m=1; m<miplevels; m++)
		{
			width  /= 2;
			height /= 2;

			if (width == 0)
				width = 1;

			if (height == 0)
				height = 1;

			image_reduce24(data1, data2, width, height);

			glTexImage2D(GL_TEXTURE_2D,
						 m,
						 GL_RGB8,
						 width,
						 height,
						 0,
						 GL_RGB,
						 GL_UNSIGNED_BYTE,
						 data1);


			// switch data1 & data2
			tmpd = data2;
			data2 = data1;
			data1 = tmpd;
		}
	}

	free(data1);
	free(data2);
	fclose(fin);

	lights_available = true;

	ConPrint("OK\n");
*/
}


/*
-------------------------------------------------
destroy light info
-------------------------------------------------
*/
void destroy_light_data(void)
{
	ConPrint("destroying light data:  \n");
/*
	if (light.light_names)
	{
		glDeleteTextures(light.num_lightmaps, light.light_names);
		free(light.light_names);
	}

	if (light.sector_lights)
	{
		for (unsigned int s=0; s<world->header.num_sectors; s++)
			free(light.sector_lights[s].lights);

		free(light.sector_lights);
	}

	if (light.lights)
		free(light.lights);

	light.num_lights = light.num_lightmaps = 0;
	light.light_names = NULL;
	light.sector_lights = NULL;
*/
	ConPrint("OK\n");
}







