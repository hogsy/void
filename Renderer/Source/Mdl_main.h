
#ifndef MDL_MAIN_H
#define MDL_MAIN_H

#include "Standard.h"

typedef struct
{
   float s, t;
   int vertex_index;
} model_glcmd_t;



typedef struct
{
   vector_t *vertices;
   // FIXME - lightnormal
} model_frame_t;



typedef struct
{
	int				num_skins;
	int				num_frames;

	void			*cmds;		// the glcommand list
	model_frame_t	*frames;
	int				skin_bin;	// rasterizer texture bin for skins
} model_t;


void model_load_map(void);
void model_destroy_map(void);
void model_draw(int mindex, float frame);

#endif


