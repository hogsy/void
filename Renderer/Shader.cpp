

#ifdef RENDERER
#include "Standard.h"
#include "Tex_image.h"
#else
#include "Com_defs.h"
#include "Com_Vector.h"
#include "Rast_main.h"
#include "../vbsp/source/std_lib.h"
#endif

#include "I_file.h"
#include "Shader.h"


/*
===================================================================================================
CShaderLayer
===================================================================================================
*/
CShaderLayer::CShaderLayer()
{
	mNumTextures = 0;
	mTextureNames = NULL;
	mSrcBlend  = VRAST_SRC_BLEND_NONE;
	mDstBlend  = VRAST_DST_BLEND_NONE;
	mDepthFunc = VRAST_DEPTH_LEQUAL;
	mDepthWrite = true;
	mAnimFreq = 0;
	mTexGen = TEXGEN_BASE;
	mIsLight = false;
}


CShaderLayer::~CShaderLayer()
{
	if (mTextureNames)
	{
		delete [] mTextureNames;
	}
}


/*
========
Parse
========
*/
void CShaderLayer::Parse(CFileBuffer *layer, int &texindex)
{
	char token[1024];

	do
	{
		layer->GetToken(token, true);

		// tex coord generation
		if (_stricmp(token, "tcgen") == 0)
		{
			layer->GetToken(token, false);
			if (_stricmp(token, "base") == 0)
				mTexGen = TEXGEN_BASE;
			else if (_stricmp(token, "lightmap") == 0)
				mTexGen = TEXGEN_LIGHT;
			else if (_stricmp(token, "vector") == 0)
			{
				mTexGen = TEXGEN_VECTOR;

				layer->GetToken(token, false);	// '('
				layer->GetToken(token, false);
				mTexVector[0].x = atof(token);
				layer->GetToken(token, false);
				mTexVector[0].y = atof(token);
				layer->GetToken(token, false);
				mTexVector[0].z = atof(token);
				layer->GetToken(token, false);	// ')'

				layer->GetToken(token, false);	// '('
				layer->GetToken(token, false);
				mTexVector[1].x = atof(token);
				layer->GetToken(token, false);
				mTexVector[1].y = atof(token);
				layer->GetToken(token, false);
				mTexVector[1].z = atof(token);
				layer->GetToken(token, false);	// ')'
			}



		}


		// only 1 texture for this layer
		if (_stricmp(token, "map") == 0)
		{
			mNumTextures = 1;
			mTextureNames = new texname_t[1];
			if (!mTextureNames)	FError("mem for texture names");

			layer->GetToken(token, false);
			if (token[0] == '\0')
				Error("error parsing shader file!!\n");

			if (_stricmp(token, "$lightmap") == 0)
			{
				mIsLight = true;
				mTextureNames[0].index = -1;
			}
			else
			{
				mTextureNames[0].index = texindex++;
				strcpy(mTextureNames[0].filename, token);
				// strip the extension off the filename
				for (int c=strlen(mTextureNames[0].filename); c>=0; c--)
				{
					if (mTextureNames[0].filename[c] == '.')
					{
						mTextureNames[0].filename[c] = '\0';
						break;
					}
				}
			}
		}

		// animation
		else if (_stricmp(token, "animmap") == 0)
		{
			mNumTextures = 0;
			mTextureNames = new texname_t[8];
			if (!mTextureNames) FError("mem for texture names");

			layer->GetToken(token, false);
			mAnimFreq = atof(token);

			for (layer->GetToken(token, false); token[0]!= '\0'; layer->GetToken(token, false))
			{
				if (_stricmp(token, "$lightmap") == 0)
					mTextureNames[mNumTextures].index = -1;
				else
				{
					mTextureNames[mNumTextures].index = texindex++;
					strcpy(mTextureNames[0].filename, token);
					// strip the extension off the filename
					for (int c=strlen(mTextureNames[0].filename); c>=0; c--)
					{
						if (mTextureNames[0].filename[c] == '.')
						{
							mTextureNames[0].filename[c] = '\0';
							break;
						}
					}
				}
				mNumTextures++;
			}
		}

		// blend funcs
		else if (_stricmp(token, "blendfunc") == 0)
		{
			mDepthWrite = false;	// blended layers dont write

			layer->GetToken(token, false);
			if (_stricmp(token, "add") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_ONE;
				mDstBlend = VRAST_DST_BLEND_ONE;
			}

			else if (_stricmp(token, "filter") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_ONE;
				mDstBlend = VRAST_DST_BLEND_ONE;
			}

			else if (_stricmp(token, "blend") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_SRC_ALPHA;
				mDstBlend = VRAST_DST_BLEND_INV_SRC_ALPHA;
			}

			else
			{
				// an explicit blend func
				if ((_stricmp(token, "gl_one") == 0) || (_stricmp(token, "one") == 0))
					mSrcBlend = VRAST_SRC_BLEND_ONE;
				else if ((_stricmp(token, "gl_zero") == 0) || (_stricmp(token, "zero") == 0))
					mSrcBlend = VRAST_SRC_BLEND_ZERO;
				else if ((_stricmp(token, "gl_dst_color") == 0) || (_stricmp(token, "dst_color") == 0))
					mSrcBlend = VRAST_SRC_BLEND_DST_COLOR;
				else if ((_stricmp(token, "gl_one_minus_dst_color") == 0) || (_stricmp(token, "one_minus_dst_color") == 0))
					mSrcBlend = VRAST_SRC_BLEND_INV_DST_COLOR;
				else if ((_stricmp(token, "gl_src_alpha") == 0) || (_stricmp(token, "src_alpha") == 0))
					mSrcBlend = VRAST_SRC_BLEND_SRC_ALPHA;
				else if ((_stricmp(token, "gl_one_minus_src_alpha") == 0) || (_stricmp(token, "one_minus_src_alpha") == 0))
					mSrcBlend = VRAST_SRC_BLEND_INV_SRC_ALPHA;
				else
					mSrcBlend = VRAST_SRC_BLEND_NONE;

				layer->GetToken(token, false);
				if ((_stricmp(token, "gl_one") == 0) || (_stricmp(token, "one") == 0))
					mDstBlend = VRAST_DST_BLEND_ONE;
				else if ((_stricmp(token, "gl_zero") == 0) || (_stricmp(token, "zero") == 0))
					mDstBlend = VRAST_DST_BLEND_ZERO;
				else if ((_stricmp(token, "gl_src_color") == 0) || (_stricmp(token, "src_color") == 0))
					mDstBlend = VRAST_DST_BLEND_SRC_COLOR;
				else if ((_stricmp(token, "gl_one_minus_src_color") == 0) || (_stricmp(token, "one_minus_src_color") == 0))
					mDstBlend = VRAST_DST_BLEND_INV_SRC_COLOR;
				else if ((_stricmp(token, "gl_src_alpha") == 0) || (_stricmp(token, "src_alpha") == 0))
					mDstBlend = VRAST_DST_BLEND_SRC_ALPHA;
				else if ((_stricmp(token, "gl_one_minus_src_alpha") == 0) || (_stricmp(token, "one_minus_src_alpha") == 0))
					mDstBlend = VRAST_DST_BLEND_INV_SRC_ALPHA;
				else
					mDstBlend = VRAST_DST_BLEND_NONE;
			}
		}

		// depth funcs
		else if (_stricmp(token, "depthfunc") == 0)
		{
			layer->GetToken(token, false);
			if (_stricmp(token, "lequal") == 0)
				mDepthFunc = VRAST_DEPTH_LEQUAL;
			else if (_stricmp(token, "equal") == 0)
				mDepthFunc = VRAST_DEPTH_EQUAL;
		}

		// depth write	- must be after the blend func or it will be overriden
		else if (_stricmp(token, "depthwrite") == 0)
			mDepthWrite = true;

		// end of layer def
		else if (_stricmp(token, "}") == 0)
			break;

		// unknown
		else
			ComPrintf("unsupported layer keyword %s\n", token);

	} while (1);

}


/*
=========
Default
=========
*/
void CShaderLayer::Default(const char *name, int &texindex)
{
	mNumTextures = 1;
	mTextureNames = new texname_t[1];
	if (!mTextureNames) FError("mem for texture names");


	if (_stricmp(name, "$lightmap") == 0)
	{
		mTextureNames[0].index = -1;
		mSrcBlend = VRAST_SRC_BLEND_ZERO;
		mDstBlend = VRAST_DST_BLEND_SRC_COLOR;
		mTexGen = TEXGEN_LIGHT;
	}
	else
	{
		mTextureNames[0].index = texindex++;
		strcpy(mTextureNames[0].filename, name);
	}
}


/*
===================================================================================================
CShader
===================================================================================================
*/
CShader::CShader(const char *name)
{
	strcpy(mName, name);
	mTextureBin = -1;
	mNumTextures = 0;
	mNumLayers = 0;
	mRefCount = 0;
}


CShader::~CShader()
{
	for (int l=0; l<mNumLayers; l++)
		delete mLayers[l];

	UnLoadTextures();
}



/*
=========
Parse
=========
*/
void CShader::Parse(CFileBuffer *shader)
{
	char token[1024];

	do
	{
		shader->GetToken(token, true);


		// new layer
		if (_stricmp(token, "{") == 0)
		{
			mLayers[mNumLayers] = new CShaderLayer();
			mLayers[mNumLayers]->Parse(shader, mNumTextures);
			mNumLayers++;
		}

		// end of shader def
		else if (_stricmp(token, "}") == 0)
			break;

		// unknown
		else
			ComPrintf("unsupported shader keyword %s\n", token);

	} while (1);
}


/*
=========
AddRef
=========
*/
void CShader::AddRef(void)
{
	if (mRefCount == 0)
		LoadTextures();

	mRefCount++;
}


/*
=========
Release
=========
*/
void CShader::Release(void)
{
	mRefCount--;
	if (mRefCount == 0)
		UnLoadTextures();
}


/*
=========
LoadTextures
=========
*/
void CShader::LoadTextures(void)
{
#ifdef RENDERER
	if (mTextureBin != -1)
		return;

	mTextureBin = g_pRast->TextureBinInit(mNumTextures);

	TextureData	 tData;
	tData.bMipMaps = true;
	tData.bClamped = false;

	static char texname[COM_MAXPATH];


	int t=0;
	for (int l=0; l<mNumLayers; l++)
	{
		for (int tex=0; tex<mLayers[l]->mNumTextures; tex++)
		{
			if (mLayers[l]->mTextureNames[tex].index != -1)
			{
				sprintf(texname,"%s/%s","textures",mLayers[l]->mTextureNames[tex].filename);

				if (!CImageReader::GetReader().Read(texname, tData))
					CImageReader::GetReader().DefaultTexture(tData);

				g_pRast->TextureLoad(mTextureBin, t, tData);
				t++;
			}
		}
	}
#endif
}


/*
=========
UnLoadTextures
=========
*/
void CShader::UnLoadTextures(void)
{
#ifdef RENDERER
	if (mTextureBin != -1)
		g_pRast->TextureBinDestroy(mTextureBin);
	mTextureBin = -1;
#endif
}


/*
=========
Default
=========
*/
void CShader::Default(void)
{
	mNumLayers = 2;
	mLayers[0] = new CShaderLayer();
	mLayers[1] = new CShaderLayer();

	mLayers[0]->Default(mName, mNumTextures);
	mLayers[1]->Default("$lightmap", mNumTextures);
}



















