

#ifdef RENDERER
#include "Standard.h"
#include "Tex_main.h"
#include "Tex_image.h"
#else
#include "Com_defs.h"
#include "Com_Vector.h"
#include "Rast_main.h"
#include "../Devvoid/Std_lib.h"
#endif

#include "I_file.h"
#include "Shader.h"
#include "Tex_image.h"
#include "Com_trace.h"


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
	mAnimFreq = 0;
	mTexGen = TEXGEN_BASE;
	mIsLight = false;
	mbMipMap = true;
	mAlphaGen.func = ALPHAGEN_IDENTITY;
	mHeadTCMod = new CTCModHead();
	mTextureClamp = false;
}


CShaderLayer::~CShaderLayer()
{
	if (mTextureNames)
	{
		delete [] mTextureNames;
	}
	
	delete mHeadTCMod;
}


/*
========
Parse
========
*/
void CShaderLayer::Parse(I_FileReader *layer)
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
			else if (_stricmp(token, "environment") == 0)
				mTexGen = TEXGEN_ENVIRONMENT;
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

		else if (_stricmp(token, "tcmod") == 0)
		{
			layer->GetToken(token, false);

			if (_stricmp(token, "scroll") == 0)
			{
				float x, y;
				layer->GetToken(token, false);
				x = (float)atof(token);
				layer->GetToken(token, false);
				y = (float)atof(token);

				mHeadTCMod->Add(new CTCModScroll(x, y));
			}

			else if (_stricmp(token, "scale") == 0)
			{
				float x, y;
				layer->GetToken(token, false);
				x = (float)atof(token);
				layer->GetToken(token, false);
				y = (float)atof(token);

				mHeadTCMod->Add(new CTCModScale(x, y));
			}

			else
				ComPrintf("Unsupported tcmod - %s\n", token);
		}


		// only 1 texture for this layer
		else if (_stricmp(token, "map") == 0)
		{
			mNumTextures = 1;
			mTextureNames = new texname_t[1];
			if (!mTextureNames)	FError("mem for texture names");

			layer->GetToken(token, false);
			if (token[0] == '\0')
				Error("error parsing shader file!!\n");

			if (_stricmp(token, "$lightmap") == 0)
			{
				mTextureClamp = true;
				mIsLight = true;
				mTextureNames[0].index = -1;
				mTexGen = TEXGEN_LIGHT;
			}
			else
			{
				mTextureNames[0].index = 0;
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

		// only 1 texture for this layer - clamped
		else if (_stricmp(token, "clampmap") == 0)
		{
			mTextureClamp = true;
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
				mTexGen = TEXGEN_LIGHT;
			}
			else
			{
				mTextureNames[0].index = 0;
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
			mTextureNames = new texname_t[MAX_ANIMATION];
			if (!mTextureNames) FError("mem for texture names");

			layer->GetToken(token, false);
			mAnimFreq = atof(token);

			for (layer->GetToken(token, false); token[0]!= '\0' && mNumTextures <= MAX_ANIMATION; )
			{
				if (_stricmp(token, "$lightmap") == 0)
					mTextureNames[mNumTextures].index = -1;
				else
				{
					mTextureNames[mNumTextures].index = 0;
					strcpy(mTextureNames[mNumTextures].filename, token);
					// strip the extension off the filename
					for (int c=strlen(mTextureNames[mNumTextures].filename); c>=0; c--)
					{
						if (mTextureNames[mNumTextures].filename[c] == '.')
						{
							mTextureNames[mNumTextures].filename[c] = '\0';
							break;
						}
					}
				}
				layer->GetToken(token, false);
				mNumTextures++;
			}
		}

		// blend funcs
		else if (_stricmp(token, "blendfunc") == 0)
		{
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
				mSrcBlend = VRAST_SRC_BLEND_ZERO;
				mDstBlend = VRAST_DST_BLEND_SRC_COLOR;
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

		// alphagen
		else if (_stricmp(token, "alphagen") == 0)
		{
			layer->GetToken(token, false);
			if (_stricmp(token, "identity") == 0)
				mAlphaGen.func = ALPHAGEN_IDENTITY;
			else if (_stricmp(token, "console") == 0)
				mAlphaGen.func = ALPHAGEN_CONSOLE;
			else if (_stricmp(token, "wave") == 0)
				mAlphaGen.func = ALPHAGEN_WAVE;
		}


		// dont create mipmaps
		else if (_stricmp(token, "nomipmap") == 0)
			mbMipMap = false;

		// end of layer def
		else if (_stricmp(token, "}") == 0)
			break;

		// unknown
		else
			ComPrintf("unsupported shader layer keyword %s\n", token);

	} while (1);

}


/*
=========
Default
=========
*/
void CShaderLayer::Default(const char *name)
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
		mIsLight = true;
		mTextureClamp = true;
	}
	else
	{
		mTextureNames[0].index = 0;
		strcpy(mTextureNames[0].filename, name);
	}
}


/*
===========
GetDims
===========
*/
void CShaderLayer::GetDims(int &width, int &height)
{
	TextureData		tdata;
	CImageReader::GetReader().Read(mTextureNames[0].filename, tdata);
	CImageReader::GetReader().FreeMipData();

	width = tdata.width;
	height= tdata.height;
}



/*
===================================================================================================
CShader
===================================================================================================
*/
CShader::CShader(const char *name)
{
	strcpy(mName, name);
	mNumLayers = 0;
	mRefCount = 0;
	mPass = 4;
	mDepthFunc = VRAST_DEPTH_LEQUAL;
	mDepthWrite = true;
	mOriginTexture = false;
	mSurfaceFlags = 0;
	mContentFlags = CONTENTS_SOLID;
}


CShader::~CShader()
{
	UnLoadTextures();

	for (int l=0; l<mNumLayers; l++)
		delete mLayers[l];
}



/*
=========
Parse
=========
*/
void CShader::Parse(I_FileReader *shader)
{
	char token[1024];

	do
	{
		shader->GetToken(token, true);

		if (token[0] == '\0')
		{
			ComPrintf("unexpected end of shader file\n");
			break;
		}


		// new layer
		if (_stricmp(token, "{") == 0)
		{
			mLayers[mNumLayers] = new CShaderLayer();
			mLayers[mNumLayers]->Parse(shader);
			mNumLayers++;
		}

		else if (_stricmp(token, "origintexture") == 0)
			mOriginTexture = true;

		// sky brushes are only rendered through skyview shaders
		else if ((_stricmp(token, "sky") == 0) || (_stricmp(token, "skybrush") ==0))
		{
			mPass = 0;
			mContentFlags |= CONTENTS_SKY;
		}

		// surface allows viewing of the sky - makes whole brush be able to view sky
		else if (_stricmp(token, "skyview") == 0)
		{
			mSurfaceFlags |= SURF_SKYVIEW;
			mContentFlags |= CONTENTS_SKYVIEW;
		}


		else if (_stricmp(token, "invisible") == 0)
		{
			// doesnt effect contents
			mSurfaceFlags |= CONTENTS_INVISIBLE;
		}

		else if (_stricmp(token, "nonsolid") == 0)
		{
			// brushes must be explicitly made nonsolid
			mContentFlags &= ~CONTENTS_SOLID;
		}


		// depth funcs
		else if (_stricmp(token, "depthfunc") == 0)
		{
			shader->GetToken(token, false);
			if (_stricmp(token, "lequal") == 0)
				mDepthFunc = VRAST_DEPTH_LEQUAL;
			else if (_stricmp(token, "equal") == 0)
				mDepthFunc = VRAST_DEPTH_EQUAL;
			else if (_stricmp(token, "always") == 0)
				mDepthFunc = VRAST_DEPTH_ALWAYS;
			else if (_stricmp(token, "none") == 0)
				mDepthFunc = VRAST_DEPTH_NONE;
		}

		// depth write	- must be after the blend func or it will be overriden
		else if (_stricmp(token, "depthwrite") == 0)
			mDepthWrite = true;


		// editor image
		else if (_stricmp(token, "qer_editorimage") == 0)
			shader->GetToken(token, false);

		// end of shader def
		else if (_stricmp(token, "}") == 0)
			break;

		// unknown
		else
			ComPrintf("unsupported shader keyword %s\n", token);

	} while (1);


	// if first layer isn't solid, whole shader is transparent
	if ((mNumLayers == 0) ||
		(mLayers[0]->mSrcBlend != VRAST_SRC_BLEND_NONE) ||
		(mLayers[0]->mDstBlend != VRAST_DST_BLEND_NONE) ||
		(mLayers[0]->mAlphaGen.func != ALPHAGEN_IDENTITY))
	{
		mSurfaceFlags |= CONTENTS_TRANSPARENT;
		mPass = 6;
	}
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
===========
GetDims
===========
*/
void CShader::GetDims(int &width, int &height)
{
	if (mLayers[0]->mIsLight)
		mLayers[1]->GetDims(width, height);
	else
		mLayers[0]->GetDims(width, height);
}


/*
===========
HasLightmap
===========
*/
bool CShader::HasLightmap(void)
{
	for (int l=0; l<mNumLayers; l++)
	{
		if (mLayers[l]->mTextureNames[0].index == -1)
			return true;
	}

	return false;
}


/*
=========
LoadTextures
=========
*/
void CShader::LoadTextures(void)
{
#ifdef RENDERER

	TextureData	 tData;
	tData.bMipMaps = true;
	tData.bClamped = false;

	for (int l=0; l<mNumLayers; l++)
	{
		for (int tex=0; tex<mLayers[l]->mNumTextures; tex++)
		{
			if (mLayers[l]->mTextureNames[tex].index != -1)
			{
				tData.bMipMaps = mLayers[l]->mbMipMap;
				mLayers[l]->mTextureNames[tex].index = g_pTex->Load(mLayers[l]->mTextureNames[tex].filename, tData);
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
	for (int l=0; l<mNumLayers; l++)
	{
		for (int tex=0; tex<mLayers[l]->mNumTextures; tex++)
		{
			if (mLayers[l]->mTextureNames[tex].index != -1)
			{
				g_pTex->UnLoad(mLayers[l]->mTextureNames[tex].index);
			}
		}
	}
#endif
}


/*
=========
Default
=========
*/
void CShader::Default(bool lightmap)
{
	mNumLayers = 1;
	mLayers[0] = new CShaderLayer();
	mLayers[0]->Default(mName);

	if (lightmap)
	{
		mNumLayers++;
		mLayers[1] = new CShaderLayer();
		mLayers[1]->Default("$lightmap");
	}
}
















