// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //
//														//
//	I know that when the game dll's are made, this		//
//	will have to be pretty much totally rewritten.		//
//														//
// !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!! //


#include "Mdl_main.h"

model_t *models;	// all model data


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



/******************************************************************************
draw a model - assumes all gl transforms are correct
******************************************************************************/
void model_draw(int mindex, float frame)
{
	while (frame > (models[mindex].num_frames-1))
		frame -= models[mindex].num_frames-1;

	vector_t v;

	int cframe, fframe;	// frame ceiling and floor
	cframe = (int)ceil(frame);
	fframe = (int)floor(frame);

	float w1, w2;	// vertex weights
	w1 = cframe - frame;
	w2 = frame - fframe;


	int *ptr = (int*)models[mindex].cmds;
	int i, num_cmds = *(int*)ptr;
	model_glcmd_t *cmd;

	while (num_cmds)
	{
		ptr += 1;


		if (num_cmds > 0)
			g_pRast->PolyStart(VRAST_TRIANGLE_STRIP);
		else
		{
			num_cmds *= -1;
			g_pRast->PolyStart(VRAST_TRIANGLE_FAN);
		}

		for (i = 0; i < num_cmds; i++)
		{
			cmd = (model_glcmd_t*) ptr;


			// if we're exactly on a frame, dont do any interpolation
			if (frame == fframe)
			{
				v.x = models[mindex].frames[fframe].vertices[cmd->vertex_index].x;
				v.y = models[mindex].frames[fframe].vertices[cmd->vertex_index].y;
				v.z = models[mindex].frames[fframe].vertices[cmd->vertex_index].z;
			}

			// find our interpolated vertex
			else
			{
				v.x = models[mindex].frames[fframe].vertices[cmd->vertex_index].x * w1 +
					  models[mindex].frames[cframe].vertices[cmd->vertex_index].x * w2;
				v.y = models[mindex].frames[fframe].vertices[cmd->vertex_index].y * w1 +
					  models[mindex].frames[cframe].vertices[cmd->vertex_index].y * w2;
				v.z = models[mindex].frames[fframe].vertices[cmd->vertex_index].z * w1 +
					  models[mindex].frames[cframe].vertices[cmd->vertex_index].z * w2;

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


/******************************************************************************
load a default model in case there is a prob loading one from disk
******************************************************************************/
void model_create_fail(model_t *m)
{
	// hard code a pyramid
	m->num_skins = 1;
	m->num_frames= 1;

	int *cmdint;
	model_glcmd_t *glcmd;


// the gl command list first
	m->cmds = (void*) new byte[33*4];
	if (!m->cmds) FError("mem for model fail!");

	cmdint = (int*)m->cmds;
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
	m->frames = new model_frame_t[1];
	if (!m->frames) FError("mem for model fail frames");

	m->frames->vertices = new vector_t[5];
	if (!m->frames->vertices) FError("mem for model fail vertices");

	m->frames->vertices[0].x = 0;
	m->frames->vertices[0].y = 0;
	m->frames->vertices[0].z = 20;

	m->frames->vertices[1].x = -15;
	m->frames->vertices[1].y = 15;
	m->frames->vertices[1].z = 0;

	m->frames->vertices[2].x = 15;
	m->frames->vertices[2].y = 15;
	m->frames->vertices[2].z = 0;

	m->frames->vertices[3].x = 15;
	m->frames->vertices[3].y = -15;
	m->frames->vertices[3].z = 0;

	m->frames->vertices[4].x = -15;
	m->frames->vertices[4].y = -15;
	m->frames->vertices[4].z = 0;
}


/******************************************************************************
read a md2 file from disk
******************************************************************************/
void model_read_md2(model_t *m, char *name)
{
	char path[260];

//	sprintf(path, "%s\\game\\models\\%s\\tris.md2", rInfo->base_dir, name);
//	sprintf(path, "%s/models/%s/tris.md2",CFileSystem::GetCurrentPath(), name);

	ConPrint("loading %s\n", path);

	FILE *fin = fopen(path, "rb");

	if (!fin)
	{
		ConPrint("couldn't find %s", name);
		model_create_fail(m);
		return;
	}

	md2_header_t header;
	fread(&header, sizeof(md2_header_t), 1, fin);

// make sure it's a valid md2
	if ((header.id != 0x32504449) || (header.version != 8))
	{
		ConPrint("%s - invalid md2 file", name);
		model_create_fail(m);
		return;
	}

	m->num_skins = header.numSkins;
	m->num_frames= header.numFrames;

// the gl command list first
	m->cmds = (void*) new byte[header.numGlCommands * 4];
	if (!m->cmds) FError("mem for model command list");
	fseek(fin, header.offsetGlCommands, SEEK_SET);
	fread(m->cmds, 4, header.numGlCommands, fin);



// vertex info for all frames
	m->frames = new model_frame_t[m->num_frames];
	if (!m->frames) FError("mem for model frames");
	fseek(fin, header.offsetFrames, SEEK_SET);


	int f, v;
	byte vertex[4];
	vector_t scale, trans;
	char fname[16];

	for (f = 0; f < m->num_frames; f++)
	{
		m->frames[f].vertices = new vector_t[header.numVertices];
		if (!m->frames[f].vertices) FError("mem for frame vertices");

		fread(&scale, sizeof(float), 3, fin);
		fread(&trans, sizeof(float), 3, fin);
		fread(&fname, 16, 1, fin);

		for (v = 0; v < header.numVertices; v++)
		{
			fread(vertex, 4, 1, fin);	// the xyz coords and a light normal index

			// scale and translate them to get the actual vertex
			m->frames[f].vertices[v].x = vertex[0]*scale.x + trans.x;
			m->frames[f].vertices[v].y = vertex[1]*scale.y + trans.y;
			m->frames[f].vertices[v].z = vertex[2]*scale.z + trans.z;
		}
	}
}


/******************************************************************************
load all the entity models in a map
******************************************************************************/
void model_load_map(void)
{
/*
	models = (model_t*) MALLOC(sizeof(model_t) * world->header.num_entities);
	if (!models) FError("mem for models");

	int m;

	for (m = 0; m < world->header.num_entities; m++)
	{
		model_read_md2(&models[m], &world->entities[MAX_ENTITY_NAME_LENGTH*m]);
	}
*/
}



/******************************************************************************
destroy all the models in the map
******************************************************************************/
void model_destroy_map(void)
{
	if (!models)
		return;
/*
	int m, f;
	for (m = 0; m < world->header.num_entities; m++)
	{
		free(models[m].cmds);

		for(f = 0; f < models[m].num_frames; f++)
			free(models[m].frames[f].vertices);

		free(models[m].frames);

//		glDeleteTextures(models[m].num_skins, models[m].skins);
//		free(models[m].skins);
	}

	free (models);
	models = NULL;
*/
}






