
#ifndef MDL_MD2_H
#define MDL_MD2_H

#include "Mdl_entry.h"


class CModelMd2 : public CModelCacheEntry
{
public:
	CModelMd2();
	~CModelMd2();

	void Draw(int skin, int fframe, int cframe, float frac);
	void LoadModel(const char *file);	// load a md2

private:

	typedef struct
	{
	   float s, t;
	   int vertex_index;
	} model_glcmd_t;


	void LoadFail(void);	// default model

	// frame data
	vector_t **frames;

	// the glcommand list
	void *cmds;
};


#endif

