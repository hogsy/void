
#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "Shader.h"


#define MAX_SHADERS	1024
typedef int hShader;


class CShaderManager
{
public:
	CShaderManager(char *indexfile);
	~CShaderManager();


	hShader LoadShader(char *name);


	CShader *mShaders[MAX_SHADERS];

private:

	ParseShaders(char *indexfile);	// parse all shaders into memory

	int mNumLightmaps;
	int mLightmapBin;
};



#endif

