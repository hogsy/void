
#ifndef MDL_MAIN_H
#define MDL_MAIN_H

#include "Standard.h"
#include "I_file.h"
#include "Mdl_entry.h"



#ifdef _DEBUG
#define MAX_DRAWMODEL_ALLOCS 64
#define	DRAWMODELS_PER_ALLOC 4
#else
#define MAX_DRAWMODEL_ALLOCS 16
#define	DRAWMODELS_PER_ALLOC 16
#endif


class CModelManager : public I_Model
{
public:
	CModelManager();
	~CModelManager();

	/* Interface functions */

	// load a model into memory
	hMdl LoadModel(const char *model, hMdl index, CacheType cache);

	// add the model to the render cache
	void DrawModel(const R_EntState &state);
	void Purge(void);

	// unload models from memory
	void UnloadModel(CacheType cache, int index);
	void UnloadModelCache(CacheType cache);
	void UnloadModelAll(void);

	void GetInfo(R_EntState &state);


	void LoadSkins(void);
	void UnLoadSkins(void);


private:

	// struct to hold a list of models to be drawn
	typedef struct drawmodel_s
	{
		vector_t	origin;
		vector_t	angles;
		CacheType	cache;
		hMdl		index;
		int			skin;
		float		frame;

		drawmodel_s *next;
	} drawmodel_t;

	int		num_drawmodel_allocs;
	drawmodel_t	*drawmodel_allocs[MAX_DRAWMODEL_ALLOCS];
	drawmodel_t	*free_drawmodels;
	drawmodel_t *drawmodels;	// list of models to be drawn

	void drawmodelAlloc(void);
	drawmodel_t* drawmodelGet(void);
	void drawmodelRelease(drawmodel_t *d);

	bool ready;


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


