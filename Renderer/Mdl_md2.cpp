#include "Standard.h"
#include "Mdl_md2.h"
#include "I_file.h"
#include "Client.h"
#include "ShaderManager.h"


typedef struct
{
   int id;
   int version;
   int skinWidth;
   int skinHeight;
   int frameSize;
   int numSkins;
   int numVertices;
   int numTexCoords;
   int numTriangles;
   int numGlCommands;
   int numFrames;
   int offsetSkins;
   int offsetTexCoords;
   int offsetTriangles;
   int offsetFrames;
   int offsetGlCommands;
   int offsetEnd;
} md2_header_t;

/*
=======================================
Constructor/Destructor 
=======================================
*/
CModelMd2::CModelMd2()
{
	frames = NULL;
	cmds = NULL;
	skin_names = NULL;
}

CModelMd2::~CModelMd2()
{
	if (frames)
	{
		for (int f=0; f<num_frames; f++)
			delete [] frames[f];
		delete [] frames;
	}
	//Delete raw memory
	if (cmds)
		delete [] cmds;
}

/*
=======================================
LoadModel 
=======================================
*/
void CModelMd2::LoadModel(I_FileReader * pFile, const char * szFileName)
{
	ComPrintf("MD2: Loading %s\n", szFileName);

	md2_header_t header;
	pFile->Read(&header, sizeof(md2_header_t), 1);

	// make sure it's a valid md2
	if ((header.id != 0x32504449) || (header.version != 8))
	{
		ComPrintf("%s - invalid md2 file", szFileName);
		pFile->Close();
		LoadFail();
		return;
	}

	// save file name
	modelfile = new char[strlen(szFileName)+1];
	strcpy(modelfile, szFileName);

	num_skins = header.numSkins;
	num_frames= header.numFrames;

	// the gl command list
	cmds = new byte [header.numGlCommands * 4];
	pFile->Seek(header.offsetGlCommands, SEEK_SET);
	pFile->Read(cmds, 4, header.numGlCommands);

	// vertex info for all frames
	frames = new model_vertex_t* [num_frames];
	pFile->Seek(header.offsetFrames, SEEK_SET);

	int f, v;
	byte vertex[4];
	vector_t scale, trans;
	char fname[16];

	for (f = 0; f < num_frames; f++)
	{
		frames[f] = new model_vertex_t[header.numVertices];

		pFile->Read(&scale, sizeof(float), 3);
		pFile->Read(&trans, sizeof(float), 3);
		pFile->Read(&fname, 16, 1);

		for (v = 0; v < header.numVertices; v++)
		{
			pFile->Read(vertex, 4, 1);	// the xyz coords and a light normal index

			// scale and translate them to get the actual vertex
			frames[f][v].v.x = vertex[0]*scale.x + trans.x;
			frames[f][v].v.y = vertex[1]*scale.y + trans.y;
			frames[f][v].v.z = vertex[2]*scale.z + trans.z;
			frames[f][v].norm= vertex[3];
		}
	}

	// skin names
	skin_names = new char* [header.numSkins];

	pFile->Seek(header.offsetSkins, SEEK_SET);
	for (int s=0; s<header.numSkins; s++)
	{
		// md2 skin name list is 64 char strings
		char tskin[64];
		pFile->Read(tskin, 64, 1);

		skin_names[s] = new char[64];

		// strip path and extension
		for (int c=strlen(tskin); c>=0; c--)
		{
			if (tskin[c] == '.')
				tskin[c] = '\0';

			else if ((tskin[c] == '/') || (tskin[c] == '/'))
			{
				strcpy(skin_names[s], &tskin[c+1]);
				break;
			}
		}
	}

	pFile->Close();
	LoadSkins();
}


/*
=======================================
LoadFail
=======================================
*/
void CModelMd2::LoadFail()
{
	modelfile = new char[strlen(MODEL_DEFAULT_NAME)+1];
	strcpy(modelfile, MODEL_DEFAULT_NAME);

	// hard code a pyramid
	num_skins = 1;
	num_frames= 1;

	int *cmdint;
	model_glcmd_t *glcmd;

	// the gl command list
	cmds = new byte [33*4];

	cmdint = (int*)cmds;
	*cmdint = -6;	// triangle fan
	cmdint += 1;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0.5f;
	glcmd->t = 0.5f;
	glcmd->vertex_index = 0;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0;
	glcmd->t = 0;
	glcmd->vertex_index = 1;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 1;
	glcmd->t = 0;
	glcmd->vertex_index = 2;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 1;
	glcmd->t = 1;
	glcmd->vertex_index = 3;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0;
	glcmd->t = 1;
	glcmd->vertex_index = 4;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0;
	glcmd->t = 0;
	glcmd->vertex_index = 1;

	*cmdint = -4;
	cmdint += 1;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0;
	glcmd->t = 0;
	glcmd->vertex_index = 1;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 0;
	glcmd->t = 1;
	glcmd->vertex_index = 4;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 1;
	glcmd->t = 1;
	glcmd->vertex_index = 3;

	glcmd = (model_glcmd_t*)cmdint;
	cmdint += 3;
	glcmd->s = 1;
	glcmd->t = 0;
	glcmd->vertex_index = 2;

	*cmdint = 0;

	// vertex info
	frames = new model_vertex_t* [1];
	frames[0] = new model_vertex_t[5];

	frames[0][0].v.x = 0;
	frames[0][0].v.y = 0;
	frames[0][0].v.z = 20;

	frames[0][1].v.x = -15;
	frames[0][1].v.y = 15;
	frames[0][1].v.z = 0;

	frames[0][2].v.x = 15;
	frames[0][2].v.y = 15;
	frames[0][2].v.z = 0;

	frames[0][3].v.x = 15;
	frames[0][3].v.y = -15;
	frames[0][3].v.z = 0;

	frames[0][4].v.x = -15;
	frames[0][4].v.y = -15;
	frames[0][4].v.z = 0;

	// skin names
	skin_names = new char*[1];
	
	// md2 skin name list is 64 char strings
	skin_names[0] = new char[64];
	strcpy(skin_names[0], "none");
	
	LoadSkins();
}


/*
=======================================
Draw
=======================================
*/
void CModelMd2::Draw(int skin, int fframe, int cframe, float frac)
{
	if (!(skin & (MODEL_SKIN_UNBOUND_GAME|MODEL_SKIN_UNBOUND_LOCAL)))
		if (skin >= num_skins)
			return;

	if (skin & MODEL_SKIN_UNBOUND_GAME)
		g_pClient->Set(CACHE_GAME, skin & ~MODEL_SKIN_UNBOUND_GAME);
	else if (skin & MODEL_SKIN_UNBOUND_LOCAL)
		g_pClient->Set(CACHE_LOCAL, skin & ~MODEL_SKIN_UNBOUND_LOCAL);
	else
		g_pRast->ShaderSet(g_pShaders->GetShader(mShaderBin, skin));

	g_pRast->TextureTexDef(NULL);
	g_pRast->TextureLightDef(NULL);

	vector_t v;

	float w1, w2;	// vertex weights
	w1 = 1 - frac;
	w2 = frac;


	int *ptr = (int*)cmds;
	int i, num_cmds = *(int*)ptr;
	model_glcmd_t *cmd;

	// we'll save some time by not interpolating normals - it doesnt really have any visible artifacts

	while (num_cmds)
	{
		ptr += 1;

		if (num_cmds > 0)
			g_pRast->PolyStart(VRAST_TRIANGLE_STRIP);
		else
		{
			num_cmds = -num_cmds;
			g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
		}

		for (i = 0; i < num_cmds; i++)
		{
			cmd = (model_glcmd_t*) ptr;


			// if we're exactly on a frame, dont do any interpolation
			if (cframe == fframe)
			{
				v.x = frames[fframe][cmd->vertex_index].v.x;
				v.y = frames[fframe][cmd->vertex_index].v.y;
				v.z = frames[fframe][cmd->vertex_index].v.z;
			}

			// find our interpolated vertex
			else
			{
				v.x = frames[fframe][cmd->vertex_index].v.x * w1 +
					  frames[cframe][cmd->vertex_index].v.x * w2;
				v.y = frames[fframe][cmd->vertex_index].v.y * w1 +
					  frames[cframe][cmd->vertex_index].v.y * w2;
				v.z = frames[fframe][cmd->vertex_index].v.z * w1 +
					  frames[cframe][cmd->vertex_index].v.z * w2;
			}

			g_pRast->PolyTexCoord(cmd->s, cmd->t);
			g_pRast->PolyNormali(frames[fframe][cmd->vertex_index].norm);
			g_pRast->PolyVertexf(v);

			ptr += 3;
		}

		g_pRast->PolyEnd();

		num_cmds = *(int*)ptr;
	}
}
