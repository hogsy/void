
#ifndef MDL_MAIN_H
#define MDL_MAIN_H

#include "Standard.h"
#include "I_file.h"



typedef struct
{
   float s, t;
   int vertex_index;
} model_glcmd_t;



class CModelCacheEntry
{
public:
	CModelCacheEntry(const char *file);
	~CModelCacheEntry();


	void Draw(int skin, float frame);
	bool IsFile(const char *file) { return (strcmp(file, modelfile)==0); }


private:

	void LoadModel(void);	// load a md2
	void LoadFail(void);	// default model

	// skin info
	int num_skins;
	int	skin_bin;		// rasterizer texture names
	char **skin_names;	// text texture names

	// filename of model
	char *modelfile;

	// frame data
	int		 num_frames;
	vector_t **frames;

	// the glcommand list
	void *cmds;
};


class CModelManager : public I_Model
{
public:
	CModelManager();
	~CModelManager();

	/* Interface functions */

	// load a model into memory
	hMdl LoadModel(const char *model, hMdl index, CacheType cache);

	// add the model to the render cache
	void DrawModel(hMdl index, CacheType cache, const R_EntState &state);

	// unload models from memory
	void UnloadModel(CacheType cache, int index);
	void UnloadModelCache(CacheType cache);
	void UnloadModelAll(void);

private:

	CModelCacheEntry *caches[MODEL_CACHE_NUM][MODEL_CACHE_SIZE];
};


extern	CModelManager	*g_pModel;





/*
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
*/

#endif


