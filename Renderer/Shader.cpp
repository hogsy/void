
#include "Shader.h"
#include "Tex_image.h"


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
}


CShaderLayer::~CShaderLayer()
{
	if (mTextureNames)
	{
		delete (mTextureNames);
	}
}


/*
========
Load
========
*/
CShaderLayer::Load(CFileBuffer *layer, int &texindex)
{
	char token[1024];

	do
	{
		layer->GetToken(token, true);


		// only 1 texture for this layer
		if (stricmp(token, "map") == 0)
		{
			mNumTextures = 1;
			mTextureNames = new texname_t[1];
			if (!mTextureNames) FError("mem for texture names");

			layer->GetToken(token, false);
			if (token[0] == '\0')
				Error("error parsing shader file!!\n");

			if (stricmp(token, "$lightmap") == 0)
			{
				// fixme - we should already know what the index is
				mTextureNames[0].index = -1;
				mTextureNames[0].type = 1;
			}
			else
			{
				mTextureNames[0].type = 0;
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
		else if (stricmp(token, "animmap") == 0)
		{
			mNumTextures = 0;
			mTextureNames = new texname_t[8];
			if (!mTextureNames) FError("mem for texture names");

			layer->GetToken(token, false);
			mAnimFreq = atof(token);

			for (layer->GetToken(token, false); token[0]!= '\0'; layer->GetToken(token, false))
			{
				if (stricmp(token, "$lightmap") == 0)
				{
					// fixme - we should already know what the index is
					mTextureNames[mNumTextures].index = -1;
					mTextureNames[mNumTextures].type = 1;
				}
				else
				{
					mTextureNames[mNumTextures].type = 0;
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
		else if (stricmp(token, "blendfunc") == 0)
		{
			mDepthWrite = false;	// blended layers dont write

			layer->GetToken(token, false);
			if (stricmp(token, "add") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_ONE;
				mDstBlend = VRAST_DST_BLEND_ONE;
			}

			else if (stricmp(token, "filter") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_ONE;
				mDstBlend = VRAST_DST_BLEND_ONE;
			}

			else if (stricmp(token, "blend") == 0)
			{
				mSrcBlend = VRAST_SRC_BLEND_SRC_ALPHA;
				mDstBlend = VRAST_DST_BLEND_INV_SRC_ALPHA;
			}

			else
			{
				// an explicit blend func
				if ((stricmp(token, "gl_one") == 0) || (stricmp(token, "one") == 0))
					mSrcBlend = VRAST_SRC_BLEND_ONE;
				else if ((stricmp(token, "gl_zero") == 0) || (stricmp(token, "zero") == 0))
					mSrcBlend = VRAST_SRC_BLEND_ZERO;
				else if ((stricmp(token, "gl_dst_color") == 0) || (stricmp(token, "dst_color") == 0))
					mSrcBlend = VRAST_SRC_BLEND_DST_COLOR;
				else if ((stricmp(token, "gl_one_minus_dst_color") == 0) || (stricmp(token, "one_minus_dst_color") == 0))
					mSrcBlend = VRAST_SRC_BLEND_INV_DST_COLOR;
				else if ((stricmp(token, "gl_src_alpha") == 0) || (stricmp(token, "src_alpha") == 0))
					mSrcBlend = VRAST_SRC_BLEND_SRC_ALPHA;
				else if ((stricmp(token, "gl_one_minus_src_alpha") == 0) || (stricmp(token, "one_minus_src_alpha") == 0))
					mSrcBlend = VRAST_SRC_BLEND_INV_SRC_ALPHA;
				else
					mSrcBlend = VRAST_SRC_BLEND_NONE;

				layer->GetToken(token, false);
				if ((stricmp(token, "gl_one") == 0) || (stricmp(token, "one") == 0))
					mDstBlend = VRAST_DST_BLEND_ONE;
				else if ((stricmp(token, "gl_zero") == 0) || (stricmp(token, "zero") == 0))
					mDstBlend = VRAST_DST_BLEND_ZERO;
				else if ((stricmp(token, "gl_src_color") == 0) || (stricmp(token, "src_color") == 0))
					mDstBlend = VRAST_DST_BLEND_SRC_COLOR;
				else if ((stricmp(token, "gl_one_minus_src_color") == 0) || (stricmp(token, "one_minus_src_color") == 0))
					mDstBlend = VRAST_DST_BLEND_INV_SRC_COLOR;
				else if ((stricmp(token, "gl_src_alpha") == 0) || (stricmp(token, "src_alpha") == 0))
					mDstBlend = VRAST_DST_BLEND_SRC_ALPHA;
				else if ((stricmp(token, "gl_one_minus_src_alpha") == 0) || (stricmp(token, "one_minus_src_alpha") == 0))
					mDstBlend = VRAST_DST_BLEND_INV_SRC_ALPHA;
				else
					mDstBlend = VRAST_DST_BLEND_NONE;
			}
		}

		// depth funcs
		else if (stricmp(token, "depthfunc") == 0)
		{
			layer->GetToken(token, false);
			if (stricmp(token, "lequal") == 0)
				mDepthFunc = VRAST_DEPTH_LEQUAL;
			else if (stricmp(token, "equal") == 0)
				mDepthFunc = VRAST_DEPTH_EQUAL;
		}

		// depth write	- must be after the blend func or it will be overriden
		else if (stricmp(token, "depthwrite") == 0)
			mDepthWrite = true;

		// end of layer def
		else if (stricmp(token, "}") == 0)
			break;

		// unknown
		else
			ComPrintf("unsupported layer keyword %s\n", token);

	} while (1);

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
	if (mRefCount != 0)
		ComPrintf("*** deleting shader with refcounts ***\n");

	for (int l=0; l<mNumLayers; l++)
		delete (mLayers[l]);

	UnloadTextures();
}



/*
=========
Load
=========
*/
void CShader::Load(CFileBuffer *shader)
{
	char token[1024];

	do
	{
		shader->GetToken(token, true);


		// new layer
		if (stricmp(token, "{") == 0)
		{
			mLayers[mNumLayers] = new CShaderLayer();
			mLayers[mNumLayers]->Load(shader, mNumTextures);
		}

		// end of layer def
		else if (stricmp(token, "}") == 0)
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
		UnloadTextures();
}


/*
=========
AddRef
=========
*/
void CShader::LoadTextures(void)
{
	if (mRefCount == 0)
		return;

	if (mTextureBin != -1)
		ComPrintf("CShader::LoadTextures - textures already loaded\n");

	mTextureBin = g_pRast->TextureBinInit(mNumTextures);


	CImageReader *texReader = new CImageReader();

	int t=0;
	for (int l=0; l<mNumLayers; l++)
	{
		for (int tex=0; tex<mLayers[l]->mNumTextures; tex++)
		{
			if (mLayers[l]->mTextureNames[tex].type == 0)
			{
				if (!texReader->Read(mLayers[l]->mTextureNames[tex].filename))
					texReader->DefaultTexture();

				// create all mipmaps
				tex_load_t tdata;
				tdata.format = texReader->GetFormat();
				tdata.height = texReader->GetHeight();
				tdata.width  = texReader->GetWidth();
				tdata.mipmaps= texReader->GetNumMips();
				tdata.mipdata= texReader->GetMipData();
				tdata.mipmap = true;
				tdata.clamp  = false;

				int mipcount = tdata.mipmaps - 1;
				while (mipcount > 0)
				{
					texReader->ImageReduce(mipcount);
					mipcount--;
				}

		
				g_pRast->TextureLoad(mTextureBin, t, &tdata);
				t++;
			}
		}
	}

	delete texReader;
}


/*
=========
AddRef
=========
*/
void CShader::UnloadTextures(void)
{
	if (mTextureBin != -1)
		g_pRast->TextureBinDestroy(mTextureBin);
	mTextureBin = -1;
}



















