
#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "Shader.h"


#define MAX_SHADERS	1024
typedef int hShader;


class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();


	void LoadShaders(CWorld *map);	// loads textures needed for world shaders
	hShader LoadShader(char *name);	// loads a specific shader, creates the default if it isn't found



	CShader *mShaders[MAX_SHADERS];

	int mNumLightmaps;
	int mLightmapBin;


private:

	int mNumShaders;
	void ParseShaders(char *shaderfile);	// parse all shaders into memory
};


extern	CShaderManager	*g_pShaders;

#endif

