#ifndef MDL_MD2_H
#define MDL_MD2_H

#include "I_clientRenderer.h"
#include "Mdl_entry.h"

class CModelMd2 : public CModelCacheEntry
{
public:
	CModelMd2();
	~CModelMd2();

	void Draw(int skin, int fframe, int cframe, float frac);
	
	// load a md2
	void LoadModel(I_FileReader * pFile, const char * szFileName);	
	
	// default model
	void LoadFail(void);	

private:

	typedef struct
	{
	   float s, t;
	   int vertex_index;
	} model_glcmd_t;

	typedef struct
	{
		vector_t v;
		byte norm;
	} model_vertex_t;

	// frame data
	model_vertex_t **frames;

	// the glcommand list
	void *cmds;
};


#endif

