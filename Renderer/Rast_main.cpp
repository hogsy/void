#include "Standard.h"
#include "Shader.h"
#include "ShaderManager.h"

// to evaluate console alphagen
#include "Con_main.h"


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
	mFirstIndex = mFirstElement = 0;

	mColor = 0xffffffff;
	mShader = NULL;
	mTexDef = NULL;
	mLightDef = NULL;
	mUseLights = false;
}



void CRasterizer::PolyStart(EPolyType type)
{
	mType = type;
	mFirstIndex = mNumIndices;
	mFirstElement = mNumElements;
}


void CRasterizer::PolyEnd(void)
{
	int t;
	int num;
	unsigned short *indices = &mIndices[mFirstIndex];

	// convert the type into triangles
	switch (mType)
	{
	case VRAST_TRIANGLE_FAN:
		num = mNumElements - mFirstElement - 2;
		for (t=0; t<num; t++)
		{
			indices[t*3 + 0] = 0;
			indices[t*3 + 1] = t + 1;
			indices[t*3 + 2] = t + 2;
		}
		mNumIndices += num*3;
		break;

	case VRAST_TRIANGLE_STRIP:
		num = mNumElements - mFirstElement - 2;
		for (t=0; t<num; t++)
		{
			if (!(t%2))
			{
				indices[t*3 + 0] = t;
				indices[t*3 + 1] = t+1;
				indices[t*3 + 2] = t+2;
			}
			else
			{
				indices[t*3 + 0] = t;
				indices[t*3 + 1] = t+2;
				indices[t*3 + 2] = t+1;
			}
		}
		mNumIndices += num*3;
		break;

	case VRAST_QUADS:
		num = (mNumElements - mFirstElement) / 4;
		for (t=0; t<num; t++)
		{
			indices[mNumIndices + 0] = t*4;
			indices[mNumIndices + 1] = t*4+1;
			indices[mNumIndices + 2] = t*4+2;

			indices[mNumIndices + 3] = t*4;
			indices[mNumIndices + 4] = t*4+2;
			indices[mNumIndices + 5] = t*4+3;

			mNumIndices += 6;
		}

		break;
	}

	if (mMaxElements < mNumElements)
		mMaxElements = mNumElements;
	if (mMaxIndices < mNumIndices)
		mMaxIndices = mNumIndices;


	// generate texture coord data
	// put it in separate arrays cause we might need to do tcmod's and don't want to have to re-evaluate them
	if (mTexDef)
	{
		for (int i=mFirstElement; i<mNumElements; i++)
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

	if (mLightDef)
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
}


void CRasterizer::Flush(void)
{
	if (!mShader && !mNumIndices)
		return;

	// 3 indices per triangle
	mTrisDrawn += mNumIndices / 3;

	for (int i=0; i<mShader->mNumLayers; i++)
		DrawLayer(i);

	mNumElements = mNumIndices = 0;
}


void CRasterizer::DrawLayer(int l)
{
	CShaderLayer *layer = mShader->mLayers[l];
	int i, a;
	vector_t dir, norm;

	if (layer->mIsLight && !mUseLights)
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

	case ALPHAGEN_WAVE:
		a = (int)((sin(GetCurTime()) + 1) * 255.0f / 2.0f);
		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].color &= 0x00ffffff;
			mVerts[i].color |= a << 24;
		}

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
		if (!mTexDef)
			break;

		for (i=0; i<mNumElements; i++)
		{
			mVerts[i].tex1[0] = mTexCoords[i][0];
			mVerts[i].tex1[1] = mTexCoords[i][1];
		}
		break;


	//
	// FIXME - move this to PolyEnd() somehow so we dont have to flush if the texdef changes
	//
	case TEXGEN_SKY:
		if (!mTexDef)
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
		if (!mLightDef)
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

			mVerts[i].tex1[1] =	mVerts[i].pos[0] * layer->mTexVector[1].x +
								mVerts[i].pos[1] * layer->mTexVector[1].y +
								mVerts[i].pos[2] * layer->mTexVector[1].z;
		}
		break;

	case TEXGEN_ENVIRONMENT:
		for (i=0; i<mNumElements; i++)
		{
			dir.x = (mVerts[i].pos[0] - camera->origin.x);
			dir.y = (mVerts[i].pos[1] - camera->origin.y);
			dir.z = (mVerts[i].pos[2] - camera->origin.z);

			norm.x = mVerts[i].pos[0];
			norm.y = mVerts[i].pos[1];
			norm.z = mVerts[i].pos[2];

			dir.Normalize();
			norm.Normalize();


			mVerts[i].tex1[0] = dir.x;// + norm.x;
			mVerts[i].tex1[1] = dir.y;// + norm.y;
		}
		break;
	}

	// tcmod's
	for (i=0; i<mNumElements; i++)
		mShader->mLayers[l]->mHeadTCMod->Evaluate(mVerts[i].tex1[0], mVerts[i].tex1[1], GetCurTime());

	// set texture
	if (layer->mIsLight)
		TextureSet(g_pShaders->mLightmaps[mLightDef->texture]);
	else
	{
		if (mShader->mLayers[l]->mNumTextures == 1)
			TextureSet(layer->mTextureNames[0].index);
		else
		{
			int texture = (int)(mShader->mLayers[l]->mAnimFreq * mShader->mLayers[l]->mNumTextures * GetCurTime());
			texture %= mShader->mLayers[l]->mNumTextures;
			TextureSet(layer->mTextureNames[texture].index);
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


void CRasterizer::PolyColor(float r, float g, float b, float a)
{
	mColor  = (byte)(r*255);
	mColor |= (byte)(g*255) << 8;
	mColor |= (byte)(b*255) << 16;
	mColor |= (byte)(a*255) << 24;
}



void CRasterizer::ShaderSet(CShader *shader)
{
	if (shader != mShader)
	{
		Flush();
		mShader = shader;
	}
}

void CRasterizer::TextureTexDef(bspf_texdef_t *def)
{
	// FIXME - dont need to flush all the time
	Flush();
	mTexDef	= def;
}

void CRasterizer::TextureLightDef(bspf_texdef_t *def)
{
	Flush();
	mLightDef = def;
}

