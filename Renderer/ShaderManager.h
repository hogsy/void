
#ifndef SHADERMANAGER_H
#define SHADERMANAGER_H

#include "Shader.h"


#define MAX_SHADERS		1024
#define MAX_SHADER_BINS	1024
typedef int hShader;


class CShaderManager
{
public:
	CShaderManager();
	~CShaderManager();


	void LoadWorld(CWorld *map);	// loads shaders needed for world
									// also loads the lightmaps
	void UnLoadWorld(void);

	void LoadBase(void);
	void UnLoadBase(void);


	void LoadShader(int bin, int index, const char *name);	// loads a specific shader, creates the default if it isn't found


	CShader *mShaders[MAX_SHADERS];

	int  BinInit(int num);
	void BinDestroy(int bin);

	int mNumLightmaps;
	int mLightmapBin;

private:

	struct shader_bin_t
	{
		shader_bin_t()
		{
			num = -1;
			indices = NULL;
		}

		hShader *indices;
		int		num;
	};

	shader_bin_t mBins[MAX_TEXTURE_BINS];

	int	mWorldBin;	// shader bin that holds the world shaders
	int	mBaseBin;	// shader bin that holds the base shaders

	int mNumShaders;
	void ParseShaders(const char *shaderfile);	// parse all shaders into memory
};


extern	CShaderManager	*g_pShaders;

#endif

