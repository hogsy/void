
#include "Standard.h"
#include "Mdl_entry.h"
#include "Tex_image.h"


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
Constructor 
=======================================
*/
CModelCacheEntry::CModelCacheEntry(const char *file)
{
	// save file name
	modelfile = new char[strlen(file)+1];
	strcpy(modelfile, file);

	num_skins = 0;
	skin_bin = -1;
	skin_names = NULL;
	num_frames = 0;
	frames = NULL;
	cmds = NULL;

	LoadModel();
}


/*
=======================================
Destructor 
=======================================
*/
CModelCacheEntry::~CModelCacheEntry()
{
	if (modelfile)
		delete modelfile;
	g_pRast->TextureBinDestroy(skin_bin);

	if (frames)
	{
		for (int f=0; f<num_frames; f++)
			delete frames[f];
		delete frames;
	}

	if (cmds)
		delete cmds;

	if (skin_names)
	{
		for (int s=0; s<num_skins; s++)
			delete skin_names[s];
		delete skin_names;
	}
}


/*
=======================================
LoadModel 
=======================================
*/
void CModelCacheEntry::LoadModel()
{
	ComPrintf("loading %s\n", modelfile);

	CFileBuffer	 fileReader;

	if(!fileReader.Open(modelfile))
	{
		// load default model
		fileReader.Close();
		LoadFail();
		return;
	}


	md2_header_t header;
	fileReader.Read(&header, sizeof(md2_header_t), 1);

// make sure it's a valid md2
	if ((header.id != 0x32504449) || (header.version != 8))
	{
		ComPrintf("%s - invalid md2 file", modelfile);
		fileReader.Close();
		LoadFail();
		return;
	}

	num_skins = header.numSkins;
	num_frames= header.numFrames;


// the gl command list
	cmds = (void*) new byte[header.numGlCommands * 4];
	if (!cmds) FError("mem for model command list");
	fileReader.Seek(header.offsetGlCommands, SEEK_SET);
	fileReader.Read(cmds, 4, header.numGlCommands);


// vertex info for all frames
	frames = new vector_t*[num_frames];
	if (!frames) FError("mem for model frames");
	fileReader.Seek(header.offsetFrames, SEEK_SET);


	int f, v;
	byte vertex[4];
	vector_t scale, trans;
	char fname[16];

	for (f = 0; f < num_frames; f++)
	{
		frames[f] = new vector_t[header.numVertices];
		if (!frames[f]) FError("mem for frame vertices");

		fileReader.Read(&scale, sizeof(float), 3);
		fileReader.Read(&trans, sizeof(float), 3);
		fileReader.Read(&fname, 16, 1);

		for (v = 0; v < header.numVertices; v++)
		{
			fileReader.Read(vertex, 4, 1);	// the xyz coords and a light normal index

			// scale and translate them to get the actual vertex
			frames[f][v].x = vertex[0]*scale.x + trans.x;
			frames[f][v].y = vertex[1]*scale.y + trans.y;
			frames[f][v].z = vertex[2]*scale.z + trans.z;
		}
	}


	// skin names

	char path[260];
	strcpy(path, modelfile);
	for (int c=strlen(path); c>=0; c--)
	{
		if ((path[c] == '\\') || (path[c] == '/'))
		{
			path[c] = '\0';
			break;
		}
	}

	char texname[260];

	skin_bin = g_pRast->TextureBinInit(header.numSkins);
	skin_names = new char*[header.numSkins];
	if (!skin_names) FError("mem for skin names");

	CImageReader *texReader = new CImageReader();

	fileReader.Seek(header.offsetSkins, SEEK_SET);
	for (int s=0; s<header.numSkins; s++)
	{
		// md2 skin name list is 64 char strings
		skin_names[s] = new char[64];
		if (!skin_names[s])	 FError("mem for skin names");

		fileReader.Read(skin_names[s], 64, 1);

		strcpy(texname, path);
		strcat(texname, skin_names[s]);

		if (!texReader->Read(texname))
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

		g_pRast->TextureLoad(skin_bin, s, &tdata);


	}

	delete texReader;

	fileReader.Close();
}


/*
=======================================
LoadFail
=======================================
*/
void CModelCacheEntry::LoadFail()
{
	// hard code a pyramid
	num_skins = 1;
	num_frames= 1;

	int *cmdint;
	model_glcmd_t *glcmd;


// the gl command list
	cmds = (void*) new byte[33*4];
	if (!cmds) FError("mem for model fail!");

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
	frames = new vector_t*[1];
	if (!frames) FError("mem for model fail frames");

	frames[0] = new vector_t[5];
	if (!frames[0]) FError("mem for model fail vertices");

	frames[0][0].x = 0;
	frames[0][0].y = 0;
	frames[0][0].z = 20;

	frames[0][1].x = -15;
	frames[0][1].y = 15;
	frames[0][1].z = 0;

	frames[0][2].x = 15;
	frames[0][2].y = 15;
	frames[0][2].z = 0;

	frames[0][3].x = 15;
	frames[0][3].y = -15;
	frames[0][3].z = 0;

	frames[0][4].x = -15;
	frames[0][4].y = -15;
	frames[0][4].z = 0;



	// skin names

	char path[260];
	strcpy(path, modelfile);
	for (int c=strlen(path); c>=0; c--)
	{
		if ((path[c] == '\\') || (path[c] == '/'))
		{
			path[c] = '\0';
			break;
		}
	}



	skin_bin = g_pRast->TextureBinInit(1);
	skin_names = new char*[1];
	if (!skin_names) FError("mem for skin names");

	CImageReader *texReader = new CImageReader();

	// md2 skin name list is 64 char strings
	skin_names[0] = new char[64];
	if (!skin_names[0])	 FError("mem for skin names");

	strcpy(skin_names[0], "none");
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

	g_pRast->TextureLoad(skin_bin, 0, &tdata);

	delete texReader;
}



/*
=======================================
Draw
=======================================
*/

void CModelCacheEntry::Draw(int skin, float frame)
{
	g_pRast->TextureSet(skin_bin, skin);

	// make sure frame is in our range
	frame = fmodf(frame, num_frames);

	vector_t v;

	int cframe, fframe;	// frame ceiling and floor
	cframe = (int)ceilf(frame);
	fframe = (int)floorf(frame);

	float w1, w2;	// vertex weights
	w1 = cframe - frame;
	w2 = frame - fframe;


	int *ptr = (int*)cmds;
	int i, num_cmds = *(int*)ptr;
	model_glcmd_t *cmd;

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
			if (frame == fframe)
			{
				v.x = frames[fframe][cmd->vertex_index].x;
				v.y = frames[fframe][cmd->vertex_index].y;
				v.z = frames[fframe][cmd->vertex_index].z;
			}

			// find our interpolated vertex
			else
			{
				v.x = frames[fframe][cmd->vertex_index].x * w1 +
					  frames[cframe][cmd->vertex_index].x * w2;
				v.y = frames[fframe][cmd->vertex_index].y * w1 +
					  frames[cframe][cmd->vertex_index].y * w2;
				v.z = frames[fframe][cmd->vertex_index].z * w1 +
					  frames[cframe][cmd->vertex_index].z * w2;

				v.x /= 2;
				v.y /= 2;
				v.z /= 2;
			}

			g_pRast->PolyTexCoord(cmd->s, cmd->t);
			g_pRast->PolyVertexf(v);

			ptr += 3;
		}

		g_pRast->PolyEnd();

		num_cmds = *(int*)ptr;
	}
}
