
#include "Standard.h"



/*
=======================================
Constructor 
=======================================
*/
CRasterizer::CRasterizer() :	m_cWndX("r_wndx","80",CVAR_INT,CVAR_ARCHIVE),
								m_cWndY("r_wndy","40",CVAR_INT,CVAR_ARCHIVE)
{	
	g_pConsole->RegisterCVar(&m_cWndX);
	g_pConsole->RegisterCVar(&m_cWndY);

	mMaxElements = mNumElements = 0;
	mMaxIndices = mNumIndices = 0;
	mColor = 0xffffffff;
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
};


void CRasterizer::PolyVertexf(vector_t &vert)
{
	mVerts[mNumElements].pos[0] = vert.x;
	mVerts[mNumElements].pos[1] = vert.z;
	mVerts[mNumElements].pos[2] =-vert.y;
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


void CRasterizer::PolyTexCoord(float s, float t)
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

