#include "Standard.h"
#include "Shader.h"
#include "ShaderManager.h"

// to evaluate console alphagen
#include "Con_main.h"

extern	CVar g_varFullbright;

// for sky texgen
extern const CCamera * camera;

/*
=======================================
Constructor 
=======================================
*/
CRasterizer::CRasterizer() :	m_cWndX("r_wndx","80",CVAR_INT,CVAR_ARCHIVE),
								m_cWndY("r_wndy","40",CVAR_INT,CVAR_ARCHIVE)
{	
	I_Console::GetConsole()->RegisterCVar(&m_cWndX);
	I_Console::GetConsole()->RegisterCVar(&m_cWndY);

	mMaxElements = mNumElements = 0;
	mMaxIndices = mNumIndices = 0;
	mColor = 0xffffffff;
	mShader = NULL;
	mTexDef = NULL;
	mLightDef = NULL;
	mUseTexDef = false;
	mUseLightDef = false;
}



void CRasterizer::PolyStart(EPolyType type)
{
	mType = type;
	mNumIndices = 0;
	mNumElements = 0;
}


void CRasterizer::PolyEnd(void)
{
	int t;
	int num;

	// convert the type into triangles
	switch (mType)
	{
	case VRAST_TRIANGLE_FAN:
		num = mNumElements-2;
		for (t=0; t<num; t++)
		{
			mIndices[t*3 + 0] = 0;
			mIndices[t*3 + 1] = t + 1;
			mIndices[t*3 + 2] = t + 2;
		}
		mNumIndices = num*3;
		break;

	case VRAST_TRIANGLE_STRIP:
		num = mNumElements-2;
		for (t=0; t<num; t++)
		{
			if (!(t%2))
			{
				mIndices[t*3 + 0] = t;
				mIndices[t*3 + 1] = t+1;
				mIndices[t*3 + 2] = t+2;
			}
			else
			{
				mIndices[t*3 + 0] = t;
				mIndices[t*3 + 1] = t+2;
				mIndices[t*3 + 2] = t+1;
			}
		}
		mNumIndices = num*3;
		break;

	case VRAST_QUADS:
		num = mNumElements / 4;
		mNumIndices = 0;
		for (t=0; t<num; t++)
		{
			mIndices[mNumIndices + 0] = t*4;
			mIndices[mNumIndices + 1] = t*4+1;
			mIndices[mNumIndices + 2] = t*4+2;

			mIndices[mNumIndices + 3] = t*4;
			mIndices[mNumIndices + 4] = t*4+2;
			mIndices[mNumIndices + 5] = t*4+3;

			mNumIndices += 6;
		}

		break;
	}

	if (mMaxElements < mNumElements)
		mMaxElements = mNumElements;
	if (mMaxIndices < mNumIndices)
		mMaxIndices = mNumIndices;

	// 3 indices per triangle
	mTrisDrawn += mNumIndices / 3;


	// generate texture coord data
	// put it in separate arrays cause we might need to do tcmod's and don't want to have to re-evaluate them
	if (mUseTexDef)
	{
		for (int i=0; i<mNumElements; i++)
		{
			mTexCoords[i][0] =	 mVerts[i].pos[0] * mTexDef->vecs[0][0] +
								 mVerts[i].pos[1] * mTexDef->vecs[0][1] +
								 mVerts[i].pos[2] * mTexDef->vecs[0][2] +
								 mTexDef->vecs[0][3];

			mTexCoords[i][1] =	 mVerts[i].pos[0] * mTexDef->vecs[1][0] +
								 mVerts[i].pos[1] * mTexDef->vecs[1][1] +
								 mVerts[i].pos[2] * mTexDef->vecs[1][2] +
								 mTexDef->vecs[1][3];
		}
	}

	if (mUseLightDef)
	{
		for (int i=0; i<mNumElements; i++)
		{
			mLightCoords[i][0] =	 mVerts[i].pos[0] * mLightDef->vecs[0][0] +
									 mVerts[i].pos[1] * mLightDef->vecs[0][1] +
									 mVerts[i].pos[2] * mLightDef->vecs[0][2] +
									 mLightDef->vecs[0][3];

			mLightCoords[i][1] =	 mVerts[i].pos[0] * mLightDef->vecs[1][0] +
									 mVerts[i].pos[1] * mLightDef->vecs[1][1] +
									 mVerts[i].pos[2] * mLightDef->vecs[1][2] +
									 mLightDef->vecs[1][3];
		}
	}

	for (int i=0; i<mShader->mNumLayers; i++)
		DrawLayer(i);

	mUseTexDef = false;
	mUseLightDef = false;
};


void CRasterizer::DrawLayer(int l)
{
	CShaderLayer *layer = mShader->mLayers[l];
	int i;

	if (layer->mIsLight && (g_varFullbright.bval || !world->light_size))
		return;

	if (!layer->mNumTextures)
		return;

	// generate alpha component
	switch (mShader->mLayers[l]->mAlphaGen.func)
	{
	case ALPHAGEN_IDENTITY:
		for (i=0; i<mNumElements; i++)
			mVerts[i].color |= 0xff000000;
		break;

	case ALPHAGEN_CONSOLE:
		for (i=0; i<mNumElements; i++)
		{
			int a = (int) (mConAlphaTop - (((g_rInfo.height-mVerts[i].pos[1]) * 2.0f / g_rInfo.height) * (mConAlphaTop-mConAlphaBot)));
			if (a < 0) a = 0;
			if (a > 255) a = 255;

			mVerts[i].color = (mVerts[i].color & 0x00ffffff) |  (a<<24);
		}
		break;
	}


	// create texture coords
	switch (mShader->mLayers[l]->mTexGen)
	{
	case TEXGEN_BASE:
		if (!mUseTexDef)
			break;

		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].tex1[0] = mTexCoords[i][0];
			mVerts[i].tex1[1] = mTexCoords[i][1];
		}
		break;


	case TEXGEN_SKY:
		if (!mUseTexDef)
			break;

		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].tex1[0] = mTexCoords[i][0] - camera->origin.x * mTexDef->vecs[0][0]
												 - camera->origin.y * mTexDef->vecs[0][1]
												 - camera->origin.z * mTexDef->vecs[0][2];

			mVerts[i].tex1[1] = mTexCoords[i][1] - camera->origin.x * mTexDef->vecs[1][0]
												 - camera->origin.y * mTexDef->vecs[1][1]
												 - camera->origin.z * mTexDef->vecs[1][2];
		}
		break;



	case TEXGEN_LIGHT:
		if (!mUseLightDef)
			break;

		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].tex1[0] = mLightCoords[i][0];
			mVerts[i].tex1[1] = mLightCoords[i][1];
		}
		break;

	case TEXGEN_VECTOR:

		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].tex1[0] =	mVerts[i].pos[0] * layer->mTexVector[0].x +
								mVerts[i].pos[1] * layer->mTexVector[0].y +
								mVerts[i].pos[2] * layer->mTexVector[0].z;

			mVerts[i].tex1[0] =	mVerts[i].pos[0] * layer->mTexVector[1].x +
								mVerts[i].pos[1] * layer->mTexVector[1].y +
								mVerts[i].pos[2] * layer->mTexVector[1].z;
		}
		break;
	}

	// tcmod's
	for (i=0; i<mNumElements; i++)
		mShader->mLayers[l]->mHeadTCMod->Evaluate(mVerts[i].tex1[0], mVerts[i].tex1[1], GetCurTime());

	// set texture
	if (layer->mIsLight)
		TextureSet(g_pShaders->mLightmapBin, mLightDef->texture); //layer->mTextureNames[0].index);
	else
	{
		if (mShader->mLayers[l]->mNumTextures == 1)
			TextureSet(mShader->mTextureBin, layer->mTextureNames[0].index);
		else
		{
			int texture = (int)(mShader->mLayers[l]->mAnimFreq * mShader->mLayers[l]->mNumTextures * GetCurTime());
			texture %= mShader->mLayers[l]->mNumTextures;
			TextureSet(mShader->mTextureBin, layer->mTextureNames[texture].index);
		}
	}

	// texture clamping
	TextureClamp(layer->mTextureClamp);

	// blendfunc
	BlendFunc(layer->mSrcBlend, layer->mDstBlend);

	PolyDraw();
}


void CRasterizer::PolyVertexf(vector_t &vert)
{
	mVerts[mNumElements].pos[0] = vert.x;
	mVerts[mNumElements].pos[1] = vert.y;
	mVerts[mNumElements].pos[2] = vert.z;
	mVerts[mNumElements].color = mColor;
	mNumElements++;
};

void CRasterizer::PolyVertexi(int x, int y)
{
	mVerts[mNumElements].pos[0] = x;
	mVerts[mNumElements].pos[1] = y;
	mVerts[mNumElements].pos[2] = 0;
	mVerts[mNumElements].color = mColor;
	mNumElements++;
};


// explicit coords go right into the array
void CRasterizer::PolyTexCoord(float s, float t)
{
	mVerts[mNumElements].tex1[0] = s;
	mVerts[mNumElements].tex1[1] = t;
};


void CRasterizer::PolyLightCoord(float s, float t)
{
	mVerts[mNumElements].tex1[0] = s;
	mVerts[mNumElements].tex1[1] = t;
};


void CRasterizer::PolyColor3f(float r, float g, float b)
{
	// strip everything but the alpha component
	mColor  = mColor & 0xff000000;
	mColor |= (byte)(r*255);
	mColor |= (byte)(g*255) << 8;
	mColor |= (byte)(b*255) << 16;
};

void CRasterizer::PolyColor4f(float r, float g, float b, float a)
{
	mColor  = (byte)(r*255);
	mColor |= (byte)(g*255) << 8;
	mColor |= (byte)(b*255) << 16;
	mColor |= (byte)(a*255) << 24;
}


