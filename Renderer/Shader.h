
#ifndef SHADER_H
#define SHADER_H

#include "Standard.h"
#include "I_file.h"

#define MAX_ANIMATION		8
#define MAX_SHADER_LAYERS	8



class CShaderLayer
{
public:
	CShaderLayer();
	~CShaderLayer();

	Load(CFileBuffer *layer, int &texindex);



	// info about textures for all layers
	struct texname_t
	{
		int type;	// 0 = char texname, 1 = lightmap pointer
		int index;	// either index to CShader::mTextureBin or CShaderManager::mLightmapBin depending on type
		char filename[64];	// only relevant for non-lightmaps
	};

	int mNumTextures;	// number of textures in this layer - 1 except for animations
	texname_t *mTextureNames;	// the names of the textures

	float	mAnimFreq;	// animation frequency

	// blend funcs
	ESourceBlend	mSrcBlend;
	EDestBlend		mDstBlend;

	// depth func
	EDepthFunc		mDepthFunc;
	bool			mDepthWrite;
};




class CShader
{
public:
	CShader();
	~CShader();

	void Load(CFileBuffer *shader);


	void AddRef(void);
	void Release(void);

	void LoadTextures(void);
	void UnloadTextures(void);

private:



	int mRefCount;

	int		mNumLayers;
	CShaderLayer *mLayers[MAX_SHADER_LAYERS];


	int mNumTextures;	// total number of textures for all layers
	int mTextureBin;	// rasterizer texture bin

};



#endif

