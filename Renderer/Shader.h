
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

	Parse(CFileBuffer *layer, int &texindex);
	void Default(const char *name, int &texindex);



	// info about textures for all layers
	struct texname_t
	{
		int index;	// either index to CShader::mTextureBin or -1 means lightmap
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
	CShader(const char *name);
	~CShader();

	void Parse(CFileBuffer *shader);
	void Default(void);

	bool IsShader(const char *s) { return (stricmp(s, mName)==0); }

	void AddRef(void);
	void Release(void);

	void LoadTextures(void);
	void UnLoadTextures(void);

private:


	char mName[64];

	int mRefCount;

	int		mNumLayers;
	CShaderLayer *mLayers[MAX_SHADER_LAYERS];


	int mNumTextures;	// total number of textures for all layers
	int mTextureBin;	// rasterizer texture bin

};



#endif

