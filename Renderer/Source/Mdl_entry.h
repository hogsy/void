
#ifndef MDL_ENTRY_H
#define MDL_ENTRY_H

#include <string.h>
#include "3dmath.h"

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

	void LoadSkins(void);
	void UnLoadSkins(void);


	int GetNumSkins(void)	{ return num_skins;		}
	int	GetNumFrames(void)	{ return num_frames;	}

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


#endif

