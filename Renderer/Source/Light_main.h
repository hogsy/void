
#ifndef LIGHT_MAIN_H
#define LIGHT_MAIN_H



typedef struct
{
	vector_t origin;
	unsigned char color[3];
// FIXME - intensity and other light info
} light_t;


typedef struct
{
	unsigned int num_lights;
	unsigned int *lights;
} sector_lights_t;


typedef struct
{
	unsigned int num_lights;
	unsigned int num_lightmaps;

	GLuint	*light_names;	// gl texture names
	light_t	*lights;

	sector_lights_t *sector_lights;
} light_info_t;


void create_light_data(char *file);
void destroy_light_data(void);


#endif





